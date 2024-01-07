#ifndef _CRASH_PRINTER_WIN
#define _CRASH_PRINTER_WIN


#include <string>

namespace crash_printer {

bool init(const std::wstring &log_file);

void deinit();

}

#endif // _CRASH_PRINTER_WIN