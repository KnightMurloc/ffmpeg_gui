//
// Created by victor on 06.06.2022.
//

#include "Form.h"
#include <string>

using std::string;

const string main_ui = "../main.ui";

Form::Form() {
    builder = Gtk::Builder::create_from_file(main_ui);
}


Form &Form::getInstance() {
    static Form form;
    return form;
}

const Glib::RefPtr<Gtk::Builder> &Form::getBuilder() const {
    return builder;
}

