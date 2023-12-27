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

#include "dll/local_storage.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#if defined(__WINDOWS__)
#define STBI_WINDOWS_UTF8
#endif
#include "stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_STATIC
#include "stb/stb_image_write.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb/stb_image_resize.h"

struct File_Data {
    std::string name;
};

#ifdef NO_DISK_WRITES
std::string Local_Storage::get_program_path()
{
    return " ";
}


std::string Local_Storage::get_user_appdata_path()
{
    return " ";
}

std::string Local_Storage::get_game_settings_path()
{
    return " ";
}

std::string Local_Storage::get_path(std::string folder)
{
    return "";
}

std::string Local_Storage::get_global_settings_path()
{
    return "";
}

Local_Storage::Local_Storage(std::string save_directory)
{

}

void Local_Storage::setAppId(uint32 appid)
{

}

int Local_Storage::store_file_data(std::string folder, std::string file, char *data, unsigned int length)
{
    return -1;
}

int Local_Storage::store_data(std::string folder, std::string file, char *data, unsigned int length)
{
    return -1;
}

int Local_Storage::store_data_settings(std::string file, char *data, unsigned int length)
{
    return -1;
}

int Local_Storage::get_file_data(std::string full_path, char *data, unsigned int max_length, unsigned int offset)
{
    return -1;
}

int Local_Storage::get_data(std::string folder, std::string file, char *data, unsigned int max_length, unsigned int offset)
{
    return -1;
}

int Local_Storage::get_data_settings(std::string file, char *data, unsigned int max_length)
{
    return 0;
}

int Local_Storage::count_files(std::string folder)
{
    return 0;
}

bool Local_Storage::file_exists(std::string folder, std::string file)
{
    return false;
}

unsigned int Local_Storage::file_size(std::string folder, std::string file)
{
    return 0;
}

bool Local_Storage::file_delete(std::string folder, std::string file)
{
    return false;
}

uint64_t Local_Storage::file_timestamp(std::string folder, std::string file)
{
    return 0;
}

bool Local_Storage::iterate_file(std::string folder, int index, char *output_filename, int32 *output_size)
{
    return false;
}

bool Local_Storage::update_save_filenames(std::string folder)
{
    return true;
}

bool Local_Storage::load_json(std::string full_path, nlohmann::json& json)
{
    return false;
}

bool Local_Storage::load_json_file(std::string folder, std::string const&file, nlohmann::json& json)
{
    return false;
}

bool Local_Storage::write_json_file(std::string folder, std::string const&file, nlohmann::json const& json)
{
    return false;
}

std::vector<std::string> Local_Storage::get_filenames_path(std::string path)
{
    return std::vector<std::string>();
}

std::vector<image_pixel_t> Local_Storage::load_image(std::string const& image_path)
{
    return std::vector<image_pixel_t>();
}

std::string Local_Storage::load_image_resized(std::string const& image_path, std::string const& image_data, int resolution)
{
    return "";
}

bool Local_Storage::save_screenshot(std::string const& image_path, uint8_t* img_ptr, int32_t width, int32_t height, int32_t channels)
{
    return false;
}

std::string Local_Storage::sanitize_string(std::string name)
{
    return "";
}

std::string Local_Storage::desanitize_string(std::string name)
{
    return "";
}

#else
#if defined(__WINDOWS__)

