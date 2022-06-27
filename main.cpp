#include <gtkmm.h>
#include <string>
#include <iostream>
#include <vector>
#include "lib.h"
#include <nlohmann/json.hpp>
#include "Entry.h"
#include "Form.h"
#include "gpu_detect/gpu_detect.h"
#include <thread>
#include <atomic>
#include <queue>
#include "xdg/xdg.h"
#include <filesystem>
#include <fstream>
using nlohmann::json;
using std::string;
using std::cout;
using std::endl;
using std::vector;

Gtk::ListBox* list = nullptr;
Gtk::CheckButton* path_enable = nullptr;
Gtk::Entry* path_entry = nullptr;
Gtk::Button* open_folder = nullptr;

string find_hw_codec(const string& codec, GPU& gpu){
    if(codec == "h264"){
        switch (gpu.vendor) {
            case Vendor::NVIDIA:
                return "h264_nvenc";
            case Vendor::AMD:
                return "h264_vaapi";
            case Vendor::INTEL:
                return "h264_qsv";
            default:
                return codec;
        }
    }

    if(codec == "hevc"){
        switch (gpu.vendor) {
            case Vendor::NVIDIA:
                return "hevc_nvenc";
            case Vendor::AMD:
                return "hevc_vaapi";
            case Vendor::INTEL:
                return "hevc_qsv";
            default:
                return codec;
        }
    }

    if(codec == "mpeg2video"){
        switch (gpu.vendor) {
            case Vendor::AMD:
                return "mpeg2_vaapi";
            case Vendor::INTEL:
                return "mpeg2_qsv";
            default:
                return codec;
        }
    }

    if(codec == "libvpx"){
        switch (gpu.vendor) {
            case Vendor::AMD:
                return "vp8_vaapi";
            default:
                return codec;
        }
    }

    if(codec == "libvpx-vp9"){
        switch (gpu.vendor) {
            case Vendor::AMD:
                return "vp9_vaapi";
            case Vendor::INTEL:
                return "vp9_qsv";
            default:
                return codec;
        }
    }

    return codec;
}

std::unique_ptr<std::thread> process_thread = nullptr;
std::atomic<bool> process_done = true;
std::mutex m;
auto& get_queue(){
    static std::queue<Entry*> queue;
    return queue;
}

void worker(const EncodeInfo& param){
    while(true){
        m.lock();
        if(get_queue().empty()){
            m.unlock();
            return;
        }
        Entry* entry = get_queue().front();get_queue().pop();
        m.unlock();

        entry->process(param);
    }
}

void process(vector<Gtk::Widget*> rows){
    Gtk::ComboBoxText* codec_comboBox = nullptr;
    Form::getInstance().getBuilder()->get_widget("codec",codec_comboBox);
    Gtk::ComboBoxText* container_comboBox = nullptr;
    Form::getInstance().getBuilder()->get_widget("container",container_comboBox);

    string codec = codec_comboBox->get_active_id();
    string container = container_comboBox->get_active_text();

    GPU gpu = get_gpu_list().front();

    string hw_codec = find_hw_codec(codec,gpu);
    cout << codec << " " << hw_codec << endl;

    EncodeInfo param;
    param.codec = codec;
    param.hw_codec = hw_codec;
    param.container = container;
    param.gpu = gpu;
    if(path_enable->get_active()){
        param.path = path_entry->get_text();
    }

    std::vector<std::thread> threads(std::thread::hardware_concurrency());

    if(rows.size() <= threads.size()){
        for(int i = 0; i < rows.size(); i++){
            auto* entry = dynamic_cast<Entry*>(rows[i]);
            threads[i] = std::thread(&Entry::process,entry, param);
        }

        for(int i = 0; i < rows.size(); i++){
            threads[i].join();
        }
    }else{
        for(auto row : rows){
            auto* entry = dynamic_cast<Entry*>(row);
            get_queue().push(entry);
        }

        for(auto& thread : threads){
            thread = std::thread(worker,param);
        }

        for(auto& thread : threads){
            thread.join();
        }
        get_queue() = std::queue<Entry*>();
    }

    process_done = true;
}

void start(){
    auto children = list->get_children();

    if(!process_done){
        return;
    }

    process_done = false;
    if(process_thread != nullptr){
        process_thread->join();
    }
    process_thread = std::make_unique<std::thread>(process,children);
}

std::string UrlDecode(const std::string& value)
{
    std::string result;
    result.reserve(value.size());

    for (std::size_t i = 0; i < value.size(); ++i)
    {
        auto ch = value[i];

        if (ch == '%' && (i + 2) < value.size())
        {
            auto hex = value.substr(i + 1, 2);
            auto dec = static_cast<char>(std::strtol(hex.c_str(), nullptr, 16));
            result.push_back(dec);
            i += 2;
        }
        else if (ch == '+')
        {
            result.push_back(' ');
        }
        else
        {
            result.push_back(ch);
        }
    }

    return result;
}

