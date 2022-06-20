#include <gtkmm.h>
#include <string>
#include <iostream>
#include <vector>
#include "lib.h"
#include <nlohmann/json.hpp>
#include "Entry.h"
#include "Form.h"
#include "gpu_detect/gpu_detect.h"
using nlohmann::json;
using std::string;
using std::cout;
using std::endl;
using std::vector;


Gtk::ListBox* list = nullptr;

string find_hw_codec(const string& codec){
    GPU gpu = get_gpu_list().front();
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

void start(){
    auto children = list->get_children();

    Gtk::ComboBoxText* codec_comboBox = nullptr;
    Form::getInstance().getBuilder()->get_widget("codec",codec_comboBox);
    Gtk::ComboBoxText* container_comboBox = nullptr;
    Form::getInstance().getBuilder()->get_widget("container",container_comboBox);

    string codec = codec_comboBox->get_active_id();
    string container = container_comboBox->get_active_text();

    string hw_codec = find_hw_codec(codec);
    cout << codec << " " << hw_codec << endl;

    for(auto row : children){
        auto* entry = dynamic_cast<Entry*>(row);
        entry->start(codec,hw_codec, container);
    }
}

void callback(const Glib::RefPtr<Gdk::DragContext>& context, const int& x, const int& y, const Gtk::SelectionData& seldata, const unsigned int& info,const unsigned int& time){
    std::istringstream files(seldata.get_data_as_string());
    string file;
    while(files >> file){
        json probe = FFmpeg::probe(file);
        auto row = Gtk::make_managed<Entry>(file, std::move(probe));
        list->add(*row);
        row->show_all();
    }
}

int main(int argc, char *argv[])
{

    auto app =
            Gtk::Application::create(argc, argv,
                                     "org.gtkmm.examples.base");

//    auto builder = Gtk::Builder::create_from_file(main_ui);

    Gtk::Window* window = nullptr;
    Form::getInstance().getBuilder()->get_widget("main", window);
    window->show_all();

    window->set_size_request(500,500);

    Form::getInstance().getBuilder()->get_widget("list",list);
    list->drag_dest_set({
        Gtk::TargetEntry("text/plain")
    },Gtk::DestDefaults::DEST_DEFAULT_ALL, Gdk::DragAction::ACTION_COPY);

    list->signal_drag_data_received().connect(sigc::ptr_fun(callback));

    Gtk::Button* start_button = nullptr;
    Form::getInstance().getBuilder()->get_widget("start",start_button);

    start_button->signal_clicked().connect(sigc::ptr_fun(start));

    window->show_all();
    int result = app->run(*window);
    delete window;
    return result;
}