static BOOL DirectoryExists(LPCWSTR szPath)
{
  DWORD dwAttrib = GetFileAttributesW(szPath);

  return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
    (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

static void createDirectoryRecursively(std::wstring path)
{
  unsigned long long pos = 0;
  do
  {
    pos = path.find_first_of(L"\\/", pos + 1);
    CreateDirectoryW(path.substr(0, pos).c_str(), NULL);
  } while (pos != std::string::npos);
}

static void create_directory(std::string in_path)
{
    std::wstring strPath = utf8_decode(in_path);
    if (DirectoryExists(strPath.c_str()) == FALSE)
        createDirectoryRecursively(strPath);
}

static std::vector<struct File_Data> get_filenames(std::string in_path)
{
    std::vector<struct File_Data> output;
    in_path = in_path.append("\\*");
    WIN32_FIND_DATAW ffd;
    HANDLE hFind = INVALID_HANDLE_VALUE;

    std::wstring strPath = utf8_decode(in_path);
    // Start iterating over the files in the path directory.
    hFind = ::FindFirstFileW (strPath.c_str(), &ffd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do // Managed to locate and create an handle to that folder.
        {
            if (wcscmp(L".", ffd.cFileName) == 0) continue;
            if (wcscmp(L"..", ffd.cFileName) == 0) continue;
            struct File_Data f_data;
            f_data.name = utf8_encode(ffd.cFileName);
            output.push_back(f_data);
        } while (::FindNextFileW(hFind, &ffd) == TRUE);
        ::FindClose(hFind);
    } else {
        //printf("Failed to find path: %s", strPath.c_str());
    }

    return output;
}

static std::vector<struct File_Data> get_filenames_recursive_w(std::wstring base_path)
{
    if (base_path.back() == *L"\\")
        base_path.pop_back();
    std::vector<struct File_Data> output;
    std::wstring strPath = base_path;
    strPath = strPath.append(L"\\*");
    WIN32_FIND_DATAW ffd;
    HANDLE hFind = INVALID_HANDLE_VALUE;

    // Start iterating over the files in the path directory.
    hFind = ::FindFirstFileW (strPath.c_str(), &ffd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do // Managed to locate and create an handle to that folder.
        {
            if (wcscmp(L".", ffd.cFileName) == 0) continue;
            if (wcscmp(L"..", ffd.cFileName) == 0) continue;
            if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                // Construct new path from our base path
                std::wstring dir_name = ffd.cFileName;

                std::wstring path = base_path;
                path += L"\\";
                path += dir_name;

                std::vector<struct File_Data> lower = get_filenames_recursive_w(path);
                std::transform(lower.begin(), lower.end(), std::back_inserter(output), [&dir_name](File_Data f) {f.name = utf8_encode(dir_name) + "\\" + f.name; return f;});
            } else {
                File_Data f;
                f.name = utf8_encode(ffd.cFileName);
                output.push_back(f);
            }
        } while (::FindNextFileW(hFind, &ffd) == TRUE);
        ::FindClose(hFind);
    } else {
        //printf("Failed to find path: %s", strPath.c_str());
    }

    reset_LastError();
    return output;
}

static std::vector<struct File_Data> get_filenames_recursive(std::string base_path)
{
    return get_filenames_recursive_w(utf8_decode(base_path));
}

#else

/* recursive mkdir */
static int mkdir_p(const char *dir, const mode_t mode) {
    char tmp[PATH_MAX_STRING_SIZE];
    char *p = NULL;
    struct stat sb;
    size_t len;
    
    /* copy path */
    len = strnlen (dir, PATH_MAX_STRING_SIZE);
    if (len == 0 || len == PATH_MAX_STRING_SIZE) {
        return -1;
    }
    memcpy (tmp, dir, len);
    tmp[len] = '\0';

    /* remove trailing slash */
    if(tmp[len - 1] == '/') {
        tmp[len - 1] = '\0';
    }

    /* check if path exists and is a directory */
    if (stat (tmp, &sb) == 0) {
        if (S_ISDIR (sb.st_mode)) {
            return 0;
        }
    }
    
    /* recursive mkdir */
    for(p = tmp + 1; *p; p++) {
        if(*p == '/') {
            *p = 0;
            /* test path */
            if (stat(tmp, &sb) != 0) {
                /* path does not exist - create directory */
                if (mkdir(tmp, mode) < 0) {
                    return -1;
                }
            } else if (!S_ISDIR(sb.st_mode)) {
                /* not a directory */
                return -1;
            }
            *p = '/';
        }
    }
    /* test path */
    if (stat(tmp, &sb) != 0) {
        /* path does not exist - create directory */
        if (mkdir(tmp, mode) < 0) {
            return -1;
        }
    } else if (!S_ISDIR(sb.st_mode)) {
        /* not a directory */
        return -1;
    }
    return 0;
}

static void create_directory(std::string strPath)
{
    mkdir_p(strPath.c_str(), 0777);
}

static std::vector<struct File_Data> get_filenames(std::string strPath)
{
  DIR *dp;
  int i = 0;
  struct dirent *ep;
  std::vector<struct File_Data> output;
  dp = opendir (strPath.c_str());

  if (dp != NULL)
  {
    while ((ep = readdir (dp))) {
      if (memcmp(ep->d_name, ".", 2) != 0 && memcmp(ep->d_name, "..", 3) != 0) {
        struct File_Data f_data;
        f_data.name = ep->d_name;
        output.push_back(f_data);
        i++;
      }
    }

    (void) closedir (dp);
  }

  return output;
}

static std::vector<struct File_Data> get_filenames_recursive(std::string base_path)
{
    std::vector<struct File_Data> output;
    std::string path;
    struct dirent *dp;
    DIR *dir = opendir(base_path.c_str());

    // Unable to open directory stream
    if (!dir)
        return output;

    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            if (dp->d_type == DT_REG) {
                File_Data f;
                f.name = dp->d_name;
                output.push_back(f);
            } else if (dp->d_type == DT_DIR) {
                // Construct new path from our base path
                std::string dir_name = dp->d_name;

                path = base_path;
                path += "/";
                path += dir_name;

                std::vector<struct File_Data> lower = get_filenames_recursive(path);
                std::transform(lower.begin(), lower.end(), std::back_inserter(output), [&dir_name](File_Data f) {f.name = dir_name + "/" + f.name; return f;});
            }
        }
    }

    closedir(dir);

    return output;
}


