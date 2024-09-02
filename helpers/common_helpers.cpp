#include "common_helpers/common_helpers.hpp"
#include "utfcpp/utf8.h"
#include <cwchar>
#include <random>
#include <iterator>
#include <system_error>

// for gmtime_s()
#define __STDC_WANT_LIB_EXT1__ 1
#include <time.h>
#include <utility>


// class KillableWorker
namespace common_helpers {

KillableWorker::KillableWorker(
    std::function<bool(void *)> thread_job,
    std::chrono::milliseconds initial_delay,
    std::chrono::milliseconds polling_time,
    std::function<bool()> should_kill)
{
    this->thread_job = thread_job;
    this->initial_delay = initial_delay;
    this->polling_time = polling_time;
    this->should_kill = should_kill;
}

KillableWorker::~KillableWorker()
{
    kill();
}

KillableWorker& KillableWorker::operator=(const KillableWorker &other)
{
    if (&other == this) {
        return *this;
    }
    
    kill();

    thread_obj = {};
    initial_delay = other.initial_delay;
    polling_time = other.polling_time;
    should_kill = other.should_kill;
    thread_job = other.thread_job;

    return *this;
}

void KillableWorker::thread_proc(void *data)
{
    // wait for some time
    if (initial_delay.count() > 0) {
        std::unique_lock lck(kill_thread_mutex);
        if (kill_thread_cv.wait_for(lck, initial_delay, [this]{ return this->kill_thread || (this->should_kill && this->should_kill()); })) {
            return;
        }
    }

    while (1) {
        if (polling_time.count() > 0) {
            std::unique_lock lck(kill_thread_mutex);
            if (kill_thread_cv.wait_for(lck, polling_time, [this]{ return this->kill_thread || (this->should_kill && this->should_kill()); })) {
                return;
            }
        }

        if (thread_job(data)) { // job is done
            return;
        }
        
    }
}

bool KillableWorker::start(void *data)
{
    if (!thread_job) return false; // no work to do
    if (thread_obj.joinable()) return true; // alrady spawned
    
    kill_thread = false;
    thread_obj = std::thread([this, data] { thread_proc(data); });
    return true;
}

void KillableWorker::kill()
{
    if (!thread_job || !thread_obj.joinable()) return; // already killed
    
    {
        std::lock_guard lk(kill_thread_mutex);
        kill_thread = true;
    }

    kill_thread_cv.notify_one();
    thread_obj.join();
}

}


std::u8string_view common_helpers::filesystem_str(std::string_view str) noexcept
{
    return std::u8string_view(reinterpret_cast<const char8_t* const>(str.data()), str.size());
}

std::u8string common_helpers::filesystem_str(const std::string &str)
{
    return std::u8string(reinterpret_cast<const char8_t* const>(str.data()), str.size());
}

const char* common_helpers::u8str_to_str(const char8_t *u8str)
{
    return reinterpret_cast<const char*>(u8str);
}

std::string_view common_helpers::u8str_to_str(std::u8string_view u8str)
{
    return std::string_view(reinterpret_cast<const char* const>(u8str.data()), u8str.size());
}

std::string common_helpers::u8str_to_str(const std::u8string &u8str)
{
    return std::string(reinterpret_cast<const char* const>(u8str.data()), u8str.size());
}

bool common_helpers::create_dir(const std::filesystem::path &dirpath)
{
    std::error_code err{};
    if (std::filesystem::is_directory(dirpath, err)) {
        return true;
    } else if (std::filesystem::exists(dirpath, err)) {// a file, we can't do anything
        return false;
    }
    
    try {
        return std::filesystem::create_directories(dirpath, err);
    } catch (...) {}

    return false;
}

void common_helpers::write(std::ofstream &file, std::string_view data)
{
    if (!file.is_open()) {
        return;
    }

    file << data << std::endl;
}

std::string common_helpers::uint8_vector_to_hex_string(const std::vector<uint8_t> &v)
{
    std::string result{};
    result.reserve(v.size() * 2);   // two digits per character

    static constexpr const char hex[] = "0123456789ABCDEF";

    for (uint8_t c : v) {
        result.push_back(hex[c / 16]);
        result.push_back(hex[c % 16]);
    }

    return result;
}

void common_helpers::thisThreadYieldFor(std::chrono::microseconds u)
{
    const auto start = std::chrono::high_resolution_clock::now();
    const auto end = start + u;
    do {
        std::this_thread::yield();
    } while (std::chrono::high_resolution_clock::now() < end);
}

