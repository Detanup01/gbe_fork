#include "common_helpers/common_helpers.hpp"
#include <fstream>
#include <filesystem>
#include <cwchar>
#include <algorithm>

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
    auto cvt_state = std::mbstate_t();
    const char* src = &str[0];
    size_t conversion_bytes = std::mbsrtowcs(nullptr, &src, 0, &cvt_state);
    std::wstring res(conversion_bytes + 1, L'\0');
    std::mbsrtowcs(&res[0], &src, res.size(), &cvt_state);
    return res.substr(0, conversion_bytes);
}

std::string common_helpers::wstr_to_a(const std::wstring &wstr)
{
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