#endif 

std::string Local_Storage::get_program_path()
{
    return get_full_program_path();
}

std::string Local_Storage::get_game_settings_path()
{
    return get_program_path().append(game_settings_folder).append(PATH_SEPARATOR);
}

std::string Local_Storage::get_user_appdata_path()
{
    std::string user_appdata_path = "SAVE";
#if defined(STEAM_WIN32)
    WCHAR szPath[MAX_PATH] = {};

    HRESULT hr = SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, szPath);

    if (SUCCEEDED(hr)) {
        user_appdata_path = utf8_encode(szPath);
    }

#else
    /* $XDG_DATA_HOME defines the base directory relative to which user specific data files should be stored. 
    If $XDG_DATA_HOME is either not set or empty, a default equal to $HOME/.local/share should be used. */
    char *datadir = getenv("XDG_DATA_HOME");
    if (datadir) {
        user_appdata_path = datadir;
    } else {
        char *homedir = getenv("HOME");
        if (homedir) {
            user_appdata_path = std::string(homedir) + "/.local/share";
        }
    }
#endif
    return user_appdata_path.append(PATH_SEPARATOR).append(PROGRAM_NAME_1).append(PROGRAM_NAME_2).append(PROGRAM_NAME_3).append(PROGRAM_NAME_4).append(PROGRAM_NAME_5).append(PROGRAM_NAME_6).append(PROGRAM_NAME_7).append(PROGRAM_NAME_8).append(" Saves");
}

static std::string replace_with(std::string s, std::string const &old, const char *new_str)
{
    int pos;
    while ((pos = s.find(old)) != std::string::npos)
        s.replace(pos, old.length(), new_str);

    return s;
}

static std::string sanitize_file_name(std::string name)
{
    if (name.empty()) return name;
    
    //I'm not sure all of these are necessary but just to be sure
    if (name[0] == '.' && name.size() > 2 && (name[1] == '\\' || name[1] == '/')) name.erase(0, 2);

#if defined(STEAM_WIN32)
    name = replace_with(name, "/", PATH_SEPARATOR);
#else
    //On linux does using "\\" in a remote storage file name create a directory?
    //I didn't test but I'm going to say yes
    name = replace_with(name, "\\", PATH_SEPARATOR);
#endif
    name = replace_with(name, "|", ".V_SLASH.");
    name = replace_with(name, ":", ".COLON.");
    name = replace_with(name, "*", ".ASTERISK.");
    name = replace_with(name, "\"", ".QUOTE.");
    name = replace_with(name, "?", ".Q_MARK.");

    return name;
}

