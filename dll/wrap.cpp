/* Copyright (C) 2019 Mr Goldberg
   This file is part of the Goldberg Emulator

   The Goldberg Emulator is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   The Goldberg Emulator is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the Goldberg Emulator; if not, see
   <http://www.gnu.org/licenses/>.  */

#if defined(WIN64) || defined(_WIN64) || defined(__MINGW64__)
    #define __WINDOWS_64__
#elif defined(WIN32) || defined(_WIN32) || defined(__MINGW32__)
	#define __WINDOWS_32__
#endif

#if defined(__WINDOWS_32__) || defined(__WINDOWS_64__)
// Nothing to be done here
#else
#define STEAM_API_FUNCTIONS_IMPL

#include "base.h"
#include "dll.h"

#define PATH_SEPARATOR_CHAR '/'
#define STEAM_PATH_CACHE_SIZE 4096

const char *STEAM_PATH;
size_t STEAM_PATH_SIZE;

#ifndef __x86_64__
# define _STAT_VER_LINUX_OLD    1
# define _STAT_VER_KERNEL       1
# define _STAT_VER_SVR4         2
# define _STAT_VER_LINUX        3
# define _MKNOD_VER_LINUX       1
# define _MKNOD_VER_SVR4        2
#else
# define _STAT_VER_KERNEL       0
# define _STAT_VER_LINUX        1
# define _MKNOD_VER_LINUX       0
#endif
#define _STAT_VER               _STAT_VER_LINUX
#define _MKNOD_VER              _MKNOD_VER_LINUX

/* From kernel_stat.h It help me save some condition  */
#define XSTAT_IS_XSTAT64     1
#define STATFS_IS_STATFS64   __STATFS_MATCHES_STATFS64
#define STAT_IS_KERNEL_STAT  1

// Returns a '/' terminated absolute path to the steam folder in user's home,
// root is returned if env home is not set
const char *get_steam_path()
{
    char *home_path = getenv("HOME");
    char steam_path[STEAM_PATH_CACHE_SIZE];
    char *steam_realpath = nullptr;

    // Build steam_path from home
    int required_size = snprintf(steam_path, STEAM_PATH_CACHE_SIZE, "%s/.steam/steam", home_path);

    // Allocate more space for steam_path if needed (required_size does not count terminator)
    if (required_size > 0 && required_size >= STEAM_PATH_CACHE_SIZE) {
        char *large_steam_path = (char *)malloc(sizeof(char) * (required_size + 1));
        int check_size = snprintf(large_steam_path, required_size + 1, "%s/.steam/steam", home_path);
        // Check that path fits this time
        if (check_size == required_size) {
            steam_realpath = realpath(large_steam_path, nullptr);
        }
        free(large_steam_path);
    } else {
        steam_realpath = realpath(steam_path, nullptr);
    }

    // Terminate path with a file separator
    if (steam_realpath && *steam_realpath) {
        size_t path_size = strlen(steam_realpath);
        if (steam_realpath[path_size - 1] != PATH_SEPARATOR_CHAR) {
            steam_realpath = (char *)realloc(steam_realpath, path_size + 2);
            steam_realpath[path_size] = PATH_SEPARATOR_CHAR;
            steam_realpath[path_size + 1] = 0;
        }
    } else {
        // Failsafe to root
        steam_realpath = strdup("/");
    }

    return steam_realpath;
}

