//
// Created by victor on 05.06.2022.
//

#include "Entry.h"
#include <string>
#include "lib.h"
#include "Form.h"
#include <thread>
#include <memory>
#include <filesystem>

using std::string;
using std::thread;
using std::any;
using std::unique_ptr;
using std::pair;
using std::make_pair;
using std::make_unique;

int Entry::entry_count = 0;

Entry::Entry(std::string &path, json info) : Gtk::ListBoxRow(){
    entry_count++;
    auto box = Gtk::make_managed<Gtk::Box>();
    box->set_spacing(10);
    box->set_orientation(Gtk::ORIENTATION_HORIZONTAL);
//    auto left_box = Gtk::make_managed<Gtk::Box>();
//    left_box->set_orientation(Gtk::ORIENTATION_VERTICAL);

//    string thumbnail = FFmpeg::make_thumbnail(path,info);
//    string thumbnail = "";
    status_image = Gtk::make_managed<Gtk::Image>();

    auto image = Gtk::make_managed<Gtk::Image>();
//    left_box->add(*image);
//    auto config_button = Gtk::make_managed<Gtk::Button>("config");
//    left_box->pack_end(*config_button,false,false);
//    config_button->signal_clicked().connect(sigc::mem_fun(this,&Entry::show_config));

    box->add(*status_image);
    box->add(*image);

    auto center_box = Gtk::make_managed<Gtk::Box>();
    center_box->set_orientation(Gtk::ORIENTATION_VERTICAL);
    std::string file_name = path.substr(path.find_last_of("/\\") + 1);
    file_name = file_name.substr(0,file_name.find_last_of('.'));
    file_name_entry = Gtk::make_managed<Gtk::Entry>();
    file_name_entry->set_text(file_name);
    progressBar = Gtk::make_managed<Gtk::ProgressBar>();

    time_label = Gtk::make_managed<Gtk::Label>();

    center_box->add(*file_name_entry);
    center_box->add(*time_label);
    center_box->add(*progressBar);
    box->add(*center_box);

//    json video_stream;
//    json audio_stream;
//    for(auto stream : info["streams"]){
//        if(stream["codec_type"] == "video"){
//            video_stream = stream;
//            continue;
//        }
//
//        if(stream["codec_type"] == "audio"){
//            audio_stream = stream;
//        }
//    }

//    string video_codec_name = video_stream["codec_name"];
//    int width = video_stream["width"];
//    int height = video_stream["height"];
//
//    string audio_codec_name = audio_stream["codec_name"];
//    string sample_rate = audio_stream["sample_rate"];

//    string info_string =
//            "video codec: " + video_codec_name + "\n"
//            "width: " + std::to_string(width) + "\n"
//            "height: " + std::to_string(height) + "\n"
//            "audio codec: " + audio_codec_name + "\n"
//            "sample rate: " + sample_rate + "\n";

//    auto info_label = Gtk::make_managed<Gtk::Label>(info_string);

//    box->add(*info_label);

//    auto model = new Gtk::ListStore()
//
//    auto videos = Gtk::make_managed<Gtk::ComboBox>();
//    auto row = Gtk::make_managed<Gtk::CellRendererToggle>();
//    auto test = Gtk::make_managed<Gtk::CheckButton>("test");
//    auto test2 = Gtk::make_managed<Gtk::CheckButton>("test2");
//    auto test3 = Gtk::make_managed<Gtk::CheckButton>("test4");

    auto right_box = Gtk::make_managed<Gtk::Box>();
    right_box->set_orientation(Gtk::ORIENTATION_VERTICAL);
    right_box->set_spacing(5);
    right_box->set_homogeneous(true);

    int count = 0;
    for(auto stream : info["streams"]){
        count++;
        if(stream["codec_type"] == "video"){
            string title = std::to_string(count) + ". ";
            if(stream.find("tags") != stream.end() && stream["tags"].find("title") != stream["tags"].end()){
                title += stream["tags"]["title"];
            }
            title += " (" + ((string) stream["codec_name"]) + ")";

            auto name = Gtk::make_managed<Gtk::CheckButton>(title);
            name->set_active(true);

//            video_list->add(*name);
            video_streams.push_back(name);
            continue;
        }

        if(stream["codec_type"] == "audio"){
            string title = std::to_string(count) + ". ";
            if(stream.find("tags") != stream.end() && stream["tags"].find("title") != stream["tags"].end()){
                title += stream["tags"]["title"];
            }
            title += " (" + ((string) stream["codec_name"]) + ")";

            auto name = Gtk::make_managed<Gtk::CheckButton>(title);
            name->set_active(true);

//            audio_list->add(*name);
            audio_streams.push_back(name);
        }
    }

    if(video_streams.size() == 1){
        right_box->add(*video_streams[0]);
    }else{
        auto video_scroll = Gtk::make_managed<Gtk::ScrolledWindow>();
        video_scroll->set_hexpand(true);
        auto video_list = Gtk::make_managed<Gtk::ListBox>();

        for(auto stream : video_streams){
            video_list->add(*stream);
        }

        video_scroll->add(*video_list);
        right_box->add(*video_scroll);
    }
    auto button_add = Gtk::make_managed<Gtk::Button>(Gtk::StockID("gtk-add"));
    auto box_audio = Gtk::make_managed<Gtk::Box>();
    if(audio_streams.size() == 1){
        box_audio->add(*audio_streams[0]);
    }else{
        auto audio_scroll = Gtk::make_managed<Gtk::ScrolledWindow>();
        audio_scroll->set_hexpand(true);
        auto audio_list = Gtk::make_managed<Gtk::ListBox>();

        for(auto stream : audio_streams){
            audio_list->add(*stream);
        }

        audio_scroll->add(*audio_list);
        box_audio->add(*audio_scroll);
    }
    box_audio->add(*button_add);
    right_box->add(*box_audio);

    box->add(*right_box);



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
    std::setlocale (LC_NUMERIC,"en_US.utf-8");
    string duration_str = info["format"]["duration"];
    this->duration = std::strtof(duration_str.c_str(), nullptr);
    std::setlocale (LC_NUMERIC,"");
    this->info = std::move(info);
//    this->file_name = std::move(file_name);
    this->full_path = path;
}