static std::string desanitize_file_name(std::string name)
{
    if (name.empty()) return name;
    
    //I'm not sure all of these are necessary but just to be sure
    name = replace_with(name, ".SLASH.", "/");
    name = replace_with(name, ".B_SLASH.", "\\");
    name = replace_with(name, ".F_SLASH.", "/");
    name = replace_with(name, ".V_SLASH.", "|");
    name = replace_with(name, ".COLON.", ":");
    name = replace_with(name, ".ASTERISK.", "*");
    name = replace_with(name, ".QUOTE.", "\"");
    name = replace_with(name, ".Q_MARK.", "?");

    return name;
}

Local_Storage::Local_Storage(std::string save_directory)
{
    this->save_directory = save_directory;

    if (this->save_directory.back() != *PATH_SEPARATOR) {
        this->save_directory.append(PATH_SEPARATOR);
    }
}

void Local_Storage::setAppId(uint32 appid)
{
    this->appid = std::to_string(appid) + PATH_SEPARATOR;
}

int Local_Storage::store_file_data(std::string folder, std::string file, char *data, unsigned int length)
{
    if (folder.back() != *PATH_SEPARATOR) {
        folder.append(PATH_SEPARATOR);
    }

    file = sanitize_file_name(file);
    std::string::size_type pos = file.rfind(PATH_SEPARATOR);

    std::string file_folder;
    if (pos == 0 || pos == std::string::npos) {
        file_folder = "";
    } else {
        file_folder = file.substr(0,pos);
    }

    create_directory(folder + file_folder);
    std::ofstream myfile;
    myfile.open(utf8_decode(folder + file), std::ios::binary | std::ios::out);
    if (!myfile.is_open()) return -1;
    myfile.write(data, length);
    int position = myfile.tellp();
    myfile.close();
    return position;
}

std::string Local_Storage::get_path(std::string folder)
{
    std::string path = save_directory + appid + folder;
    create_directory(path);
    return path;
}

std::string Local_Storage::get_global_settings_path()
{
    return save_directory + settings_storage_folder + PATH_SEPARATOR;
}

std::vector<std::string> Local_Storage::get_filenames_path(std::string path)
{
    if (path.back() != *PATH_SEPARATOR) {
        path.append(PATH_SEPARATOR);
    }

    std::vector<struct File_Data> filenames = get_filenames(path);
    std::vector<std::string> output;
    std::transform(filenames.begin(), filenames.end(), std::back_inserter(output), [](struct File_Data d) { return d.name;});
    return output;
}

int Local_Storage::store_data(std::string folder, std::string file, char *data, unsigned int length)
{
    if (folder.back() != *PATH_SEPARATOR) {
        folder.append(PATH_SEPARATOR);
    }

    return store_file_data(save_directory + appid + folder, file, data, length);
}

int Local_Storage::store_data_settings(std::string file, char *data, unsigned int length)
{
    return store_file_data(get_global_settings_path(), file, data, length);
}

int Local_Storage::get_file_data(std::string full_path, char *data, unsigned int max_length, unsigned int offset)
{
    std::ifstream myfile;
    myfile.open(utf8_decode(full_path), std::ios::binary | std::ios::in);
    if (!myfile.is_open()) return -1;

    myfile.seekg (offset, std::ios::beg);
    myfile.read (data, max_length);
    myfile.close();
    reset_LastError();
    return myfile.gcount();
}

int Local_Storage::get_data(std::string folder, std::string file, char *data, unsigned int max_length, unsigned int offset)
{
    file = sanitize_file_name(file);
    if (folder.back() != *PATH_SEPARATOR) {
        folder.append(PATH_SEPARATOR);
    }

    std::string full_path = save_directory + appid + folder + file;
    return get_file_data(full_path, data, max_length, offset);
}

int Local_Storage::get_data_settings(std::string file, char *data, unsigned int max_length)
{
    file = sanitize_file_name(file);
    std::string full_path = get_global_settings_path() + file;
    return get_file_data(full_path, data, max_length);
}

int Local_Storage::count_files(std::string folder)
{
    if (folder.back() != *PATH_SEPARATOR) {
        folder.append(PATH_SEPARATOR);
    }

    return get_filenames_recursive(save_directory + appid + folder).size();
}

