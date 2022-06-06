//
// Created by victor on 05.06.2022.
//

#ifndef FFMPEG_GUI_ENTRY_H
#define FFMPEG_GUI_ENTRY_H

#include <gtkmm.h>
#include <nlohmann/json.hpp>
#include <iostream>

using nlohmann::json;

class Entry : public Gtk::ListBoxRow {
private:
    void remove_self();
public:
    Entry(std::string& path, json info);

    ~Entry() override {
        std::cout << "test destroy" << std::endl;
    }
};


#endif //FFMPEG_GUI_ENTRY_H