void Entry::remove_self() {
    auto parent = this->get_parent();
    parent->remove(*this);
    delete this;
}

//const string &Entry::getFileName() const {
//    return file_name;
//}

const std::vector<Gtk::CheckButton *> &Entry::getVideoStreams() const {
    return video_streams;
}

const std::vector<Gtk::CheckButton *> &Entry::getVideoAudio() const {
    return audio_streams;
}

Gtk::ProgressBar *Entry::getProgressBar() const {
    return progressBar;
}

//void Entry::start(const string &codec, const string &hw_codec, const string &container) {
////    std::cout << "start" << std::endl;
////    thread th(&Entry::process,this, codec, hw_codec, container);
//    if(this->process_thread.get() != nullptr){
//        this->process_thread->join();
//    }
//    this->process_thread = make_unique<thread>(&Entry::process,this, codec, hw_codec, container);
//}

bool update_progress_bar(pair<Entry*, double>* data){
//    std::cout << data->second << std::endl;
    Gtk::ProgressBar* bar = nullptr;
    Form::getInstance().getBuilder()->get_widget("progress",bar);
    double progress = bar->get_fraction();
    progress += (data->second - data->first->getProgressBar()->get_fraction()) / Entry::getEntryCount();
    bar->set_fraction(progress);

    data->first->getProgressBar()->set_fraction(data->second);
    if(data->second == 1){
        data->first->getStatusImage()->set_from_icon_name("gtk-ok",Gtk::IconSize(Gtk::BuiltinIconSize::ICON_SIZE_BUTTON));
    }

    delete data;
    return false;
}

