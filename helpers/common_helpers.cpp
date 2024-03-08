#include "common_helpers/common_helpers.hpp"
#include <fstream>
#include <cwchar>
#include <algorithm>
#include <thread>

static bool create_dir_impl(std::filesystem::path &dirpath)
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

bool common_helpers::create_dir(const std::string &filepath)
{
    std::filesystem::path parent = std::filesystem::path(filepath).parent_path();
    return create_dir_impl(parent);
}

bool common_helpers::create_dir(const std::wstring &filepath)
{
    std::filesystem::path parent = std::filesystem::path(filepath).parent_path();
    return create_dir_impl(parent);
}

void common_helpers::write(std::ofstream &file, const std::string &data)
{
    if (!file.is_open()) {
        return;
    }

    file << data << std::endl;
}

std::wstring common_helpers::str_to_w(const std::string &str)
{
    if (str.empty()) return std::wstring();
    auto cvt_state = std::mbstate_t();
    const char* src = &str[0];
    size_t conversion_bytes = std::mbsrtowcs(nullptr, &src, 0, &cvt_state);
    std::wstring res(conversion_bytes + 1, L'\0');
    std::mbsrtowcs(&res[0], &src, res.size(), &cvt_state);
    return res.substr(0, conversion_bytes);
}

std::string common_helpers::wstr_to_a(const std::wstring &wstr)
{
    if (wstr.empty()) return std::string();
    auto cvt_state = std::mbstate_t();
    const wchar_t* src = &wstr[0];
    size_t conversion_bytes = std::wcsrtombs(nullptr, &src, 0, &cvt_state);
    std::string res(conversion_bytes + 1, '\0');
    std::wcsrtombs(&res[0], &src, res.size(), &cvt_state);
    return res.substr(0, conversion_bytes);
}

bool common_helpers::starts_with_i(const std::string &target, const std::string &query)
{
    return starts_with_i(str_to_w(target), str_to_w(query));
}

bool common_helpers::starts_with_i(const std::wstring &target, const std::wstring &query)
{
    if (target.size() < query.size()) {
        return false;
    }

    auto _target = std::wstring(target);
    auto _query = std::wstring(query);
    std::transform(_target.begin(), _target.end(), _target.begin(),
        [](wchar_t c) { return std::tolower(c); });

    std::transform(_query.begin(), _query.end(), _query.begin(),
        [](wchar_t c) { return std::tolower(c); });

    return _target.compare(0, _query.length(), _query) == 0;

}

bool common_helpers::ends_with_i(const std::string &target, const std::string &query)
{
    return ends_with_i(str_to_w(target), str_to_w(query));
}

bool common_helpers::ends_with_i(const std::wstring &target, const std::wstring &query)
{
    if (target.size() < query.size()) {
        return false;
    }

    auto _target = std::wstring(target);
    auto _query = std::wstring(query);
    std::transform(_target.begin(), _target.end(), _target.begin(),
        [](wchar_t c) { return std::tolower(c); });

    std::transform(_query.begin(), _query.end(), _query.begin(),
        [](wchar_t c) { return std::tolower(c); });

    return _target.compare(_target.length() - _query.length(), _query.length(), _query) == 0;

}

std::string common_helpers::string_strip(const std::string& str)
{
    static constexpr const char whitespaces[] = " \t\r\n";

    size_t start = str.find_first_not_of(whitespaces);
    size_t end = str.find_last_not_of(whitespaces);
    
    if (start == std::string::npos) return {};

    if (start == end) {
        auto c = str[start];
        for (auto c_white = whitespaces; *c_white; ++c_white) {
            if (c == *c_white) return {};
        }
    }

    return str.substr(start, end - start + 1);
}

std::string common_helpers::uint8_vector_to_hex_string(const std::vector<uint8_t>& v)
{
    std::string result{};
    result.reserve(v.size() * 2);   // two digits per character

    static constexpr const char hex[] = "0123456789ABCDEF";

    for (uint8_t c : v)
    {
        result.push_back(hex[c / 16]);
        result.push_back(hex[c % 16]);
    }

    return result;
}

std::string common_helpers::ascii_to_lowercase(std::string data) {
    std::transform(data.begin(), data.end(), data.begin(),
        [](unsigned char c){ return std::tolower(c); });
    return data;
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

std::string common_helpers::to_lower(std::string str)
{
    std::string _str(str.size(), '\0');
    std::transform(str.begin(), str.end(), _str.begin(), [](char c) { return std::tolower(c); });
    return _str;
}

std::wstring common_helpers::to_lower(std::wstring wstr)
{
    std::wstring _wstr(wstr.size(), '\0');
    std::transform(wstr.begin(), wstr.end(), _wstr.begin(), [](wchar_t c) { return std::tolower(c); });
    return _wstr;
}

std::string common_helpers::to_upper(std::string str)
{
    std::string _str(str.size(), '\0');
    std::transform(str.begin(), str.end(), _str.begin(), [](char c) { return std::toupper(c); });
    return _str;
}

std::wstring common_helpers::to_upper(std::wstring wstr)
{
    std::wstring _wstr(wstr.size(), '\0');
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

std::string common_helpers::to_absolute(const std::string &path, const std::string &base)
{
    if (path.empty()) return path;
    auto path_abs = to_absolute_impl(
        std::filesystem::path(path),
        base.empty() ? std::filesystem::current_path() : std::filesystem::path(base)
    );
    return path_abs.u8string();
}

std::wstring common_helpers::to_absolute(const std::wstring &path, const std::wstring &base)
{
    if (path.empty()) return path;
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
    std::filesystem::path path(filepath);
    return file_exist(path);
}

bool common_helpers::file_exist(const std::wstring &filepath)
{
    if (filepath.empty()) return false;
    std::filesystem::path path(filepath);
    return file_exist(path);
}

bool common_helpers::file_size(const std::filesystem::path &filepath, size_t &size)
{
    if (common_helpers::file_exist(filepath)) {
        size = std::filesystem::file_size(filepath);
        return true;
    }
    return false;
}

bool common_helpers::file_size(const std::string &filepath, size_t &size)
{
    const auto file_p = std::filesystem::path(filepath);
    return file_size(file_p, size);
}

bool common_helpers::file_size(const std::wstring &filepath, size_t &size)
{
    const auto file_p = std::filesystem::path(filepath);
    return file_size(file_p, size);
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
    std::filesystem::path path(dirpath);
    return dir_exist(path);
}

bool common_helpers::dir_exist(const std::wstring &dirpath)
{
    if (dirpath.empty()) return false;
    std::filesystem::path path(dirpath);
    return dir_exist(path);
}