bool Local_Storage::file_exists(std::string folder, std::string file)
{
    file = sanitize_file_name(file);
    if (folder.back() != *PATH_SEPARATOR) {
        folder.append(PATH_SEPARATOR);
    }

    std::string full_path = save_directory + appid + folder + file;
    return file_exists_(full_path);
}

unsigned int Local_Storage::file_size(std::string folder, std::string file)
{
    file = sanitize_file_name(file);
    if (folder.back() != *PATH_SEPARATOR) {
        folder.append(PATH_SEPARATOR);
    }

    std::string full_path = save_directory + appid + folder + file;
    return file_size_(full_path);
}

bool Local_Storage::file_delete(std::string folder, std::string file)
{
    file = sanitize_file_name(file);
    if (folder.back() != *PATH_SEPARATOR) {
        folder.append(PATH_SEPARATOR);
    }

    std::string full_path = save_directory + appid + folder + file;
#if defined(STEAM_WIN32)
    return _wremove(utf8_decode(full_path).c_str()) == 0;
#else
    return remove(full_path.c_str()) == 0;
#endif
}

uint64_t Local_Storage::file_timestamp(std::string folder, std::string file)
{
    file = sanitize_file_name(file);
    if (folder.back() != *PATH_SEPARATOR) {
        folder.append(PATH_SEPARATOR);
    }

    std::string full_path = save_directory + appid + folder + file;

#if defined(STEAM_WIN32)
    struct _stat buffer = {};
    if (_wstat(utf8_decode(full_path).c_str(), &buffer) != 0) return 0;
#else
    struct stat buffer = {};
    if (stat (full_path.c_str(), &buffer) != 0) return 0;
#endif
    return buffer.st_mtime;
}

bool Local_Storage::iterate_file(std::string folder, int index, char *output_filename, int32 *output_size)
{
    if (folder.back() != *PATH_SEPARATOR) {
        folder.append(PATH_SEPARATOR);
    }

    std::vector<struct File_Data> files = get_filenames_recursive(save_directory + appid + folder);
    if (index < 0 || index >= files.size()) return false;

    std::string name = desanitize_file_name(files[index].name);
    if (output_size) *output_size = file_size(folder, name);
#if defined(STEAM_WIN32)
    name = replace_with(name, PATH_SEPARATOR, "/");
#endif
    strcpy(output_filename, name.c_str());
    return true;
}

bool Local_Storage::update_save_filenames(std::string folder)
{
    std::vector<struct File_Data> files = get_filenames_recursive(save_directory + appid + folder);

    for (auto &f : files) {
        std::string path = f.name;
        PRINT_DEBUG("Local_Storage:: remote file %s\n", path.c_str());
        std::string to = sanitize_file_name(desanitize_file_name(path));
        if (path != to && !file_exists(folder, to)) {
            //create the folder
            store_data(folder, to, (char *)"", 0);
            file_delete(folder, to);

            std::string from = (save_directory + appid + folder + PATH_SEPARATOR + path);
            to = (save_directory + appid + folder + PATH_SEPARATOR + to);
            PRINT_DEBUG("Local_Storage::update_save_filenames renaming %s to %s\n", from.c_str(), to.c_str());
            if (std::rename(from.c_str(), to.c_str()) < 0) {
                PRINT_DEBUG("ERROR RENAMING\n");
            }
        }
    }

    return true;
}

bool Local_Storage::load_json(std::string full_path, nlohmann::json& json)
{
    std::ifstream inventory_file(utf8_decode(full_path));
    // If there is a file and we opened it
    if (inventory_file)
    {
        inventory_file.seekg(0, std::ios::end);
        size_t size = inventory_file.tellg();
        std::string buffer(size, '\0');
        inventory_file.seekg(0);
        // Read it entirely, if the .json file gets too big,
        // I should look into this and split reads into smaller parts.
        inventory_file.read(&buffer[0], size);
        inventory_file.close();

        try {
            json = std::move(nlohmann::json::parse(buffer));
            PRINT_DEBUG("Loaded json \"%s\". Loaded %zu items.\n", full_path.c_str(), json.size());
            return true;
        } catch (std::exception& e) {
            PRINT_DEBUG("Error while parsing \"%s\" json: %s\n", full_path.c_str(), e.what());
        }
    }
    else
    {
        PRINT_DEBUG("Couldn't open file \"%s\" to read json\n", full_path.c_str());
    }

    reset_LastError();
    return false;
}

