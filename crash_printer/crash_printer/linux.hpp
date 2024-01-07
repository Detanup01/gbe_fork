#ifndef _CRASH_PRINTER_LINUX
#define _CRASH_PRINTER_LINUX


#include <string>

namespace crash_printer {

bool init(const std::string &log_file);

void deinit();

}

#endif // _CRASH_PRINTER_LINUX
