//
// Created by victor on 20.07.2022.
//

#include "Settings.h"

Settings &Settings::getInstance() {
    static Settings settings;
    return settings;
}