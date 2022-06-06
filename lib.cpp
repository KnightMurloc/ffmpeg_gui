#include "lib.h"
#include <sstream>
#include <hash_fun.h>
#include <filesystem>

using std::stringstream;
using std::string;
using std::filesystem::temp_directory_path;
using std::filesystem::path;

json FFmpeg::probe(string& file) {

    std::array<char, 256> buffer{};
    string cmd = "ffprobe -print_format json -show_streams -show_entries format=duration \"" + file + "\" 2> /dev/null";
    std::cout << cmd << std::endl;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    json result;
    stringstream tmp;
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        tmp << buffer.data();
    }

    tmp >> result;
    return std::move(result);
}

float convertBitrate(stringstream& stream){
    string raw;
    stream >> raw;
    char size = raw[raw.length() - 5];
    raw[raw.length() - 5] = '\0';
    float bitrate = atof(raw.c_str());
    switch (size) {
        case 'k':
            bitrate *= 1024;
            break;
        case 'm':
            bitrate *= 1024 * 1024;
            break;
    }
    return bitrate;
}

void FFmpeg::run() {
    string cmd = "ffmpeg -progress - -y ";
    for(const auto& arg : preArgs){
        cmd += " " + arg;
    }
    cmd += " -i \"" + input + "\"";
    for(const auto& arg : args){
        cmd += " " + arg;
    }

    cmd += " \"" + output + "\"";

    if(!debug){
        cmd += " 2> /dev/null";
    }
//    cmd += " | awk -F \"=\" '{print $2}'";
    std::cout << cmd << std::endl;
    std::array<char, 256> buffer{};
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);

    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    if(this->callback != nullptr) {
        stringstream tmp;
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            std::cout << "test" << std::endl;
//        tmp << buffer.data(); //frame
//
//        fgets(buffer.data(), buffer.size(), pipe.get());
//        tmp << buffer.data(); //fps
//        fgets(buffer.data(), buffer.size(), pipe.get());
//        tmp << buffer.data(); //stream_0_0_q
//        fgets(buffer.data(), buffer.size(), pipe.get());
//        tmp << buffer.data(); //bitrate
//        fgets(buffer.data(), buffer.size(), pipe.get());
//        tmp << buffer.data(); //total_size
//        fgets(buffer.data(), buffer.size(), pipe.get());
//        tmp << buffer.data(); //out_time_us
//        fgets(buffer.data(), buffer.size(), pipe.get());
//        tmp << buffer.data(); //out_time_ms
//        fgets(buffer.data(), buffer.size(), pipe.get());
//        tmp << buffer.data(); //out_time
//        fgets(buffer.data(), buffer.size(), pipe.get());
//        tmp << buffer.data(); //dup_frames
//        fgets(buffer.data(), buffer.size(), pipe.get());
//        tmp << buffer.data(); //drop_frames
//        fgets(buffer.data(), buffer.size(), pipe.get());
//        tmp << buffer.data(); //speed
//        fgets(buffer.data(), buffer.size(), pipe.get());
//        tmp << buffer.data(); //status
//        unsigned long frame;
//        tmp >> frame;
//
//        float fps;
//        tmp>>fps;
//
//        float stream_0_0_q;
//        tmp >> stream_0_0_q;
//
//        float bitrate = convertBitrate(tmp);
//        unsigned long total_size;
//        tmp >> total_size;
//
//        unsigned long out_time_us;
//        tmp >> out_time_us;
//        unsigned long out_time_ms;
//        tmp >> out_time_ms;
//        string out_time;
//        tmp >> out_time;
//
//        unsigned long dup_frames;
//        tmp >> dup_frames;
//        unsigned long drop_frames;
//        tmp >> drop_frames;
//        float speed;
//        string speed_raw;
//        tmp >> speed_raw;
//        speed_raw[speed_raw.length() - 1] = '\0';
//        speed = atof(speed_raw.c_str());
//
//        FFmpegStatus status;
//        string status_raw;
//        tmp >> status_raw;
//        status = status_raw == "continue" ? FFmpegStatus::CONTINUE : FFmpegStatus::END;
//
//        callback(FeedBack(
//                frame,
//                fps,
//                stream_0_0_q,
//                bitrate,
//                total_size,
//                out_time_us,
//                out_time_ms,
//                out_time,
//                dup_frames,
//                drop_frames,
//                speed,
//                status
//                ));
        }
    }
}

