#include "lib.h"
#include <sstream>
#include <hash_fun.h>
#include <filesystem>

using std::stringstream;
using std::string;
using std::filesystem::temp_directory_path;
using std::filesystem::path;
using std::any;

json FFmpeg::probe(const string& file) {

    std::array<char, 256> buffer{};
    //TODO get ffprobe path from settings
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
    return result;
}

int FFmpeg::run() {
    string cmd = ffmpeg_bin + " -hide_banner -loglevel warning -progress - -y ";
    for(const auto& arg : preArgs){
        cmd += " " + arg;
    }
    for(auto& file : input){
        cmd += " -i \"" + file + "\"";
    }
//    cmd += " -i \"" + input + "\"";
    for(const auto& arg : args){
        cmd += " " + arg;
    }

    cmd += " \"" + output + "\"";
    std::cout << err_file.empty() << std::endl;
    if(!err_file.empty()){
        cmd += " 2> " + err_file;
    }else if(!debug){
        cmd += " 2> /dev/null";
    }
//    cmd += " | awk -F \"=\" '{print $2}'";
    std::cout << cmd << std::endl;
    std::array<char, 256> buffer{};
    FILE* pipe = popen(cmd.c_str(), "r");

    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    if(this->callback != nullptr) {
        stringstream tmp;
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
//            std::cout << "test" << std::endl;
            std::setlocale (LC_NUMERIC,"en_US.utf-8");
            unsigned int frame;
            sscanf(buffer.data(),"frame=%u",&frame);
            fgets(buffer.data(), buffer.size(), pipe);

            float fps;
            sscanf(buffer.data(),"fps=%f",&fps);
            fgets(buffer.data(), buffer.size(), pipe);

            float stream_0_0_q;
            sscanf(buffer.data(),"stream_0_0_q=%f",&stream_0_0_q);
            fgets(buffer.data(), buffer.size(), pipe);

            float bitrate;
            char size;
            sscanf(buffer.data(),"bitrate=%f%c",&stream_0_0_q,&size);
            fgets(buffer.data(), buffer.size(), pipe);
            switch (size) {
                case 'k':
                    bitrate *= 1024;
                    break;
                case 'm':
                    bitrate *= 1024 * 1024;
                    break;
            }

            unsigned long total_size;
            sscanf(buffer.data(),"total_size=%lu",&total_size);
            fgets(buffer.data(), buffer.size(), pipe);

            unsigned long out_time_us;
            sscanf(buffer.data(),"out_time_us=%lu",&out_time_us);
            fgets(buffer.data(), buffer.size(), pipe);

            unsigned long out_time_ms;
            sscanf(buffer.data(),"out_time_ms=%lu",&out_time_ms);
            fgets(buffer.data(), buffer.size(), pipe);

            std::string out_time(64,'\0');
            sscanf(buffer.data(),"out_time_ms=%s",out_time.data());
            fgets(buffer.data(), buffer.size(), pipe);

            unsigned long dup_frames;
            sscanf(buffer.data(),"dup_frames=%lu",&dup_frames);
            fgets(buffer.data(), buffer.size(), pipe);

            unsigned long drop_frames;
//            std::cout << buffer.data() << std::endl;
            sscanf(buffer.data(),"dup_frames=%lu",&drop_frames);
            fgets(buffer.data(), buffer.size(), pipe);

            float speed;
//            std::cout << buffer.data() << std::endl;
            sscanf(buffer.data(),"speed=%f",&speed);
            fgets(buffer.data(), buffer.size(), pipe);

            string status(16,'\0');
            sscanf(buffer.data(),"progress=%s",status.data());
//            std::cout << status << std::endl;
//            std::cout << (strcmp(status.c_str(),"end") == 0) << std::endl;
            std::setlocale (LC_NUMERIC,"");
            callback(FeedBack(
                    frame,
                    fps,
                    stream_0_0_q,
                    bitrate,
                    total_size,
                    out_time_us,
                    out_time_ms,
                    out_time,
                    dup_frames,
                    drop_frames,
                    speed,
                    (strcmp(status.c_str(),"end") == 0) ? FFmpegStatus::END : FFmpegStatus::CONTINUE
                    ), data);
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

    return pclose(pipe);
}

bool FFmpeg::isDebug() const {
    return debug;
}

void FFmpeg::setDebug(bool debug) {
    FFmpeg::debug = debug;
}

const string &FFmpeg::getOutput() const {
    return output;
}

void FFmpeg::setOutput(const string output) {
    FFmpeg::output = output;
}

const std::function<void(const FeedBack&, std::any)> &FFmpeg::getCallback() const {
    return callback;
}

void FFmpeg::setCallback(const std::function<void(const FeedBack&, std::any)> &callback, any data) {
    FFmpeg::callback = callback;
    this->data = data;
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
    ffmpeg.addInput(file);
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

bool FFmpeg::heave_hwaccel(std::string& name) {
    return name == "h264" ||
           name == "hevc" ||
           name == "mjpeg" ||
           name == "mpeg2" ||
           name == "vp8" ||
           name == "vp9";
}

void FFmpeg::addInput(const string& file) {
    input.push_back(file);
}

const string &FFmpeg::getFfmpegBin() const {
    return ffmpeg_bin;
}

void FFmpeg::setFfmpegBin(const string &ffmpegBin) {
    ffmpeg_bin = ffmpegBin;
}

const string &FFmpeg::getFfprobeBin() const {
    return ffprobe_bin;
}

void FFmpeg::setFfprobeBin(const string &ffprobeBin) {
    ffprobe_bin = ffprobeBin;
}

const string &FFmpeg::getErrFile() const {
    return err_file;
}

void FFmpeg::setErrFile(const string &errFile) {
    err_file = errFile;
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