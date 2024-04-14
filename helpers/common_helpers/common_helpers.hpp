#pragma once

#include <cstdlib>
#include <string>
#include <string_view>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <vector>

namespace common_helpers {

bool create_dir(const std::string_view &dir);
bool create_dir(const std::wstring_view &dir);

void write(std::ofstream &file, const std::string_view &data);

std::wstring str_to_w(const std::string_view &str);
std::string wstr_to_a(const std::wstring_view &wstr);

bool starts_with_i(const std::string_view &target, const std::string_view &query);
bool starts_with_i(const std::wstring_view &target, const std::wstring_view &query);

bool ends_with_i(const std::string_view &target, const std::string_view &query);
bool ends_with_i(const std::wstring_view &target, const std::wstring_view &query);

std::string string_strip(const std::string_view &str);

std::string uint8_vector_to_hex_string(const std::vector<uint8_t> &v);

bool str_cmp_insensitive(const std::string_view &str1, const std::string_view &str2);
bool str_cmp_insensitive(const std::wstring_view &str1, const std::wstring_view &str2);

std::string ascii_to_lowercase(std::string data);

void thisThreadYieldFor(std::chrono::microseconds u);

void consume_bom(std::ifstream &input);

std::string to_lower(const std::string_view &str);
std::wstring to_lower(const std::wstring_view &wstr);

std::string to_upper(const std::string_view &str);
std::wstring to_upper(const std::wstring_view &wstr);

std::string to_absolute(const std::string_view &path, const std::string_view &base = std::string_view());
std::wstring to_absolute(const std::wstring_view &path, const std::wstring_view &base = std::wstring_view());

bool file_exist(const std::filesystem::path &filepath);
bool file_exist(const std::string &filepath);
bool file_exist(const std::wstring &filepath);

bool file_size(const std::filesystem::path &filepath, size_t &size);
bool file_size(const std::string &filepath, size_t &size);
bool file_size(const std::wstring &filepath, size_t &size);

bool dir_exist(const std::filesystem::path &dirpath);
bool dir_exist(const std::string_view &dirpath);
bool dir_exist(const std::wstring_view &dirpath);

}
