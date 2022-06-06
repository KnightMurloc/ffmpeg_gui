#include <gtkmm.h>
#include <string>
#include <iostream>
#include <vector>
#include "lib.h"
#include <nlohmann/json.hpp>
#include "Entry.h"

using nlohmann::json;
using std::string;
using std::cout;
using std::endl;
using std::vector;

const string main_ui = "../main.ui";
Gtk::ListBox* list = nullptr;

void callback(const Glib::RefPtr<Gdk::DragContext>& context, const int& x, const int& y, const Gtk::SelectionData& seldata, const unsigned int& info,const unsigned int& time){
    string file = seldata.get_data_as_string();
    cout << file << endl;

    json probe = FFmpeg::probe(file);

    auto row = Gtk::make_managed<Entry>(file, std::move(probe));

    list->add(*row);
    
    row->show_all();

}

int main(int argc, char *argv[])
{

    auto app =
            Gtk::Application::create(argc, argv,
                                     "org.gtkmm.examples.base");

    auto builder = Gtk::Builder::create_from_file(main_ui);

    Gtk::Window* window = nullptr;
    builder->get_widget("main", window);
    window->show_all();

    window->set_size_request(500,500);

    builder->get_widget("list",list);
    list->drag_dest_set({
        Gtk::TargetEntry("text/plain")
    },Gtk::DestDefaults::DEST_DEFAULT_ALL, Gdk::DragAction::ACTION_COPY);

    list->signal_drag_data_received().connect(sigc::ptr_fun(callback));

    window->show_all();
    int result = app->run(*window);
    delete window;
    return result;
}