// Fixes given path by navigating filesystem and lowering case to match
// existing entries on disk
bool match_path(char *path, int start, bool accept_same_case)
{
    if (!path[start + 1]) {
        return true;
    }

    // Snap to the next separator in path
    int separator = start + 1;
    while (path[separator] != PATH_SEPARATOR_CHAR && path[separator]) {
        separator++;
    }

    bool is_last_component = path[separator] != PATH_SEPARATOR_CHAR;

    char stored_char = path[separator];
    path[separator] = 0;
    bool path_accessible = access(path, F_OK) == 0;
    path[separator] = stored_char;

    if (!path_accessible || (!is_last_component && !match_path(path, separator, accept_same_case))) {
        DIR *current_directory = nullptr;
        int component = start + 1;

        if (start) {
            stored_char = path[start];
            path[start] = 0;
            current_directory = opendir(path);
            path[start] = stored_char;
            component = start + 1;
        } else {
            if (*path == PATH_SEPARATOR_CHAR) {
                component = start + 1;
                current_directory = opendir("/");
            } else {
                component = start;
                current_directory = opendir(".");
            }
        }

        //        0123456789012345678901234567890123456789
        // path = /this/is/a/sample/path/to/file.txt
        //                  ^^     ^
        //                  ab     c
        // a. start = 10
        // b. component = 11
        // c. separator = 17
        // current_directory = /this/is/a/

        if (current_directory) {
            dirent64 *entry = (dirent64 *)readdir64(current_directory);
            while (entry) {
                const char *entry_name = entry->d_name;
                stored_char = path[separator];
                path[separator] = 0;

                // Fix current component if entry with similar name exists
                if (!strcasecmp(&path[component], entry_name)) {
                    bool case_differs = strcmp(&path[component], entry_name) != 0;
                    path[separator] = stored_char;
                    if (case_differs) {
                        char *iterator = &path[component];
                        // Replace with entry name
                        while (*entry_name != PATH_SEPARATOR_CHAR && *entry_name) {
                            *(iterator++) = *(entry_name++);
                        }
                        // Fix next component
                        if (is_last_component || match_path(path, separator, accept_same_case)) {
                            closedir(current_directory);
                            return true;
                        }
                    }
                } else {
                    path[separator] = stored_char;
                }
                entry = (dirent64 *)readdir64(current_directory);
            }
        }

        if (current_directory) {
            closedir(current_directory);
        }

        return accept_same_case && is_last_component;
    }

    return true;
}

// Tries to convert the given path to the preferred lower-cased version
const char *lowercase_path(const char *path, bool accept_same_case, bool stop_at_separator)
{
    std::locale loc;
    char *path_lowercased = nullptr;

    if (path && *path) {
        // If file does not exist
        if (access(path, F_OK)) {
            // Make a copy of the path on which to work on
            path_lowercased = strdup(path);
            if (!path_lowercased) {
                return nullptr;
            }
            // Load steam path if not done already
            if (!STEAM_PATH) {
                STEAM_PATH = get_steam_path();
                STEAM_PATH_SIZE = strlen(STEAM_PATH);
            }

            char *lowercase_iterator = path_lowercased;
            // Lowercase whole steam path if possible
            bool has_steam_root = false;
            if (!strncasecmp(path_lowercased, STEAM_PATH, STEAM_PATH_SIZE)) {
                memcpy(path_lowercased, STEAM_PATH, STEAM_PATH_SIZE);
                lowercase_iterator = &path_lowercased[STEAM_PATH_SIZE - 1];
                has_steam_root = true;
            }
            // Lowercase rest of the path
            char *iterator = lowercase_iterator;
            while ((!stop_at_separator || *iterator != PATH_SEPARATOR_CHAR) && *iterator) {
                *iterator = std::tolower(*iterator, loc);
                iterator++;
            }

            // Check if we can access the lowered-case path
            int error = access(path_lowercased, F_OK);
            if (!error) {
                // The new path is valid
                return path_lowercased;
            } else {
                if (accept_same_case) {
                    const char *name_iterator = &path[lowercase_iterator - path_lowercased];
                    while (*lowercase_iterator) {
                        *(lowercase_iterator++) = *(name_iterator++);
                    }
                }
                // Retry accesing the file again and tweak the path if needed
                if (match_path(path_lowercased, has_steam_root? STEAM_PATH_SIZE - 1 : 0, accept_same_case)) {
                    return path_lowercased;
                }
            }
        }
    }

    return path;
}

