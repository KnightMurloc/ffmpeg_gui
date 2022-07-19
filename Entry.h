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
#include "gpu_detect/gpu_detect.h"

using nlohmann::json;

struct EncodeInfo{
    std::string codec;
    std::string hw_codec;
    std::string container;

    std::string path;

    std::vector<std::string> extra_options;
    std::string audio_codec;
    std::string audio_bitrate;

    GPU gpu;
};

class AudioStream;

class Entry : public Gtk::ListBoxRow {
    friend void callback(const FeedBack& feedBack, std::any data);
private:
    static int entry_count;
    std::string full_path;

    std::vector<Gtk::CheckButton*> video_streams;
    std::vector<AudioStream*> audio_streams;
    std::vector<AudioStream*> external_audio;

    Gtk::ProgressBar* progressBar;
    Gtk::Entry* file_name_entry;
    Gtk::Image* status_image;
    Gtk::Label* time_label;

//    std::unique_ptr<std::thread> process_thread = nullptr;

    float duration;

    void remove_self();

    void add_audio_stream();

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

    Gtk::ProgressBar *getProgressBar() const;

    float getDuration() const;

    Gtk::Image *getStatusImage() const;
    void process(const EncodeInfo& param);

    Gtk::Label *getTimeLabel() const;
};

class AudioStream : public Gtk::CheckButton {
private:
    bool external = false;
    std::string path;
public:
    AudioStream(const Glib::ustring &label, bool isExternal = false, std::string path = "");

    bool isExternal() const;

    const std::string &getPath() const;

    virtual ~AudioStream();
};

#endif //FFMPEG_GUI_ENTRY_H
