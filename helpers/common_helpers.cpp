#include "common_helpers/common_helpers.hpp"
#include "utfcpp/utf8.h"
#include <fstream>
#include <cwchar>
#include <algorithm>
#include <cctype>
#include <random>
#include <iterator>

// for gmtime_s()
#define __STDC_WANT_LIB_EXT1__ 1
#include <time.h>


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

static bool create_dir_impl(const std::filesystem::path &dirpath)
{
    if (std::filesystem::is_directory(dirpath))
    {
        return true;
    }
    else if (std::filesystem::exists(dirpath)) // a file, we can't do anything
    {
        return false;
    }
    
    return std::filesystem::create_directories(dirpath);
}

bool common_helpers::create_dir(std::string_view filepath)
{
    const auto parent(std::filesystem::u8path(filepath).parent_path());
    return create_dir_impl(parent);
}

bool common_helpers::create_dir(std::wstring_view filepath)
{
    const auto parent(std::filesystem::path(filepath).parent_path());
    return create_dir_impl(parent);
}

void common_helpers::write(std::ofstream &file, std::string_view data)
{
    if (!file.is_open()) {
        return;
    }

    file << data << std::endl;
}

bool common_helpers::starts_with_i(std::string_view target, std::string_view query)
{
    if (target.size() < query.size()) {
        return false;
    }

    return str_cmp_insensitive(target.substr(0, query.size()), query);
}

bool common_helpers::starts_with_i(std::wstring_view target, std::wstring_view query)
{
    if (target.size() < query.size()) {
        return false;
    }

    return str_cmp_insensitive(target.substr(0, query.size()), query);
}

bool common_helpers::ends_with_i(std::string_view target, std::string_view query)
{
    if (target.size() < query.size()) {
        return false;
    }

    const auto target_offset = target.length() - query.length();
    return str_cmp_insensitive(target.substr(target_offset), query);
}

bool common_helpers::ends_with_i(std::wstring_view target, std::wstring_view query)
{
    if (target.size() < query.size()) {
        return false;
    }

    const auto target_offset = target.length() - query.length();
    return str_cmp_insensitive(target.substr(target_offset), query);
}

