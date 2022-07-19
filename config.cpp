//
// Created by victor on 14.07.2022.
//

#include "config.h"
#include <iostream>

using std::string;
using std::make_unique;

std::string ConfigComboBox::getValue() {
    return this->get_active_text();
}

std::string ConfigSlider::getValue() {
    return std::to_string(std::round(this->get_value()));
}

std::string ConfigEntry::getValue() {
    return this->get_text();
}

std::string ConfigMap::getValue() {
    return this->get_active_id();
}

const std::string &Param::getName() const {
    return name;
}

std::string Param::getValue() const {
    return value->getValue();
}

Param::Param(string name, json info) : name(name) {

    if(info.find("param") != info.end()){
        param = info["param"];
    }else{
        param = name;
    }

    string type = info["type"];
    auto box = Gtk::make_managed<Gtk::Box>();
    box->set_orientation(Gtk::ORIENTATION_HORIZONTAL);
    auto label = Gtk::make_managed<Gtk::Label>(name);
    label->set_xalign(0);
    box->add(*label);

    if(type == "array"){
        auto combo = Gtk::make_managed<ConfigComboBox>();
        combo->set_hexpand(true);
        for(const string& str : info["values"]){
            combo->append(str);
        }

        if(info.find("default") == info.end()){
            def = "default";
            combo->append("default");
            combo->set_active_text("default");
        }else{
            def = info["default"];
            combo->set_active_text(def);
        }

        box->add(*combo);
        value = static_cast<IConfigValue *>(combo);
        this->add(*box);
        return;
    }

    if(type == "range"){
        auto range = Gtk::make_managed<ConfigSlider>();
//        range->set_hexpand(true);
        int min = info["min"];
        int max = info["max"];
        def = info["default"];
        range->set_digits(0);
        range->set_range(min,max);
        range->set_value(std::strtod(def.c_str(),nullptr));

        box->pack_start(*range);
        value = static_cast<IConfigValue *>(range);
        this->add(*box);
        return;
    }

    if(type == "map"){
        auto combo = Gtk::make_managed<ConfigMap>();
        combo->set_hexpand(true);

        for(const auto& entry : info["values"].items()){
            combo->append(entry.value(),entry.key());
        }

        if(info.find("default") == info.end()){
            def = "default";
            combo->append("default","default");
            combo->set_active_text("default");
        }else{
            def = info["default"];
            combo->set_active_text(def);
        }

        box->add(*combo);
        value = static_cast<IConfigValue *>(combo);
        this->add(*box);
        return;
    }

    if(type == "entry"){
        auto entry = Gtk::make_managed<ConfigEntry>();
        entry->set_hexpand(true);

        def = info["default"];
        entry->set_text(def);

        box->add(*entry);
        value = static_cast<IConfigValue*>(entry);

        this->add(*box);
        return;
    }
}

const string &Param::getParam() const {
    return param;
}

bool Param::isDefault() const {
    return getValue() == def;
}
