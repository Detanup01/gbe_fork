#ifndef _CRASH_PRINTER_COMMON_H
#define _CRASH_PRINTER_COMMON_H


#include <string>
#include <fstream>

namespace crash_printer {

bool create_dir(const std::string &dir);

bool create_dir(const std::wstring &dir);

void write(std::ofstream &file, const std::string &data);

}

#endif // _CRASH_PRINTER_COMMON_H
