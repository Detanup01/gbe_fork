#pragma once

#include <string>
#include <fstream>

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

}