std::string common_helpers::string_strip(std::string_view str)
{
    static constexpr const char whitespaces[] = " \t\r\n";
    
    if (str.empty()) return {};

    size_t start = str.find_first_not_of(whitespaces);
    size_t end = str.find_last_not_of(whitespaces);
    
    if (start == std::string::npos) return {};

    if (start == end) { // happens when string is 1 char
        auto c = str[start];
        for (auto c_white = whitespaces; *c_white; ++c_white) {
            if (c == *c_white) return {};
        }
    }

    return std::string(str.substr(start, end - start + 1));
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

bool common_helpers::str_cmp_insensitive(std::string_view str1, std::string_view str2)
{
    if (str1.size() != str2.size()) return false;

    return std::equal(str1.begin(), str1.end(), str2.begin(), [](const char c1, const char c2){
        return std::toupper(c1) == std::toupper(c2);
    });
}

bool common_helpers::str_cmp_insensitive(std::wstring_view str1, std::wstring_view str2)
{
    if (str1.size() != str2.size()) return false;

    return std::equal(str1.begin(), str1.end(), str2.begin(), [](const wchar_t c1, const wchar_t c2){
        return std::toupper(c1) == std::toupper(c2);
    });
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

std::string common_helpers::to_lower(std::string_view str)
{
    if (str.empty()) return {};
    
    std::string _str(str.size(), '\0');
    std::transform(str.begin(), str.end(), _str.begin(), [](char c) { return std::tolower(c); });
    return _str;
}

std::wstring common_helpers::to_lower(std::wstring_view wstr)
{
    if (wstr.empty()) return {};
    
    std::wstring _wstr(wstr.size(), L'\0');
    std::transform(wstr.begin(), wstr.end(), _wstr.begin(), [](wchar_t c) { return std::tolower(c); });
    return _wstr;
}

std::string common_helpers::to_upper(std::string_view str)
{
    if (str.empty()) return {};

    std::string _str(str.size(), '\0');
    std::transform(str.begin(), str.end(), _str.begin(), [](char c) { return std::toupper(c); });
    return _str;
}

std::wstring common_helpers::to_upper(std::wstring_view wstr)
{
    if (wstr.empty()) return {};
    
    std::wstring _wstr(wstr.size(), L'\0');
    std::transform(wstr.begin(), wstr.end(), _wstr.begin(), [](wchar_t c) { return std::toupper(c); });
    return _wstr;
}

static std::filesystem::path to_absolute_impl(const std::filesystem::path &path, const std::filesystem::path &base)
{
    if (path.is_absolute()) {
        return path;
    }

    return std::filesystem::absolute(base / path);
}

std::string common_helpers::to_absolute(std::string_view path, std::string_view base)
{
    if (path.empty()) return {};
    auto path_abs = to_absolute_impl(
        std::filesystem::u8path(path),
        base.empty() ? std::filesystem::current_path() : std::filesystem::u8path(base)
    );
    return path_abs.u8string();
}

std::wstring common_helpers::to_absolute(std::wstring_view path, std::wstring_view base)
{
    if (path.empty()) return {};
    auto path_abs = to_absolute_impl(
        std::filesystem::path(path),
        base.empty() ? std::filesystem::current_path() : std::filesystem::path(base)
    );
    return path_abs.wstring();
}

bool common_helpers::file_exist(const std::filesystem::path &filepath)
{
    if (std::filesystem::is_directory(filepath)) {
        return false;
    } else if (std::filesystem::exists(filepath)) {
        return true;
    }
    
    return false;
}

bool common_helpers::file_exist(const std::string &filepath)
{
    if (filepath.empty()) return false;
    return file_exist(std::filesystem::u8path(filepath));
}

bool common_helpers::file_exist(const std::wstring &filepath)
{
    if (filepath.empty()) return false;
    return file_exist(std::filesystem::path(filepath));
}

bool common_helpers::file_size(const std::filesystem::path &filepath, size_t &size)
{
    if (common_helpers::file_exist(filepath)) {
        size = static_cast<size_t>(std::filesystem::file_size(filepath));
        return true;
    }
    return false;
}

bool common_helpers::file_size(const std::string &filepath, size_t &size)
{
    return file_size(std::filesystem::u8path(filepath), size);
}

bool common_helpers::file_size(const std::wstring &filepath, size_t &size)
{
    return file_size(std::filesystem::path(filepath), size);
}

bool common_helpers::dir_exist(const std::filesystem::path &dirpath)
{
    if (std::filesystem::is_directory(dirpath)) {
        return true;
    }
    
    return false;
}

bool common_helpers::dir_exist(const std::string &dirpath)
{
    if (dirpath.empty()) return false;
    return dir_exist(std::filesystem::u8path(dirpath));
}

bool common_helpers::dir_exist(const std::wstring &dirpath)
{
    if (dirpath.empty()) return false;
    return dir_exist(std::filesystem::path(dirpath));
}

bool common_helpers::remove_file(const std::filesystem::path &filepath)
{
    if (!std::filesystem::exists(filepath)) {
        return true;
    }

    if (std::filesystem::is_directory(filepath)) {
        return false;
    }

    return std::filesystem::remove(filepath);
}

bool common_helpers::remove_file(const std::string &filepath)
{
    return remove_file(std::filesystem::u8path(filepath));
}

bool common_helpers::remove_file(const std::wstring &filepath)
{
    return remove_file(std::filesystem::path(filepath));
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

std::string common_helpers::str_replace_all(std::string_view source, std::string_view substr, std::string_view replace)
{
    if (source.empty() || substr.empty()) return std::string(source);

    std::string out{};
    out.reserve(source.size() / 4); // out could be bigger or smaller than source, start small

    size_t start_offset = 0;
    auto f_idx = source.find(substr);
    while (std::string::npos != f_idx) {
        // copy the chars before the match
        auto chars_count_until_match = f_idx - start_offset;
        out.append(source, start_offset, chars_count_until_match);
        // copy the replace str
        out.append(replace);

        // adjust the start offset to point at the char after this match
        start_offset = f_idx + substr.size();
        // search for next match
        f_idx = source.find(substr, start_offset);
    }

    // copy last remaining part
    out.append(source, start_offset, std::string::npos);

    return out;
}
