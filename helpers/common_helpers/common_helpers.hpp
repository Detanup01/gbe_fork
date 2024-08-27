#pragma once

#include <cstdlib>
#include <string>
#include <string_view>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <vector>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cctype>
#include <algorithm>
#include <concepts>
#include <type_traits>


// class KillableWorker
namespace common_helpers {

class KillableWorker
{
private:
    std::thread thread_obj{};

    // don't run immediately, wait some time
    std::chrono::milliseconds initial_delay{};
    // time between each invokation
    std::chrono::milliseconds polling_time{};

    std::function<bool()> should_kill{};

    std::function<bool(void *)> thread_job;

    std::mutex kill_thread_mutex{};
    std::condition_variable kill_thread_cv{};
    bool kill_thread{};
    
    void thread_proc(void *data);

public:
    KillableWorker(
        std::function<bool(void *)> thread_proc = {},
        std::chrono::milliseconds initial_delay = {},
        std::chrono::milliseconds polling_time = {},
        std::function<bool()> should_kill = {});
    ~KillableWorker();

    KillableWorker& operator=(const KillableWorker &other);

    // spawn the thread if necessary
    bool start(void *data = nullptr);
    // kill the thread if necessary
    void kill();
};

}


// helper c++ concepts for templates
namespace common_helpers {

template<typename T>
using remove_cvrefptr = std::remove_cvref< std::remove_pointer_t<T> >;
template<typename T>
using remove_cvrefptr_t = remove_cvrefptr<T>::type;

template <typename TString>
concept BsicStringable = requires {
    typename TString::value_type;
} && (
    std::is_same_v< std::basic_string<typename TString::value_type>, TString > ||
    std::is_same_v< std::basic_string_view<typename TString::value_type>, TString >
);

// (const char*)"str"
template <typename TString>
concept StaticPtrStringable =
    std::is_pointer_v<TString> &&
    !std::is_null_pointer_v<TString> &&
    std::is_integral_v< remove_cvrefptr_t<TString> >;

// (const char[4])"str"
template <typename TString>
concept StaticArrayStringable =
    std::is_array_v< std::remove_cvref_t<TString> > &&
    std::is_integral_v< std::remove_extent_t< std::remove_cvref_t<TString> > >;

template <typename TString>
concept StaticStringable = StaticPtrStringable<TString> || StaticArrayStringable<TString>;

template <typename TString>
concept StringViewable = BsicStringable<TString> || StaticStringable<TString>;

template<typename T>
struct str_info;

template<BsicStringable TString>
struct str_info<TString>
{
    using char_type = TString::value_type;
};

template<StaticStringable TString>
struct str_info<TString>
{
    using char_type = std::remove_extent_t< remove_cvrefptr_t<TString> >;
};

template <typename TString>
using str_info_t = str_info<TString>::char_type;

}


