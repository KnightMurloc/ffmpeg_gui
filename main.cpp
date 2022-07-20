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
#include "Settings.h"
#include "config.h"
#include "build_config.h"

using nlohmann::json;
using std::string;
using std::cout;
using std::endl;
using std::vector;

Gtk::ListBox* list = nullptr;
Gtk::CheckButton* path_enable = nullptr;
Gtk::Entry* path_entry = nullptr;
Gtk::Button* open_folder = nullptr;

string find_hw_codec(const string& codec, GPU gpu){
    auto settings = Settings::getInstance();

    if(codec == "h264"){
        switch (gpu.vendor) {
            case Vendor::NVIDIA:
                return "h264_nvenc";
            case Vendor::AMD:
                if(settings.isUseVaapi()){
                    return "h264_vaapi";
                }else{
                    return "h264_amf";
                }
            case Vendor::INTEL:
                if(settings.isUseVaapi()){
                    return "h264_vaapi";
                }else{
                    return "h264_qsv";
                }
            default:
                return codec;
        }
    }

    if(codec == "hevc"){
        switch (gpu.vendor) {
            case Vendor::NVIDIA:
                return "hevc_nvenc";
            case Vendor::AMD:
                if(settings.isUseVaapi()){
                    return "hevc_vaapi";
                }else{
                    return "hevc_amf";
                }
            case Vendor::INTEL:
                if(settings.isUseVaapi()){
                    return "hevc_vaapi";
                }else{
                    return "hevc_qsv";
                }
            default:
                return codec;
        }
    }

    if(codec == "mpeg2video"){

        if(settings.isUseVaapi() && gpu.vendor != Vendor::NVIDIA){
            return "mpeg2_vaapi";
        }else{
            return codec;
        }
    }

    if(codec == "libvpx"){
        switch (gpu.vendor) {
            case Vendor::INTEL:
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
                if(settings.isUseVaapi()){
                    return "vp9_vaapi";
                }else {
                    return "vp9_qsv";
                }
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

//    Gtk::Entry* video_bitrate_entry = nullptr;
    Gtk::Entry* audio_bitrate_entry = nullptr;
    Gtk::ComboBoxText* audio_codec = nullptr;
    Gtk::ListBox* test_list = nullptr;

//    Form::getInstance().getBuilder()->get_widget("video_bitrate_entry",video_bitrate_entry);
    Form::getInstance().getBuilder()->get_widget("audio_bitrate_entry",audio_bitrate_entry);
    Form::getInstance().getBuilder()->get_widget("audio_codec",audio_codec);
    Form::getInstance().getBuilder()->get_widget("test_list",test_list);

    string codec = codec_comboBox->get_active_id();
    string container = container_comboBox->get_active_text();

    auto settings = Settings::getInstance();

    string hw_codec = find_hw_codec(codec,settings.getGpu());
    cout << codec << " " << hw_codec << endl;

    EncodeInfo param;
    param.codec = codec;
    param.hw_codec = hw_codec;
    param.container = container;
    param.gpu = settings.getGpu();

    if(path_enable->get_active()){
        param.path = path_entry->get_text();
    }

//    if(video_bitrate_entry->get_text() != "-1"){
//        param.video_bitrate = video_bitrate_entry->get_text();
//    }

    for(auto row : test_list->get_children()){
        auto* option = dynamic_cast<Param*>(row);

//        cout << option->getValue() << endl;
        if(!option->isDefault()) {
            param.extra_options.push_back("-" + option->getParam() + " " + option->getValue());
        }
    }

    if(audio_bitrate_entry->get_text() != "-1"){
        param.audio_bitrate = audio_bitrate_entry->get_text();
    }

    param.audio_codec = audio_codec->get_active_text();
    cout << param.audio_codec << endl;

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

std::string UrlDecode(const std::string& value){
    std::string result;
    result.reserve(value.size());

    for (std::size_t i = 0; i < value.size(); ++i){
        auto ch = value[i];

        if (ch == '%' && (i + 2) < value.size()){
            auto hex = value.substr(i + 1, 2);
            auto dec = static_cast<char>(std::strtol(hex.c_str(), nullptr, 16));
            result.push_back(dec);
            i += 2;
        }
        else if (ch == '+'){
            result.push_back(' ');
        }
        else{
            result.push_back(ch);
        }
    }

    return result;
}

void add_file(string& file){
    json probe = FFmpeg::probe(file);
    auto row = Gtk::make_managed<Entry>(file, std::move(probe));
    list->add(*row);
    row->show_all();
}

void callback(const Glib::RefPtr<Gdk::DragContext>& context,
              const int& x,
              const int& y, const Gtk::SelectionData& seldata, const unsigned int& info,const unsigned int& time){
    for(auto& uri : seldata.get_uris()){
        auto file = UrlDecode(uri);
        add_file(file);
    }
}

void save_config(Gtk::Widget* self){
    Gtk::ComboBoxText* codec_comboBox = nullptr;
    Form::getInstance().getBuilder()->get_widget("codec",codec_comboBox);
    Gtk::ComboBoxText* container_comboBox = nullptr;
    Form::getInstance().getBuilder()->get_widget("container",container_comboBox);

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

    json settings_json;
    auto& settings = Settings::getInstance();
    settings_json["gpu"]["name"] = settings.getGpu().name;
    settings_json["gpu"]["vendor"] = settings.getGpu().vendor;

    settings_json["ffmpeg_path"] = settings.getFfmpegPath();
    settings_json["ffprobe_path"] = settings.getFfprobePath();
    settings_json["use_vaapi"] = settings.isUseVaapi();

    config["settings"] = settings_json;

    std::ofstream stream(config_file);

    stream << config;
    stream.close();
}

void restore_config(){
    string config_dir = xdg::config().home().string() + "/ffmpeg_gui";
    if (!std::filesystem::is_directory(config_dir) || !std::filesystem::exists(config_dir)) { // Check if src folder exists
        std::filesystem::create_directory(config_dir); // create src folder
    }
    string config_file = config_dir + "/config";

    std::ifstream stream(config_file);
    auto builder = Form::getInstance().getBuilder();
    auto& settings = Settings::getInstance();
    if(stream) {
        json config;
        stream >> config;

        Gtk::ComboBoxText *codec_comboBox = nullptr;
        builder->get_widget("codec", codec_comboBox);
        Gtk::ComboBoxText *container_comboBox = nullptr;
        builder->get_widget("container", container_comboBox);

        codec_comboBox->set_active_id(config["codec"]);
        container_comboBox->set_active_text(config["container"]);

        path_enable->set_active(config["path_enable"]);
        path_entry->set_text(config["path_entry"]);
        path_entry->set_sensitive(config["path_enable"]);
        open_folder->set_sensitive(config["path_enable"]);

        json settings_json = config["settings"];

        GPU gpu{
            .name = settings_json["gpu"]["name"],
            .vendor = settings_json["gpu"]["vendor"]
        };
        settings.setGpu(gpu);
        settings.setFfmpegPath(settings_json["ffmpeg_path"]);
        settings.setFfprobePath(settings_json["ffprobe_path"]);
        settings.setUseVaapi(settings_json["use_vaapi"]);

    }else {
        GPU gpu = get_gpu_list().front();
        settings.setGpu(gpu);
        cout << settings.getGpu().name << endl;

        string appdir = std::getenv("APPDIR") ? std::getenv("APPDIR") : "";
        //if app run as appimage
        if (!appdir.empty()) {
            std::filesystem::path ffmpeg_path{appdir + "/usr/bin/ffmpeg"};
            std::filesystem::path ffprobe_path{appdir + "/usr/bin/ffprobe"};
            if (std::filesystem::exists(ffmpeg_path)) {
                settings.setFfmpegPath(ffmpeg_path.string());
            } else {
                //TODO use which find ffmpeg binary
                settings.setFfmpegPath("/bin/ffmpeg");
            }

            if (std::filesystem::exists(ffprobe_path)) {
                settings.setFfprobePath(ffprobe_path.string());
            } else {
                //TODO use which find ffprobe binary
                settings.setFfprobePath("/bin/ffprobe");
            }

        } else {
            //TODO use which find ffmpeg binary
            settings.setFfmpegPath("/bin/ffmpeg");
            //TODO use which find ffprobe binary
            settings.setFfprobePath("/bin/ffprobe");

            Gtk::Entry *ffmpeg_entry = nullptr;
            Gtk::Entry *ffprobe_entry = nullptr;

            builder->get_widget("ffmpeg_entry", ffmpeg_entry);
            builder->get_widget("ffprobe_entry", ffprobe_entry);

            ffmpeg_entry->set_text("/bin/ffmpeg");
            ffprobe_entry->set_text("/bin/ffprobe");
        }


    }

    Gtk::Entry* ffmpeg_entry = nullptr;
    Gtk::Entry* ffprobe_entry = nullptr;
    builder->get_widget("ffmpeg_entry", ffmpeg_entry);
    builder->get_widget("ffprobe_entry", ffprobe_entry);
    ffmpeg_entry->set_text(settings.getFfmpegPath());
    ffprobe_entry->set_text(settings.getFfprobePath());


    Gtk::Label *gpu_name_label = nullptr;
    Gtk::CheckButton *use_vaapi_check = nullptr;
    Gtk::Label *api_name_label = nullptr;
    builder->get_widget("gpu_name_label", gpu_name_label);
    builder->get_widget("use_vaapi_check", use_vaapi_check);
    builder->get_widget("api_name_label", api_name_label);
    gpu_name_label->set_text(settings.getGpu().name);
    if (settings.getGpu().vendor == Vendor::INTEL) {
        api_name_label->set_text("use vaapi instead of quicksync");
    } else if (settings.getGpu().vendor == Vendor::AMD) {
        api_name_label->set_text("use vaapi instead of AMF");
    }else{
        api_name_label->set_text("use vaapi");
        use_vaapi_check->set_active(false);
        use_vaapi_check->set_sensitive(false);
        settings.setUseVaapi(false);
    }
}

void show_config(){
    auto builder = Form::getInstance().getBuilder();
    Gtk::Dialog* dialog = nullptr;
    builder->get_widget("config",dialog);

    Gtk::ListBox* video_config = nullptr;
    builder->get_widget("video_config",video_config);

//    video_config->set_data("codec"/)

    Gtk::ComboBoxText* codec_comboBox = nullptr;
    Form::getInstance().getBuilder()->get_widget("codec",codec_comboBox);

    cout << video_config->get_data("codec") << " " << codec_comboBox->get_active_id() << " ";
    if(video_config->get_data("codec")){
        cout <<
        (char*) video_config->get_data("codec") <<
        " " << (codec_comboBox->get_active_id() == (char*) video_config->get_data("codec") ? "true" : "false") << endl;
    }else{
        cout << endl;
    }
    if(video_config->get_data("codec") && codec_comboBox->get_active_id() != (char*) video_config->get_data("codec")){
        for(auto& row : video_config->get_children()){
            video_config->remove(*row);
        }
    }
    if(!video_config->get_data("codec") || codec_comboBox->get_active_id() != (char*) video_config->get_data("codec")){
        delete[] (char*) video_config->get_data("codec");
        video_config->set_data("codec", (void *) strdup(codec_comboBox->get_active_id().c_str()));
        cout << (char*) video_config->get_data("codec") << endl;

        json bitrate_json;
        bitrate_json["type"] = "entry";
        bitrate_json["default"] = "-1";
        bitrate_json["param"] = "b:v";

        auto bitrate = Gtk::make_managed<Param>("bitrate", bitrate_json);
        video_config->add(*bitrate);

        std::ifstream stream(codecs_file);

        json codecs;
        stream >> codecs;

        if(codecs.find(codec_comboBox->get_active_id()) != codecs.end()){
            json codec = codecs[codec_comboBox->get_active_id()];
            for(const auto& entry : codec.items()){
                auto param = Gtk::make_managed<Param>(entry.key(), entry.value());
                video_config->add(*param);
            }
        }

        video_config->show_all();
    }

    dialog->run();

    dialog->close();
}

int main(int argc, char *argv[])
{
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

    Gtk::Button* config_button = nullptr;
    Form::getInstance().getBuilder()->get_widget("config_button",config_button);

    Gtk::Dialog *dialog = nullptr;
    Form::getInstance().getBuilder()->get_widget("config", dialog);

    dialog->add_button(Gtk::StockID("ok"), 0);

    Gtk::Dialog* settings = nullptr;
    Form::getInstance().getBuilder()->get_widget("Settings", settings);
    settings->add_button(Gtk::StockID("ok"), 0);

    config_button->signal_clicked().connect(sigc::ptr_fun(show_config));
    Gtk::ImageMenuItem* open_button = nullptr;
    Gtk::ImageMenuItem* open_settings = nullptr;
    Form::getInstance().getBuilder()->get_widget("open_button",open_button);
    Form::getInstance().getBuilder()->get_widget("open_settings",open_settings);

    open_button->signal_activate().connect([](){
        auto dialog = Gtk::FileChooserNative::create(
                "Please choose a video file",
                Gtk::FileChooserAction::FILE_CHOOSER_ACTION_OPEN);

        auto filter = Gtk::FileFilter::create();
        filter->set_name("video");
        filter->add_pattern("*.mkv");
        filter->add_pattern("*.avi");
        filter->add_pattern("*.webm");
        filter->add_pattern("*.mp4");
        filter->add_pattern("*.m4v");
        auto filter_any = Gtk::FileFilter::create();
        filter_any->set_name("any");
        filter_any->add_pattern("*");
        dialog->add_filter(filter);
        dialog->add_filter(filter_any);

        dialog->set_select_multiple(true);
        if(dialog->run() == Gtk::RESPONSE_ACCEPT){
            for(auto& file : dialog->get_filenames()){
                add_file(file);
            }
        }
    });

    open_settings->signal_activate().connect([](){
        Gtk::Dialog* dialog = nullptr;
        Form::getInstance().getBuilder()->get_widget("Settings",dialog);
        dialog->run();

        dialog->close();
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