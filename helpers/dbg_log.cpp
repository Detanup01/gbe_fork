#include "dbg_log/dbg_log.hpp"
#include "common_helpers/common_helpers.hpp"

#include <cstdarg>
#include <cstdio>
#include <filesystem>
#include <thread>
#include <sstream>
#include <chrono>
#include <mutex>

static FILE* out_file = nullptr;
auto const static start_time = std::chrono::system_clock::now();
static std::recursive_mutex f_mtx{};

bool dbg_log::init(const wchar_t *path)
{
	auto p = std::filesystem::path(path).u8string();
	return init(p.c_str());
}

bool dbg_log::init(const char *path)
{
#ifndef EMU_RELEASE_BUILD
	std::lock_guard lk(f_mtx);
	if (!out_file) {
		out_file = std::fopen(path, "a");
		if (!out_file) {
			return false;
		}
	}

#endif

	return true;
}

void dbg_log::write(const std::wstring &str)
{

#ifndef EMU_RELEASE_BUILD
	write(common_helpers::wstr_to_a(str));
#endif

}

void dbg_log::write(const std::string &str)
{

#ifndef EMU_RELEASE_BUILD
	write(str.c_str());
#endif

}

void dbg_log::write(const char *fmt, ...)
{

#ifndef EMU_RELEASE_BUILD
	std::lock_guard lk(f_mtx);
	if (out_file) {
		auto elapsed = std::chrono::system_clock::now() - start_time;

		std::stringstream ss{};
		ss << "[" << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << " ms] [tid: "  << std::this_thread::get_id() << "] ";
		auto ss_str = ss.str();

		std::fprintf(out_file, ss_str.c_str());

		std::va_list args;
		va_start(args, fmt);
		std::vfprintf(out_file, fmt, args);
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
