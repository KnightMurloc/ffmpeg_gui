//
// Created by victor on 05.06.2022.
//

#ifndef FFMPEG_GUI_ENTRY_H
#define FFMPEG_GUI_ENTRY_H

#include <gtkmm.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <memory>
#include "lib.h"

using nlohmann::json;

struct EncodeInfo{
    std::string codec;
    std::string hw_codec;
    std::string container;

    std::string path;
};

class Entry : public Gtk::ListBoxRow {
    friend void callback(const FeedBack& feedBack, std::any data);
private:
    static int entry_count;
    std::string full_path;
    json info;

    std::vector<Gtk::CheckButton*> video_streams;
    std::vector<Gtk::CheckButton*> video_audio;

    Gtk::ProgressBar* progressBar;
    Gtk::Entry* file_name_entry;
    Gtk::Image* status_image;
    Gtk::Label* time_label;

//    std::unique_ptr<std::thread> process_thread = nullptr;

    float duration;

    void remove_self();


public:

    static int getEntryCount();

    Entry(std::string& path, json info);

    ~Entry() override {
        entry_count--;
        std::cout << "test destroy" << std::endl;
//        if(process_thread != nullptr)
//            process_thread->join();
    }

//    void process(const std::string codec, const std::string hw_codec, const std::string container);
//    void start(const std::string& codec, const std::string& hw_codec, const std::string& container);

//    const std::string &getFileName() const;

    const std::vector<Gtk::CheckButton *> &getVideoStreams() const;

    const std::vector<Gtk::CheckButton *> &getVideoAudio() const;

    Gtk::ProgressBar *getProgressBar() const;

    float getDuration() const;

    Gtk::Image *getStatusImage() const;
    void process(const EncodeInfo& param);

    Gtk::Label *getTimeLabel() const;
};


#endif //FFMPEG_GUI_ENTRY_H