STEAMAPI_API FILE *__wrap_freopen(const char *path, const char *modes, FILE *stream)
{
    bool is_writable = strpbrk(modes, "wa+") != 0;
    const char *path_lowercased = lowercase_path(path, is_writable, true);
    FILE *result = freopen(path_lowercased, modes, stream);
    if (path_lowercased != path) {
        free((void *)path_lowercased);
    }
    return result;
}

STEAMAPI_API FILE *__wrap_fopen(const char *path, const char *modes)
{
    bool is_writable = strpbrk(modes, "wa+") != 0;
    const char *path_lowercased = lowercase_path(path, is_writable, true);
    FILE *result = fopen(path_lowercased, modes);
    if (path_lowercased != path) {
        free((void *)path_lowercased);
    }
    return result;
}

STEAMAPI_API FILE *__wrap_fopen64(const char *path, const char *modes)
{
    bool is_writable = strpbrk(modes, "wa+") != 0;
    const char *path_lowercased = lowercase_path(path, is_writable, true);
    FILE *result = fopen64(path_lowercased, modes);
    if (path_lowercased != path) {
        free((void *)path_lowercased);
    }
    return result;
}

STEAMAPI_API int __wrap_open(const char *path, int flags, mode_t mode)
{
    bool is_writable = flags & (X_OK | W_OK);
    const char *path_lowercased = lowercase_path(path, is_writable, true);
    int result = open(path_lowercased, flags, mode);
    if (path_lowercased != path) {
        free((void *)path_lowercased);
    }
    return result;
}

STEAMAPI_API int __wrap_open64(const char *path, int flags, mode_t mode)
{
    bool is_writable = flags & (X_OK | W_OK);
    const char *path_lowercased = lowercase_path(path, is_writable, true);
    int result = open64(path_lowercased, flags, mode);
    if (path_lowercased != path) {
        free((void *)path_lowercased);
    }
    return result;
}

STEAMAPI_API int __wrap_access(const char *path, int mode)
{
    const char *path_lowercased = lowercase_path(path, false, false);
    int result = access(path_lowercased, mode);
    if (path_lowercased != path) {
        free((void *)path_lowercased);
    }
    return result;
}

STEAMAPI_API int __wrap___xstat(int ver, const char * path, struct stat * stat_buf)
{
    const char *path_lowercased = lowercase_path(path, false, false);
    int result;

    switch (ver) {
    case _STAT_VER_KERNEL:
        result = stat(path_lowercased, stat_buf);
        break;
    default:
        result = EINVAL;
    }

    if (path_lowercased != path) {
        free((void *)path_lowercased);
    }
    return result;
}

STEAMAPI_API int __wrap_stat(const char * path, struct stat * stat_buf)
{
    return __wrap___xstat(3, path, stat_buf);
}

STEAMAPI_API int __wrap___lxstat(int ver, const char * path, struct stat * stat_buf)
{
    const char *path_lowercased = lowercase_path(path, false, false);
    int result;

    switch (ver) {
    case _STAT_VER_KERNEL:
        result = lstat(path_lowercased, stat_buf);
        break;
    default:
        result = EINVAL;
    }

    if (path_lowercased != path) {
        free((void *)path_lowercased);
    }
    return result;
}

STEAMAPI_API int __wrap_lstat(const char * path, struct stat * stat_buf)
{
    return __wrap___lxstat(3, path, stat_buf);
}

STEAMAPI_API int __wrap_scandir(const char *path, struct dirent ***namelist, int (*sel)(const struct dirent *), int (*compar)(const struct dirent **, const struct dirent **))
{
    const char *path_lowercased = lowercase_path(path, false, false);
    int result = scandir(path_lowercased, namelist, sel, compar);
    if (path_lowercased != path) {
        free((void *)path_lowercased);
    }
    return result;
}

