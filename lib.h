//
// Created by victor on 02.06.2022.
//

#ifndef FFMPEG_WRAP_LIB_H
#define FFMPEG_WRAP_LIB_H

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <vector>
#include <functional>
#include <nlohmann/json.hpp>

using nlohmann::json;

enum class FFmpegStatus{
    CONTINUE,
    END
};

class FeedBack;

class FFmpeg{
private:
    std::vector<std::string> args;
    std::vector<std::string> preArgs;
    bool debug = false;
    std::string input;
    std::string output;
    std::function<void(FeedBack)> callback = nullptr;
public:
    [[nodiscard]] static json probe(std::string& file);
    [[nodiscard]] static std::string make_thumbnail(std::string file,const json& info);

    void run();

    [[nodiscard]] bool isDebug() const;

    void setDebug(bool debug);

    [[nodiscard]] const std::string &getInput() const;

    void setInput(const std::string input);

    [[nodiscard]] const std::string &getOutput() const;

    void setOutput(const std::string output);

    [[nodiscard]] const std::function<void(FeedBack)> &getCallback() const;

    void setCallback(const std::function<void(FeedBack)> &callback);

    void addArg(std::string arg);
    void addPreArg(std::string arg);
};

class FeedBack{
private:
    unsigned long frame;
    float fps;
    float stream_0_0_q;
    float bitrate;
    unsigned long total_size;
    unsigned long out_time_us;
    unsigned long out_time_ms;
    std::string out_time;
    unsigned long dup_frames;
    unsigned long drop_frames;
    float speed;
    FFmpegStatus status;
public:
    FeedBack(unsigned long frame, float fps, float stream00Q, float bitrate, unsigned long totalSize,
             unsigned long outTimeUs, unsigned long outTimeMs, const std::string &outTime, unsigned long dupFrames,
             unsigned long dropFrames, float speed, FFmpegStatus status);

    [[nodiscard]] unsigned long getFrame() const;

    [[nodiscard]] float getFps() const;

    [[nodiscard]] float getStream00Q() const;

    [[nodiscard]] float getBitrate() const;

    [[nodiscard]] unsigned long getTotalSize() const;

    [[nodiscard]] unsigned long getOutTimeUs() const;

    [[nodiscard]] unsigned long getOutTimeMs() const;

    [[nodiscard]] const std::string &getOutTime() const;

    [[nodiscard]] unsigned long getDupFrames() const;

    [[nodiscard]] unsigned long getDropFrames() const;

    [[nodiscard]] float getSpeed() const;

    [[nodiscard]] FFmpegStatus getStatus() const;
};
#endif //FFMPEG_WRAP_LIB_H
