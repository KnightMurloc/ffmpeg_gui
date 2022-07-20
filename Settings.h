//
// Created by victor on 11.07.2022.
//

#ifndef FFMPEG_GUI_SETTINGS_H
#define FFMPEG_GUI_SETTINGS_H

#include "gpu_detect/gpu_detect.h"
#include <string>

class Settings{
private:
    GPU gpu;
    std::string ffmpeg_path;
    std::string ffprobe_path;
    bool use_vaapi;

    Settings() = default;
public:
    static Settings& getInstance();

    [[nodiscard]] const GPU &getGpu() const {
        return gpu;
    }

    void setGpu(const GPU &gpu) {
        Settings::gpu = gpu;
    }

    [[nodiscard]] const std::string &getFfmpegPath() const {
        return ffmpeg_path;
    }

    void setFfmpegPath(const std::string &ffmpegPath) {
        ffmpeg_path = ffmpegPath;
    }

    [[nodiscard]] const std::string &getFfprobePath() const {
        return ffprobe_path;
    }

    void setFfprobePath(const std::string &ffprobePath) {
        ffprobe_path = ffprobePath;
    }

    [[nodiscard]] bool isUseVaapi() const {
        return use_vaapi;
    }

    void setUseVaapi(bool useVaapi) {
        use_vaapi = useVaapi;
    }
};



#endif //FFMPEG_GUI_SETTINGS_H