STEAMAPI_API int __wrap_scandir64(const char *path, struct dirent64 ***namelist, int (*sel)(const struct dirent64 *), int (*compar)(const struct dirent64 **, const struct dirent64 **))
{
    const char *path_lowercased = lowercase_path(path, false, false);
    int result = scandir64(path_lowercased, namelist, sel, compar);
    if (path_lowercased != path) {
        free((void *)path_lowercased);
    }
    return result;
}

STEAMAPI_API DIR *__wrap_opendir(const char *path)
{
    const char *path_lowercased = lowercase_path(path, false, false);
    DIR *result = opendir(path_lowercased);
    if (path_lowercased != path) {
        free((void *)path_lowercased);
    }
    return result;
}

STEAMAPI_API int __wrap___xstat64(int ver, const char *path, struct stat64 *stat_buf)
{
    const char *path_lowercased = lowercase_path(path, false, false);
    int result;

    switch (ver) {
    case _STAT_VER_KERNEL:
        result = stat64(path_lowercased, stat_buf);
        break;
    default:
        result = EINVAL;
    }

    if (path_lowercased != path) {
        free((void *)path_lowercased);
    }
    return result;
}

STEAMAPI_API int __wrap___lxstat64(int ver, const char *path, struct stat64 *stat_buf)
{
    const char *path_lowercased = lowercase_path(path, false, false);
    int result;

    switch (ver) {
    case _STAT_VER_KERNEL:
        result = lstat64(path_lowercased, stat_buf);
        break;
    default:
        result = EINVAL;
    }

    if (path_lowercased != path) {
        free((void *)path_lowercased);
    }
    return result;
}

STEAMAPI_API int __wrap_statvfs(const char *path, struct statvfs *buf)
{
    const char *path_lowercased = lowercase_path(path, false, false);
    int result = statvfs(path_lowercased, buf);
    if (path_lowercased != path) {
        free((void *)path_lowercased);
    }
    return result;
}

STEAMAPI_API int __wrap_statvfs64(const char *path, struct statvfs64 *buf)
{
    const char *path_lowercased = lowercase_path(path, false, false);
    int result = statvfs64(path_lowercased, buf);
    if (path_lowercased != path) {
        free((void *)path_lowercased);
    }
    return result;
}

STEAMAPI_API int __wrap_chmod(const char *path, mode_t mode)
{
    const char *path_lowercased = lowercase_path(path, false, false);
    int result = chmod(path_lowercased, mode);
    if (path_lowercased != path) {
        free((void *)path_lowercased);
    }
    return result;
}

STEAMAPI_API int __wrap_chown(const char *path, uid_t owner, gid_t group)
{
    const char *path_lowercased = lowercase_path(path, false, false);
    int result = chown(path_lowercased, owner, group);
    if (path_lowercased != path) {
        free((void *)path_lowercased);
    }
    return result;
}

STEAMAPI_API int __wrap_lchown(const char *path, uid_t owner, gid_t group)
{
    const char *path_lowercased = lowercase_path(path, false, false);
    int result = lchown(path_lowercased, owner, group);
    if (path_lowercased != path) {
        free((void *)path_lowercased);
    }
    return result;
}

STEAMAPI_API int __wrap_symlink(const char *path1, const char *path2)
{
    const char *path_lowercased1 = lowercase_path(path1, true, true);
    const char *path_lowercased2 = lowercase_path(path2, false, false);
    int result = symlink(path_lowercased1, path_lowercased2);
    if (path_lowercased1 != path1) {
        free((void *)path_lowercased1);
    }
    if (path_lowercased2 != path2) {
        free((void *)path_lowercased2);
    }
    return result;
}

STEAMAPI_API int __wrap_link(const char *path1, const char *path2)
{
    const char *path_lowercased1 = lowercase_path(path1, true, true);
    const char *path_lowercased2 = lowercase_path(path2, false, false);
    int result = link(path_lowercased1, path_lowercased2);
    if (path_lowercased1 != path1) {
        free((void *)path_lowercased1);
    }
    if (path_lowercased2 != path2) {
        free((void *)path_lowercased2);
    }
    return result;
}

