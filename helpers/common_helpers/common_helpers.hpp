#pragma once

#include <string>
#include <fstream>
#include <filesystem>

namespace common_helpers {

bool create_dir(const std::string &dir);

bool create_dir(const std::wstring &dir);

void write(std::ofstream &file, const std::string &data);

std::wstring str_to_w(const std::string &str);

std::string wstr_to_a(const std::wstring &wstr);

bool starts_with_i(const std::string &target, const std::string &query);

bool starts_with_i(const std::wstring &target, const std::wstring &query);

bool ends_with_i(const std::string &target, const std::string &query);

bool ends_with_i(const std::wstring &target, const std::wstring &query);

std::string to_lower(std::string str);

std::wstring to_lower(std::wstring wstr);

std::string to_upper(std::string str);

std::wstring to_upper(std::wstring wstr);

std::string to_absolute(const std::string &path, const std::string &base = std::string());

std::wstring to_absolute(const std::wstring &path, const std::wstring &base = std::wstring());

bool file_exist(const std::filesystem::path &filepath);

bool file_exist(const std::string &filepath);

bool file_exist(const std::wstring &filepath);

bool file_size(const std::filesystem::path &filepath, size_t &size);

bool file_size(const std::string &filepath, size_t &size);

bool file_size(const std::wstring &filepath, size_t &size);

bool dir_exist(const std::filesystem::path &dirpath);

bool dir_exist(const std::string &dirpath);

bool dir_exist(const std::wstring &dirpath);

}
