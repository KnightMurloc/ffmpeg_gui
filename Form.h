//
// Created by victor on 06.06.2022.
//

#ifndef FFMPEG_GUI_FORM_H
#define FFMPEG_GUI_FORM_H

#include <gtkmm.h>

class Form {
private:
    Glib::RefPtr<Gtk::Builder> builder;

    Form();
public:
    static Form& getInstance();

    const Glib::RefPtr<Gtk::Builder> &getBuilder() const;
};


#endif //FFMPEG_GUI_FORM_H