bool FFmpeg::isDebug() const {
    return debug;
}

void FFmpeg::setDebug(bool debug) {
    FFmpeg::debug = debug;
}

const string &FFmpeg::getInput() const {
    return input;
}

void FFmpeg::setInput(const string input) {
    FFmpeg::input = input;
}

const string &FFmpeg::getOutput() const {
    return output;
}

void FFmpeg::setOutput(const string output) {
    FFmpeg::output = output;
}

const std::function<void(FeedBack)> &FFmpeg::getCallback() const {
    return callback;
}

void FFmpeg::setCallback(const std::function<void(FeedBack)> &callback) {
    FFmpeg::callback = callback;
}

void FFmpeg::addArg(string arg) {
    this->args.push_back(arg);
}

std::string FFmpeg::make_thumbnail(std::string file,const json& info) {
    string duration_str = info["format"]["duration"];
    float duration = std::atof(duration_str.c_str());
    std::cout << duration << std::endl;

//    int frame_time = (int) ((((double) rand() / (RAND_MAX))) * duration);
    int frame_time = duration / 2;
    const std::hash<std::string> hasher;
    const auto hashResult = hasher(file);
//    path thumbnail_path = temp_directory_path();
    stringstream tmp;
    tmp << temp_directory_path().string() << "/" << std::hex << hashResult << ".png";
    FFmpeg ffmpeg;
    ffmpeg.setInput(file);
    ffmpeg.addPreArg("-ss " + std::to_string(frame_time));
    ffmpeg.addArg("-vf scale=64:64");//TODO возвращать с сохранением пропорций
    ffmpeg.addArg("-vframes 1");
    ffmpeg.setOutput(tmp.str());
    ffmpeg.run();

    return std::move(tmp.str());
}

void FFmpeg::addPreArg(std::string arg) {
    preArgs.push_back(arg);
}


//getters
FeedBack::FeedBack(unsigned long frame, float fps, float stream00Q, float bitrate, unsigned long totalSize,
                   unsigned long outTimeUs, unsigned long outTimeMs, const std::string &outTime,
                   unsigned long dupFrames, unsigned long dropFrames, float speed, FFmpegStatus status)
        : frame(frame),
          fps(fps),
          stream_0_0_q(stream00Q),
          bitrate(bitrate),
          total_size(totalSize),
          out_time_us(outTimeUs),
          out_time_ms(outTimeMs),
          out_time(outTime),
          dup_frames(dupFrames),
          drop_frames(dropFrames),
          speed(speed),
          status(status) {}
unsigned long FeedBack::getFrame() const {
    return frame;
}

float FeedBack::getFps() const {
    return fps;
}

float FeedBack::getStream00Q() const {
    return stream_0_0_q;
}

float FeedBack::getBitrate() const {
    return bitrate;
}

unsigned long FeedBack::getTotalSize() const {
    return total_size;
}

unsigned long FeedBack::getOutTimeUs() const {
    return out_time_us;
}

unsigned long FeedBack::getOutTimeMs() const {
    return out_time_ms;
}

const std::__cxx11::basic_string<char> &FeedBack::getOutTime() const {
    return out_time;
}

unsigned long FeedBack::getDupFrames() const {
    return dup_frames;
}

unsigned long FeedBack::getDropFrames() const {
    return drop_frames;
}

float FeedBack::getSpeed() const {
    return speed;
}

FFmpegStatus FeedBack::getStatus() const {
    return status;
}