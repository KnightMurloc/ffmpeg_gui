#include <gtkmm.h>
#include <string>
#include <iostream>
#include <vector>
#include "lib.h"
#include <nlohmann/json.hpp>
#include <filesystem>
#include "Entry.h"

using nlohmann::json;
using std::string;
using std::cout;
using std::endl;
using std::vector;

const string main_ui = "../main.ui";
Gtk::ListBox* list = nullptr;

class Test : public Gtk::Button{
public:

    Test(const Glib::ustring &label, bool mnemonic = false) : Gtk::Button(label, mnemonic) {
        cout << "new" << endl;
    }

    ~Test() override {
        cout << "destroy" << endl;
    }
};

static void removed(Gtk::ListBoxRow* label){
    cout << "test" << endl;
    list->remove(*label);
//    for(auto label : list->get_children()){
//        cout << label->get_name() << endl;
//    }
}

string url_encode(const string &value) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
        string::value_type c = (*i);

        // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded
        escaped << std::uppercase;
        escaped << '%' << std::setw(2) << int((unsigned char) c);
        escaped << std::nouppercase;
    }

    return escaped.str();
}




void callback(const Glib::RefPtr<Gdk::DragContext>& context, const int& x, const int& y, const Gtk::SelectionData& seldata, const unsigned int& info,const unsigned int& time){
    string file = seldata.get_data_as_string();
    cout << file << endl;

    json probe = FFmpeg::probe(file);


//    Gtk::Box box;
//    box.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
//    Gtk::Image image;
//    box.add(image);
//
//    Gtk::Box center_box;
//    center_box.set_orientation(Gtk::ORIENTATION_VERTICAL);
//    Gtk::Label file_lable(file);
//    Gtk::ProgressBar progressBar;
//    center_box.add(file_lable);
//    center_box.add(progressBar);
//    box.add(center_box);
//
//    list->append(box);

//    Glib::RefPtr<Gtk::Label> box(new Gtk::Label(file));
//
//    list->append(*box.get());
//    auto label = new Gtk::Label(file);
//    Glib::RefPtr<Test> label(new Test(file));

//    Test test("test");

//    auto label = Gtk::make_managed<Test>(file);
//    label->set_name(file);
//    auto row = Gtk::make_managed<Gtk::ListBoxRow>();
//    row->add(*label);
//    label->signal_clicked().connect([&label](){
//        auto test = list->get_children()[0];
//        list->remove(*test);
//    });
//    label->signal_clicked().connect(sigc::bind<typeof(row)>(sigc::ptr_fun(removed), row));
//    sigc::bind(,label);

//    list->

    auto row = Gtk::make_managed<Entry>(file, std::move(probe));

    list->add(*row);
    
    row->show_all();

}

int main(int argc, char *argv[])
{

    auto app =
            Gtk::Application::create(argc, argv,
                                     "org.gtkmm.examples.base");

    Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_file(main_ui);

    Gtk::Window* window = nullptr;
    builder->get_widget("main", window);
    window->show_all();

    window->set_size_request(500,500);

    builder->get_widget("list",list);
    list->drag_dest_set({
        Gtk::TargetEntry("text/plain")
    },Gtk::DestDefaults::DEST_DEFAULT_ALL, Gdk::DragAction::ACTION_COPY);

    list->signal_drag_data_received().connect(sigc::ptr_fun(callback));

//    for (int i = 0; i < 10; ++i) {
//        Gtk::Label* label = new Gtk::Label("test" + std::to_string(i));
//        list->append(*label);
//    }

    window->show_all();

    return app->run(*window);
}