namespace common_helpers {

std::u8string_view filesystem_str(std::string_view str) noexcept;
std::u8string filesystem_str(const std::string &str);
template<typename TChar>
    requires (!std::is_same_v<TChar, char>)
constexpr std::basic_string_view<TChar> filesystem_str(std::basic_string_view<TChar> str)
{
    // all other types are fine, only char has to be casted to char8_t
    return str;
}
template<typename TChar>
    requires (!std::is_same_v<TChar, char>)
constexpr std::basic_string<TChar> filesystem_str(const std::basic_string<TChar> &str)
{
    // all other types are fine, only char has to be casted to char8_t
    return std::basic_string<TChar>(str);
}

std::string_view u8str_to_str(std::u8string_view u8str) noexcept;
std::string u8str_to_str(const std::u8string &u8str);

template<StringViewable TString, typename TChar = str_info_t<TString>>
constexpr std::filesystem::path std_fs_path(const TString &tpath)
{
    std::basic_string_view<TChar> path(tpath);
    return std::filesystem::path(filesystem_str(path));
}

bool create_dir(const std::filesystem::path &dirpath);
template<StringViewable TString, typename TChar = str_info_t<TString>>
constexpr bool create_dir(const TString &tfilepath)
{
    std::basic_string_view<TChar> filepath(tfilepath);
    const auto parent(std_fs_path(filepath).parent_path());
    return create_dir(parent);
}

void write(std::ofstream &file, std::string_view data);

template<StringViewable TString1, StringViewable TString2, typename TChar = str_info_t<TString1>>
    requires std::is_same_v<TChar, str_info_t<TString2>>
bool str_cmp_insensitive(const TString1 &tstr1, const TString2 &tstr2)
{
    std::basic_string_view<TChar> str1(tstr1);
    std::basic_string_view<TChar> str2(tstr2);
    if (str1.size() != str2.size()) return false;

    return std::equal(str1.begin(), str1.end(), str2.begin(), [](const TChar c1, const TChar c2){
        return std::toupper(c1) == std::toupper(c2);
    });
}

template<StringViewable TStringT, StringViewable TStringQ, typename TChar = str_info_t<TStringT>>
    requires std::is_same_v<TChar, str_info_t<TStringQ>>
bool starts_with_i(const TStringT &ttarget, const TStringQ &tquery)
{
    std::basic_string_view<TChar> target(ttarget);
    std::basic_string_view<TChar> query(tquery);
    if (target.size() < query.size()) {
        return false;
    }

    return str_cmp_insensitive(target.substr(0, query.size()), query);
}

template<StringViewable TStringT, StringViewable TStringQ, typename TChar = str_info_t<TStringT>>
    requires std::is_same_v<TChar, str_info_t<TStringQ>>
bool ends_with_i(const TStringT &ttarget, const TStringQ &tquery)
{
    std::basic_string_view<TChar> target(ttarget);
    std::basic_string_view<TChar> query(tquery);
    if (target.size() < query.size()) {
        return false;
    }

    const auto target_offset = target.length() - query.length();
    return str_cmp_insensitive(target.substr(target_offset), query);
}

template<StringViewable TString, typename TChar = str_info_t<TString>>
std::basic_string<TChar> string_strip(const TString &tstr)
{
    std::basic_string_view<TChar> str(tstr);
    if (str.empty()) return {};

    static constexpr const char whitespaces[] = " \t\r\n";
    
    const auto start = str.find_first_not_of(whitespaces);
    const auto end = str.find_last_not_of(whitespaces);
    
    if (std::basic_string<TChar>::npos == start) return {};

    if (start == end) { // happens when string is 1 char
        auto c = str[start];
        for (auto c_white = whitespaces; *c_white; ++c_white) {
            if (c == *c_white) return {};
        }
    }

    return std::basic_string<TChar>(str.substr(start, end - start + 1));
}

std::string uint8_vector_to_hex_string(const std::vector<uint8_t> &v);

void thisThreadYieldFor(std::chrono::microseconds u);

void consume_bom(std::ifstream &input);

template<StringViewable TString, typename TChar = str_info_t<TString>>
std::basic_string<TChar> to_lower(const TString &tstr)
{
    std::basic_string_view<TChar> str(tstr);
    if (str.empty()) return {};

    std::basic_string<TChar> _str(str.size(), 0);
    std::transform(str.begin(), str.end(), _str.begin(), [](TChar c) { return std::tolower(c); });
    return _str;
}

template<StringViewable TString, typename TChar = str_info_t<TString>>
std::basic_string<TChar> to_upper(const TString &tstr)
{
    std::basic_string_view<TChar> str(tstr);
    if (str.empty()) return {};

    std::basic_string<TChar> _str(str.size(), 0);
    std::transform(str.begin(), str.end(), _str.begin(), [](TChar c) { return std::toupper(c); });
    return _str;
}

std::filesystem::path to_absolute(const std::filesystem::path &path, const std::filesystem::path &base);
template<StringViewable TStringP, StringViewable TStringB, typename TChar = str_info_t<TStringP>>
    requires std::is_same_v<TChar, str_info_t<TStringB>>
std::basic_string<TChar> to_absolute(const TStringP &tpath, const TStringB &tbase = "")
{
    std::basic_string_view<TChar> path(tpath);
    std::basic_string_view<TChar> base(tbase);
    if (path.empty()) return {};
    std::filesystem::path path_abs = to_absolute(
        std_fs_path(path),
        base.empty() ? std::filesystem::current_path() : std_fs_path(base)
    );
    return u8str_to_str(path_abs.u8string());
}

std::filesystem::path to_canonical(const std::filesystem::path &path);
template<StringViewable TString, typename TChar = str_info_t<TString>>
std::basic_string<TChar> to_canonical(const TString &tpath)
{
    std::basic_string_view<TChar> path(tpath);
    std::filesystem::path path_canon = to_canonical(std_fs_path(path));
    return u8str_to_str( path_canon.u8string() );
}

bool file_exist(const std::filesystem::path &filepath);
template<StringViewable TString, typename TChar = str_info_t<TString>>
bool file_exist(const TString &tfilepath)
{
    std::basic_string_view<TChar> filepath(tfilepath);
    if (filepath.empty()) return false;
    return file_exist(std_fs_path(filepath));
}

bool file_size(const std::filesystem::path &filepath, size_t &size);
template<StringViewable TString, typename TChar = str_info_t<TString>>
bool file_size(const TString &tfilepath, size_t &size)
{
    std::basic_string_view<TChar> filepath(tfilepath);
    return file_size(std_fs_path(filepath), size);
}

bool dir_exist(const std::filesystem::path &dirpath);
template<StringViewable TString, typename TChar = str_info_t<TString>>
bool dir_exist(const TString &tdirpath)
{
    std::basic_string_view<TChar> dirpath(tdirpath);
    if (dirpath.empty()) return false;
    return dir_exist(std_fs_path(dirpath));
}

bool remove_file(const std::filesystem::path &filepath);
template<StringViewable TString, typename TChar = str_info_t<TString>>
bool remove_file(const TString &tfilepath)
{
    std::basic_string_view<TChar> filepath(tfilepath);
    return remove_file(std_fs_path(filepath));
}

std::string get_current_dir();

// between 0 and max, 0 and max are included
size_t rand_number(size_t max);

std::string get_utc_time();

std::wstring to_wstr(std::string_view str);
std::string to_str(std::wstring_view wstr);

template<StringViewable TStringSrc, StringViewable TStringSub, StringViewable TStringRep, typename TChar = str_info_t<TStringSrc>>
    requires std::is_same_v<TChar, str_info_t<TStringSub>> && std::is_same_v<TChar, str_info_t<TStringRep>>
std::basic_string<TChar> str_replace_all(const TStringSrc &tsource, const TStringSub &tsubstr, const TStringRep &treplace)
{
    std::basic_string_view<TChar> source(tsource);
    std::basic_string_view<TChar> substr(tsubstr);
    std::basic_string_view<TChar> replace(treplace);
    if (source.empty() || substr.empty()) return std::basic_string<TChar>(source);

    std::basic_string<TChar> out{};
    out.reserve(source.size() / 4); // out could be bigger or smaller than source, start small

    size_t start_offset = 0;
    auto f_idx = source.find(substr);
    while (std::basic_string<TChar>::npos != f_idx) {
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
    out.append(source, start_offset, std::basic_string<TChar>::npos);

    return out;
}

std::fstream open_fstream(const std::filesystem::path &filepath, std::ios_base::openmode mode);
template<StringViewable TString, typename TChar = str_info_t<TString>>
std::fstream open_fstream(const TString &tfilepath, std::ios_base::openmode mode)
{
    return open_fstream( std_fs_path(tfilepath), mode );
}

std::ifstream open_fread(const std::filesystem::path &filepath, std::ios_base::openmode mode = std::ios_base::in);
template<StringViewable TString, typename TChar = str_info_t<TString>>
std::ifstream open_fread(const TString &tfilepath, std::ios_base::openmode mode = std::ios_base::in)
{
    return open_fread( std_fs_path(tfilepath), mode );
}

std::ofstream open_fwrite(const std::filesystem::path &filepath, std::ios_base::openmode mode = std::ios_base::out);
template<StringViewable TString, typename TChar = str_info_t<TString>>
std::ofstream open_fwrite(const TString &tfilepath, std::ios_base::openmode mode = std::ios_base::out)
{
    return open_fwrite( std_fs_path(tfilepath), mode );
}

}