bool Local_Storage::load_json_file(std::string folder, std::string const&file, nlohmann::json& json)
{
    if (!folder.empty() && folder.back() != *PATH_SEPARATOR) {
        folder.append(PATH_SEPARATOR);
    }
    std::string inv_path = std::move(save_directory + appid + folder);
    std::string full_path = inv_path + file;

    return load_json(full_path, json);
}

bool Local_Storage::write_json_file(std::string folder, std::string const&file, nlohmann::json const& json)
{
    if (!folder.empty() && folder.back() != *PATH_SEPARATOR) {
        folder.append(PATH_SEPARATOR);
    }
    std::string inv_path = std::move(save_directory + appid + folder);
    std::string full_path = inv_path + file;

    create_directory(inv_path);

    std::ofstream inventory_file(utf8_decode(full_path), std::ios::trunc | std::ios::out);
    if (inventory_file)
    {
        inventory_file << std::setw(2) << json;
        return true;
    }
    
    PRINT_DEBUG("Couldn't open file \"%s\" to write json\n", full_path.c_str());

    reset_LastError();
    return false;
}

std::vector<image_pixel_t> Local_Storage::load_image(std::string const& image_path)
{
    std::vector<image_pixel_t> res;
    int width, height;
    image_pixel_t* img = (image_pixel_t*)stbi_load(image_path.c_str(), &width, &height, nullptr, 4);
    if (img != nullptr)
    {
        res.resize(width*height);
        std::copy(img, img + width * height, res.begin());

        stbi_image_free(img);
    }

    reset_LastError();
    return res;
}

std::string Local_Storage::load_image_resized(std::string const& image_path, std::string const& image_data, int resolution)
{
    std::string resized_image(resolution * resolution * 4, 0);
    char *resized_img = (char*)malloc(sizeof(char) * resolution * resolution * 4);
    PRINT_DEBUG("Local_Storage::load_image_resized: %s for resized image (%i)\n", (resized_img == nullptr ? "could not allocate memory" : "memory allocated"), (resolution * resolution * 4));

    if (resized_img != nullptr) {
        if (image_path.length() > 0) {
            int width, height;
            unsigned char *img = stbi_load(image_path.c_str(), &width, &height, nullptr, 4);
            PRINT_DEBUG("Local_Storage::load_image_resized: \"%s\" %s\n", image_path.c_str(), (img == nullptr ? stbi_failure_reason() : "loaded"));
            if (img != nullptr) {
                stbir_resize_uint8(img, width, height, 0, (unsigned char*)resized_img, resolution, resolution, 0, 4);
                resized_image = std::string(resized_img, resolution * resolution * 4);
                stbi_image_free(img);
            }
        } else if (image_data.length() > 0) {
            stbir_resize_uint8((unsigned char*)image_data.c_str(), 184, 184, 0, (unsigned char*)resized_img, resolution, resolution, 0, 4);
            resized_image = std::string(resized_img, resolution * resolution * 4);
        }
        free(resized_img);
    }

    reset_LastError();
    return resized_image;
}

bool Local_Storage::save_screenshot(std::string const& image_path, uint8_t* img_ptr, int32_t width, int32_t height, int32_t channels)
{
    std::string screenshot_path = std::move(save_directory + appid + screenshots_folder + PATH_SEPARATOR); 
    create_directory(screenshot_path);
    screenshot_path += image_path;
    return stbi_write_png(screenshot_path.c_str(), width, height, channels, img_ptr, 0) == 1;
}

std::string Local_Storage::sanitize_string(std::string name)
{
    return sanitize_file_name(name);
}

std::string Local_Storage::desanitize_string(std::string name)
{
    return desanitize_file_name(name);
}

#endif