bool update_time_info(pair<Entry*, string>* data){

    data->first->getTimeLabel()->set_text(data->second);

    delete data;

    return false;
}

void callback(const FeedBack& feedBack, any data){
    auto* entry = std::any_cast<Entry*>(data);
    double progress = ((double) feedBack.getOutTimeMs() / 1000000.0) / entry->getDuration();
    double time_left = (entry->getDuration() - (double) feedBack.getOutTimeMs() / 1000000.0) / feedBack.getSpeed();


    int seconds = (int) time_left;
    int hr=(int)(seconds/3600);
    int min=((int)(seconds/60))%60;
    int sec=(int)(seconds%60);

    string text;
    text += std::to_string(hr);
    text += ":";
    text += std::to_string(min);
    text += ":";
    text += std::to_string(sec);

    if(feedBack.getStatus() == FFmpegStatus::END){
        progress = 1;
    }
    g_idle_add((GSourceFunc) update_progress_bar,(void*) new pair(entry,progress));
    g_idle_add((GSourceFunc) update_time_info,(void*) new pair(entry,text));

}

void Entry::process(const EncodeInfo& param) {
    string path = param.path.empty() ? std::filesystem::path(this->full_path).parent_path().string() : param.path;
    if(path.back() != '/'){
        path += '/';
    }
    string file = file_name_entry->get_text();

    FFmpeg ffmpeg;

    if(param.gpu.vendor == Vendor::AMD){
        ffmpeg.addPreArg("-vaapi_device /dev/dri/renderD128");
    }

    ffmpeg.setInput(this->full_path);
    ffmpeg.setOutput(path + file + "." + param.container);
    ffmpeg.addArg("-map 0");
    for(size_t i = 0; i < audio_streams.size(); i++){
        auto& stream = audio_streams[i];
        if(!stream->get_active()){
            ffmpeg.addArg("-map -0:a:" + std::to_string(i));
        }
    }

    ffmpeg.addArg("-c:a " + param.audio_codec);
    if(!param.audio_bitrate.empty()){
        ffmpeg.addArg("-b:a " + param.audio_bitrate);
    }
    ffmpeg.addArg("-c:v " + param.hw_codec); //first try hw codec
    if(!param.video_bitrate.empty()){
        ffmpeg.addArg("-b:v " + param.video_bitrate);
    }

    if(param.gpu.vendor == Vendor::AMD){
        ffmpeg.addArg("-vf format=\"nv12|vaapi,hwupload\"");
    }

    ffmpeg.setCallback(callback,this);
    //if hw are failed
    if(ffmpeg.run()){
        std::cout << "fallback to software" << std::endl;
        ffmpeg = FFmpeg();
        ffmpeg.setInput(this->full_path);
        ffmpeg.setOutput(path + file + "." + param.container);
        ffmpeg.addArg("-map 0");
        for(size_t i = 0; i < audio_streams.size(); i++){
            auto& stream = audio_streams[i];
            if(!stream->get_active()){
                ffmpeg.addArg("-map -0:a:" + std::to_string(i));
            }
        }

        ffmpeg.addArg("-c:a " + param.audio_codec);
        if(!param.audio_bitrate.empty()){
            ffmpeg.addArg("-b:a " + param.audio_bitrate);
        }
        ffmpeg.addArg("-c:v " + param.codec);
        if(!param.video_bitrate.empty()){
            ffmpeg.addArg("-b:v " + param.video_bitrate);
        }
        ffmpeg.setCallback(callback,this);
        ffmpeg.run();
    }//TODO check if software are failed and show error
}

float Entry::getDuration() const {
    return duration;
}

Gtk::Image *Entry::getStatusImage() const {
    return status_image;
}

int Entry::getEntryCount() {
    return entry_count;
}

Gtk::Label *Entry::getTimeLabel() const {
    return time_label;
}
