#include "dbg_log/dbg_log.hpp"
#include "common_helpers/common_helpers.hpp"
#include "utfcpp/utf8.h"

#include <iterator>
#include <cwchar>
#include <cstdarg>
#include <filesystem>
#include <sstream>
#include <string>
#include <stdio.h>

#include "common_helpers/os_detector.h"


void dbg_log::open()
{
#ifndef EMU_RELEASE_BUILD
	if (!out_file && filepath.size()) {

		// https://en.cppreference.com/w/cpp/filesystem/path/u8path
		const auto fsp = std::filesystem::u8path(filepath);
#if defined(__WINDOWS__)
		out_file = _wfopen(fsp.c_str(), L"a");
#else
		out_file = std::fopen(fsp.c_str(), "a");
#endif

	}

#endif

}

void dbg_log::write_stamp()
{
	auto elapsed = std::chrono::high_resolution_clock::now() - start_time;
	auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
	auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();

	std::stringstream ss{};
	ss << "[" << duration_ms << " ms, " << duration_us << " us] ";
	auto ss_str = ss.str();

	std::fprintf(out_file, "%s", ss_str.c_str());
}

dbg_log::dbg_log(std::string_view path)
{
	
#ifndef EMU_RELEASE_BUILD
	filepath = path;
#endif

}

dbg_log::dbg_log(std::wstring_view path)
{
	
#ifndef EMU_RELEASE_BUILD
	filepath = common_helpers::to_str(path);
#endif

}

dbg_log::~dbg_log()
{
	
#ifndef EMU_RELEASE_BUILD
	close();
#endif

}

void dbg_log::write(std::string_view str)
{

#ifndef EMU_RELEASE_BUILD
	write(str.data());
#endif

}

void dbg_log::write(std::wstring_view str)
{

#ifndef EMU_RELEASE_BUILD
	write(str.data());
#endif

}

void dbg_log::write(const char *fmt, ...)
{

#ifndef EMU_RELEASE_BUILD
	std::lock_guard lk(f_mtx);
	open();
	if (out_file) {
		write_stamp();

		std::va_list args;
		va_start(args, fmt);
		std::vfprintf(out_file, fmt, args);
		va_end(args);

		std::fprintf(out_file, "\n");
		std::fflush(out_file);
	}
#endif

}

void dbg_log::write(const wchar_t *fmt, ...)
{

#ifndef EMU_RELEASE_BUILD
	std::lock_guard lk(f_mtx);
	open();
	if (out_file) {
		write_stamp();

		std::va_list args;
		va_start(args, fmt);
		std::vfwprintf(out_file, fmt, args);
		va_end(args);

		std::fprintf(out_file, "\n");
		std::fflush(out_file);
	}
#endif

}

void dbg_log::close()
{

#ifndef EMU_RELEASE_BUILD
	std::lock_guard lk(f_mtx);
	if (out_file) {
		std::fprintf(out_file, "\nLog file closed\n\n");
		std::fclose(out_file);
		out_file = nullptr;
	}
#endif

}
