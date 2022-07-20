//
// Created by victor on 06.06.2022.
//

#include "Form.h"
#include <string>

#include "build_config.h"

using std::string;

Form::Form() {
    builder = Gtk::Builder::create_from_file(Glib::canonicalize_filename(main_ui,std::getenv("APPDIR")));
}


Form &Form::getInstance() {
    static Form form;
    return form;
}

const Glib::RefPtr<Gtk::Builder> &Form::getBuilder() const {
    return builder;
}