void common_helpers::consume_bom(std::ifstream &input)
{
    if (!input.is_open()) return;

    auto pos = input.tellg();
    int bom[3]{};
    bom[0] = input.get();
    bom[1] = input.get();
    bom[2] = input.get();
    if (bom[0] != 0xEF || bom[1] != 0xBB || bom[2] != 0xBF) {
        input.clear();
        input.seekg(pos, std::ios::beg);
    }
}

std::filesystem::path common_helpers::to_absolute(const std::filesystem::path &path, const std::filesystem::path &base)
{
    if (path.is_absolute()) {
        return path;
    }

    try {
        std::error_code err{};
        return std::filesystem::absolute(base / path, err);
    } catch (...) {}
    
    return {};
}

std::filesystem::path common_helpers::to_canonical(const std::filesystem::path &path)
{
    try {
        std::error_code err{};
        return std::filesystem::canonical(path, err);
    } catch (...) {}

    return {};
}

bool common_helpers::file_exist(const std::filesystem::path &filepath)
{
    std::error_code err{};
    if (std::filesystem::is_directory(filepath, err)) {
        return false;
    } else if (std::filesystem::exists(filepath, err)) {
        return true;
    }
    
    return false;
}

bool common_helpers::file_size(const std::filesystem::path &filepath, size_t &size)
{
    if (common_helpers::file_exist(filepath)) {
        std::error_code err{};
        size = static_cast<size_t>(std::filesystem::file_size(filepath, err));
        return true;
    }
    
    size = 0;
    return false;
}

bool common_helpers::dir_exist(const std::filesystem::path &dirpath)
{
    std::error_code err{};
    if (std::filesystem::is_directory(dirpath, err)) {
        return true;
    }
    
    return false;
}

bool common_helpers::remove_file(const std::filesystem::path &filepath)
{
    std::error_code err{};
    if (!std::filesystem::exists(filepath, err)) {
        return true;
    }

    if (std::filesystem::is_directory(filepath, err)) {
        return false;
    }

    return std::filesystem::remove(filepath, err);
}

std::string common_helpers::get_current_dir()
{
    try {
        std::error_code err{};
        return u8str_to_str(std::filesystem::current_path(err).u8string());
    } catch (...) {}

    return {};
}

size_t common_helpers::rand_number(size_t max)
{
    // https://en.cppreference.com/w/cpp/numeric/random/uniform_int_distribution
    std::random_device rd{};  // a seed source for the random number engine
    std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<size_t> distrib(0, max);

    return distrib(gen);
}

std::string common_helpers::get_utc_time()
{
    // https://en.cppreference.com/w/cpp/chrono/c/strftime
    std::time_t time = std::time({});
    std::tm utc_time{};

    bool is_ok = false;
#if defined(__linux__) || defined(linux)
    {
        auto utc_ptr = std::gmtime(&time);
        if (utc_ptr) {
            utc_time = *utc_ptr;
            is_ok = true;
        }
    }
#else
    is_ok = !gmtime_s(&utc_time, &time);
#endif
    std::string time_str(4 +1 +2 +1 +2 +3 +2 +1 +2 +1 +2 +1, '\0');
    if (is_ok) {
        constexpr const static char fmt[] = "%Y/%m/%d - %H:%M:%S";
        size_t chars = std::strftime(&time_str[0], time_str.size(), fmt, &utc_time);
        time_str.resize(chars);
    }
    return time_str;
}

std::wstring common_helpers::to_wstr(std::string_view str)
{
    // test a path like this: "C:\test\命定奇谭ğğğğğÜÜÜÜ"
    if (str.empty() || !utf8::is_valid(str)) {
        return {};
    }

    try {
        std::wstring wstr{};
        utf8::utf8to16(str.begin(), str.end(), std::back_inserter(wstr));
        return wstr;
    }
    catch (...) {}

    return {};
}

std::string common_helpers::to_str(std::wstring_view wstr)
{
    // test a path like this: "C:\test\命定奇谭ğğğğğÜÜÜÜ"
    if (wstr.empty()) {
        return {};
    }

    try {
        std::string str{};
        utf8::utf16to8(wstr.begin(), wstr.end(), std::back_inserter(str));
        return str;
    } catch(...) { }

    return {};
}

std::fstream common_helpers::open_fstream(const std::filesystem::path &filepath, std::ios_base::openmode mode)
{
    return std::fstream( filepath, mode );
}

std::ifstream common_helpers::open_fread(const std::filesystem::path &filepath, std::ios_base::openmode mode)
{
    return std::ifstream( filepath, mode );
}

std::ofstream common_helpers::open_fwrite(const std::filesystem::path &filepath, std::ios_base::openmode mode)
{
    return std::ofstream( filepath, mode );
}
