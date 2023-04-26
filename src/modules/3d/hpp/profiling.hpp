#ifndef DS3D_COMMON_HPP_PROFILING_HPP
#define DS3D_COMMON_HPP_PROFILING_HPP

// include all ds3d hpp header files
#include "3d/common/common.h"
#include <sys/time.h>
#include <chrono>

/**
 * @file Gstreamer callback probe (callback function) logic to profile (pipeline FPS, pipeline timing) and interact with
 *  the gstreamer element buffer to (1) FileReader: used to inject data from file into appsrc (if configured with yaml for example) and
 *  (2) FileWriter: take data from GST_BUFFER of an element and write it to file
 *  (view profiling::FileWriter depthWriter in src/application/context.hpp)
 *
 */

namespace ds3d {
namespace profiling {

class FpsCalculation
{
    public:
        FpsCalculation(uint32_t interval) : _max_frame_nums(interval) {}
        float updateFps(uint32_t source_id)
        {
            struct timeval time_now;
            gettimeofday(&time_now, nullptr);
            double now = (double)time_now.tv_sec + time_now.tv_usec / (double)1000000;  // second
            float fps = -1.0f;
            auto iSrc = _timestamps.find(source_id);
            if (iSrc != _timestamps.end()) {
                auto& tms = iSrc->second;
                fps = tms.size() / (now - tms.front());
                while (tms.size() >= _max_frame_nums) {
                    tms.pop();
                }
            } else {
                iSrc = _timestamps.emplace(source_id, std::queue<double>()).first;
            }
            iSrc->second.push(now);

            return fps;
        }

    private:
        std::unordered_map<uint32_t, std::queue<double>> _timestamps;
        uint32_t _max_frame_nums = 50;
};

class Timing
{
    public:
        Timing(uint32_t maxSlots = 50) : _maxTimeLotNum(maxSlots) {}
        ~Timing() = default;
        void push(double t)
        {
            _timelots.push(t);
            _total += t;
            DS_ASSERT(_maxTimeLotNum > 0);
            while (_timelots.size() > _maxTimeLotNum) {
                _total -= _timelots.front();
                _timelots.pop();
            }
        }
        double avg() const
        {
            if (_timelots.size()) {
                return _total / _timelots.size();
            }
            return 0;
        }

    private:
        std::queue<double> _timelots;
        double _total = 0;
        uint32_t _maxTimeLotNum = 0;
};

class FileWriter {
    std::ofstream _file;
    std::string _path;

    public:
        FileWriter() = default;
        ~FileWriter() { close(); }

        bool open(const std::string& path, std::ios::openmode mode = std::ios::out | std::ios::binary)
        {
            _path = path;
            _file.open(path.c_str(), mode);
            return _file.is_open();
        }
        bool isOpen() const { return _file.is_open(); }

        void close()
        {
            if (_file.is_open()) {
                _file.close();
            }
        }

        bool write(const void* buf, size_t size)
        {
            DS_ASSERT(_file.is_open());
            return _file.write((const char*)buf, size).good();
        }
};

class FileReader {
    std::ifstream _file;
    std::string _path;

    public:
        FileReader() = default;
        ~FileReader() { close(); }

        bool open(const std::string& path, std::ios::openmode mode = std::ios::in | std::ios::binary)
        {
            _path = path;
            _file.open(path.c_str(), mode);
            return _file.is_open();
        }
        bool isOpen() const { return _file.is_open(); }
        bool eof() const { return _file.eof(); }

        void close()
        {
            if (_file.is_open()) {
                _file.close();
            }
        }

        int32_t read(void* buf, size_t size)
        {
            DS_ASSERT(_file.is_open());
            if (_file) {
                return (int32_t)_file.readsome((char*)buf, size);
            }
            return -1;
        }
};

}}  // namespace ds3d::profiling

#endif  // DS3D_COMMON_HPP_PROFILING_HPP