STEAMAPI_API int __wrap_mknod(const char *path, mode_t mode, dev_t dev)
{
    const char *path_lowercased = lowercase_path(path, true, true);
    int result = mknod(path_lowercased, mode, dev);
    if (path_lowercased != path) {
        free((void *)path_lowercased);
    }
    return result;
}

STEAMAPI_API int __wrap_mount(const char *source, const char *target, const char *filesystemtype, unsigned long mountflags, const void *data)
{
    const char *source_lowercased = lowercase_path(source, false, false);
    const char *target_lowercased = lowercase_path(target, false, false);
    int result = mount(source_lowercased, target_lowercased, filesystemtype, mountflags, data);
    if (source_lowercased != source) {
        free((void *)source_lowercased);
    }
    if (target_lowercased != target) {
        free((void *)target_lowercased);
    }
    return result;
}

STEAMAPI_API int __wrap_unlink(const char *path)
{
    const char *path_lowercased = lowercase_path(path, false, false);
    int result = unlink(path);
    if (path_lowercased != path) {
        free((void *)path_lowercased);
    }
    return result;
}

STEAMAPI_API int __wrap_mkfifo(const char *path, mode_t mode)
{
    const char *path_lowercased = lowercase_path(path, true, true);
    int result = mkfifo(path, mode);
    if (path_lowercased != path) {
        free((void *)path_lowercased);
    }
    return result;
}

STEAMAPI_API int __wrap_rename(const char *old_name, const char *new_name)
{
    const char *old_name_lowercased = lowercase_path(old_name, true, true);
    const char *new_name_lowercased = lowercase_path(new_name, false, false);
    int result = rename(old_name_lowercased, new_name_lowercased);
    if (old_name_lowercased != old_name) {
        free((void *)old_name_lowercased);
    }
    if (new_name_lowercased != new_name) {
        free((void *)new_name_lowercased);
    }
    return result;
}

STEAMAPI_API int __wrap_utime(const char *path, const struct utimbuf *times)
{
    const char *path_lowercased = lowercase_path(path, false, false);
    int result = utime(path_lowercased, times);
    if (path_lowercased != path) {
        free((void *)path_lowercased);
    }
    return result;
}

STEAMAPI_API int __wrap_utimes(const char *path, const struct timeval times[2])
{
    const char *path_lowercased = lowercase_path(path, false, false);
    int result = utimes(path_lowercased, times);
    if (path_lowercased != path) {
        free((void *)path_lowercased);
    }
    return result;
}

STEAMAPI_API int __wrap_mkdir(const char *path, mode_t mode)
{
    const char *path_lowercased = lowercase_path(path, true, true);
    int result = mkdir(path_lowercased, mode);
    if (path_lowercased != path) {
        free((void *)path_lowercased);
    }
    return result;
}

STEAMAPI_API int __wrap_rmdir(const char *path)
{
    const char *path_lowercased = lowercase_path(path, false, false);
    int result = rmdir(path_lowercased);
    if (path_lowercased != path) {
        free((void *)path_lowercased);
    }
    return result;
}

STEAMAPI_API void *__wrap_dlopen(const char *path, int mode)
{
    const char *path_lowercased = lowercase_path(path, false, false);
    void * result = dlopen(path_lowercased, mode);
    if (path_lowercased != path) {
        free((void *)path_lowercased);
    }
    return result;
}

STEAMAPI_API void *__wrap_dlmopen(Lmid_t lmid, const char *path, int flags)
{
    const char *path_lowercased = lowercase_path(path, false, false);
    void * result = dlmopen(lmid, path_lowercased, flags);
    if (path_lowercased != path) {
        free((void *)path_lowercased);
    }
    return result;
}

#endif
