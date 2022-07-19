//
// Created by victor on 14.07.2022.
//

#ifndef FFMPEG_GUI_CONFIG_H
#define FFMPEG_GUI_CONFIG_H

#include <gtkmm.h>
#include <string>
#include <memory>
#include <nlohmann/json.hpp>

using namespace nlohmann;

class IConfigValue{
public:
    virtual std::string getValue() = 0;
};

class Param : public Gtk::ListBoxRow {
private:
    std::string name;
    std::string param;
    IConfigValue* value;
    std::string def;

public:
    Param(std::string name, json info);

    const std::string &getName() const;

    std::string getValue() const;

    const std::string &getParam() const;

    bool isDefault() const;
};

class ConfigComboBox : public Gtk::ComboBoxText, public IConfigValue {
public:
    std::string getValue() override;
};


class ConfigSlider : public Gtk::Scale, public IConfigValue {
public:
    std::string getValue() override;
};

class ConfigEntry : public Gtk::Entry, public IConfigValue {
public:
    std::string getValue() override;
};

class ConfigMap : public Gtk::ComboBoxText, public IConfigValue {
public:
    std::string getValue() override;
};

#endif //FFMPEG_GUI_CONFIG_H
