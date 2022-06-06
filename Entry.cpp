//
// Created by victor on 05.06.2022.
//

#include "Entry.h"
#include <string>
#include "lib.h"


using std::string;

Entry::Entry(std::string &path, json info) : Gtk::ListBoxRow(){
    auto box = Gtk::make_managed<Gtk::Box>();
    box->set_spacing(10);
    box->set_orientation(Gtk::ORIENTATION_HORIZONTAL);

    string thumbnail = FFmpeg::make_thumbnail(path,info);

    auto image = Gtk::make_managed<Gtk::Image>(thumbnail);
    box->add(*image);

    auto center_box = Gtk::make_managed<Gtk::Box>();
    center_box->set_orientation(Gtk::ORIENTATION_VERTICAL);
    std::string file_name = path.substr(path.find_last_of("/\\") + 1);
    auto file_lable = Gtk::make_managed<Gtk::Label>(file_name);
    auto progressBar = Gtk::make_managed<Gtk::ProgressBar>();
    center_box->add(*file_lable);
    center_box->add(*progressBar);
    box->add(*center_box);

    json video_stream;
    json audio_stream;
    for(auto stream : info["streams"]){
        if(stream["codec_type"] == "video"){
            video_stream = stream;
            continue;
        }

        if(stream["codec_type"] == "audio"){
            audio_stream = stream;
        }
    }

    string video_codec_name = video_stream["codec_name"];
    int width = video_stream["width"];
    int height = video_stream["height"];

    string audio_codec_name = audio_stream["codec_name"];
    string sample_rate = audio_stream["sample_rate"];

    string info_string =
            "video codec: " + video_codec_name + "\n"
            "width: " + std::to_string(width) + "\n"
            "height: " + std::to_string(height) + "\n"
            "audio codec: " + audio_codec_name + "\n"
            "sample rate: " + sample_rate + "\n";

    auto info_label = Gtk::make_managed<Gtk::Label>(info_string);
    box->add(*info_label);

    auto stock = Gtk::StockID("gtk-close");
//    Gtk::BuiltinIconSize size{id: "button"};
    auto remove_image = Gtk::make_managed<Gtk::Image>(stock, Gtk::IconSize(Gtk::BuiltinIconSize::ICON_SIZE_BUTTON));
    auto remove_button = Gtk::make_managed<Gtk::Button>();
    remove_button->set_valign(Gtk::Align::ALIGN_CENTER);
    remove_button->set_relief(Gtk::RELIEF_NONE);
    remove_button->set_image(*remove_image);
    remove_button->signal_clicked().connect(sigc::mem_fun(this,&Entry::remove_self));

    box->pack_end(*remove_button,false,false);
    this->add(*box);

    box->show_all();
}

void Entry::remove_self() {
    auto parent = this->get_parent();
    parent->remove(*this);
}