void callback(const Glib::RefPtr<Gdk::DragContext>& context,
              const int& x,
              const int& y, const Gtk::SelectionData& seldata, const unsigned int& info,const unsigned int& time){
    for(auto& uri : seldata.get_uris()){
        auto file = UrlDecode(uri);
        json probe = FFmpeg::probe(file);
        auto row = Gtk::make_managed<Entry>(file, std::move(probe));
        list->add(*row);
        row->show_all();
    }

    //    for(auto url : seldata.get_data_as_string()){
//        cout << url << endl;
//    }

//    cout << UrlDecode(seldata.get_data_as_string()) << endl;

    //    std::istringstream files(seldata.get_data_as_string());
//
//    cout << files.str() << endl;
//    string file;
//    while(std::getline(files,file,'\n')){
//        json probe = FFmpeg::probe(file);
//        auto row = Gtk::make_managed<Entry>(file, std::move(probe));
//        list->add(*row);
//        row->show_all();
//    }
}

void save_config(Gtk::Widget* self){
    Gtk::ComboBoxText* codec_comboBox = nullptr;
    Form::getInstance().getBuilder()->get_widget("codec",codec_comboBox);
    Gtk::ComboBoxText* container_comboBox = nullptr;
    Form::getInstance().getBuilder()->get_widget("container",container_comboBox);

//        cout << codec_comboBox->get_active_id() << endl;
    string config_dir = xdg::config().home().string() + "/ffmpeg_gui";
    if (!std::filesystem::is_directory(config_dir) || !std::filesystem::exists(config_dir)) { // Check if src folder exists
        std::filesystem::create_directory(config_dir); // create src folder
    }
    string config_file = std::move(config_dir) + "/config";
    json config;
    config["codec"] = codec_comboBox->get_active_id();
    config["container"] = container_comboBox->get_active_text();

    config["path_enable"] = path_enable->get_active();
    config["path_entry"] = path_entry->get_text();

    std::ofstream stream(config_file);

    stream << config;
    stream.close();
}

void restore_config(){
    string config_dir = xdg::config().home().string() + "/ffmpeg_gui";
    if (!std::filesystem::is_directory(config_dir) || !std::filesystem::exists(config_dir)) { // Check if src folder exists
        std::filesystem::create_directory(config_dir); // create src folder
    }
    string config_file = std::move(config_dir) + "/config";

    std::ifstream stream(config_file);

    json config;
    stream >> config;

    Gtk::ComboBoxText* codec_comboBox = nullptr;
    Form::getInstance().getBuilder()->get_widget("codec",codec_comboBox);
    Gtk::ComboBoxText* container_comboBox = nullptr;
    Form::getInstance().getBuilder()->get_widget("container",container_comboBox);

    codec_comboBox->set_active_id(config["codec"]);
    container_comboBox->set_active_text(config["container"]);

//    config["path_enable"] = path_enable->get_active();
//    config["path_entry"] = path_entry->get_text();
    path_enable->set_active(config["path_enable"]);
    path_entry->set_text(config["path_entry"]);
    path_entry->set_sensitive(config["path_enable"]);
    open_folder->set_sensitive(config["path_enable"]);
}

int main(int argc, char *argv[])
{
    cout << xdg::config().home() << endl;
    auto app = Gtk::Application::create(argc, argv,"org.gtkmm.examples.base");

    Gtk::Window* window = nullptr;
    Form::getInstance().getBuilder()->get_widget("main", window);
    window->show_all();

    window->set_size_request(500,500);

    Form::getInstance().getBuilder()->get_widget("list",list);
    list->drag_dest_set({
        Gtk::TargetEntry("text/uri-list")
    },Gtk::DestDefaults::DEST_DEFAULT_ALL, Gdk::DragAction::ACTION_COPY);

    list->signal_drag_data_received().connect(sigc::ptr_fun(callback));

    Gtk::Button* start_button = nullptr;
    Form::getInstance().getBuilder()->get_widget("start",start_button);

    start_button->signal_clicked().connect(sigc::ptr_fun(start));

    Form::getInstance().getBuilder()->get_widget("path_enable",path_enable);
    Form::getInstance().getBuilder()->get_widget("path_entry",path_entry);


    Form::getInstance().getBuilder()->get_widget("open_folder",open_folder);
    open_folder->signal_clicked().connect([&window](){
       auto dialog = Gtk::FileChooserNative::create(
               "Please choose a folder",
               *window,
               Gtk::FileChooserAction::FILE_CHOOSER_ACTION_SELECT_FOLDER);

       dialog->run();

       if(!dialog->get_filename().empty()){
           path_entry->set_text(dialog->get_filename());
       }


    });
    restore_config();
    path_enable->signal_toggled().connect([](){
        path_entry->set_sensitive(path_enable->get_active());
        open_folder->set_sensitive(path_enable->get_active());
    });

    window->signal_remove().connect(sigc::ptr_fun(save_config));

    window->show_all();
    int result = app->run(*window);
    if(process_thread != nullptr){
        process_thread->join();
    }
    delete window;
    return result;
}