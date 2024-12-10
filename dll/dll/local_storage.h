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

#ifndef LOCAL_STORAGE_INCLUDE_H
#define LOCAL_STORAGE_INCLUDE_H

#include "base.h"

#define MAX_FILENAME_LENGTH 300

union image_pixel_t {
    uint32_t pixel;

    struct pixel_channels_t {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    } channels;
};

struct image_t {
    size_t width{};
    size_t height{};
    std::vector<image_pixel_t> pix_map{};
};

class Local_Storage {
private:
    static std::string saves_folder_name;

public:
    static constexpr char inventory_storage_folder[]   = "inventory";
    static constexpr char settings_storage_folder[]    = "settings";
    static constexpr char remote_storage_folder[]      = "remote";
    static constexpr char stats_storage_folder[]       = "stats";
    static constexpr char leaderboard_storage_folder[] = "leaderboard";
    static constexpr char user_data_storage[]          = "local";
    static constexpr char screenshots_folder[]         = "screenshots";
    static constexpr char game_settings_folder[]       = "steam_settings";

    static std::string get_program_path();
    static std::string get_game_settings_path();
    static std::string get_user_appdata_path();

    static int get_file_data(const std::string &full_path, char *data, unsigned int max_length, unsigned int offset=0);
    static int store_file_data(std::string folder, std::string file, const char *data, unsigned int length);
    
    static std::vector<std::string> get_filenames_path(std::string path);
    static std::vector<std::string> get_folders_path(std::string path);

    static void set_saves_folder_name(std::string_view str);
    static const std::string& get_saves_folder_name();

private:
    std::string save_directory{};
    std::string appid{}; // game appid
    
public:
    Local_Storage(const std::string &save_directory);

    const std::string& get_current_save_directory() const;
    void setAppId(uint32 appid);
    int store_data(std::string folder, std::string file, char *data, unsigned int length);
    int store_data_settings(std::string file, const char *data, unsigned int length);
    int get_data(std::string folder, std::string file, char *data, unsigned int max_length, unsigned int offset=0);
    unsigned int data_settings_size(std::string file);
    int get_data_settings(std::string file, char *data, unsigned int max_length);
    int count_files(std::string folder);
    bool iterate_file(std::string folder, int index, std::string &output_filename, int32 *output_size);
    bool file_exists(std::string folder, std::string file);
    unsigned int file_size(std::string folder, std::string file);
    bool file_delete(std::string folder, std::string file);
    uint64_t file_timestamp(std::string folder, std::string file);
    std::string get_global_settings_path();
    std::string get_path(std::string folder);

    bool update_save_filenames(std::string folder);

    bool load_json(const std::string &full_path, nlohmann::json& json);
    bool load_json_file(std::string folder, std::string const& file, nlohmann::json& json);
    bool write_json_file(std::string folder, std::string const& file, nlohmann::json const& json);

    std::vector<image_pixel_t> load_image(std::string const& image_path);
    static std::string load_image_resized(std::string const& image_path, std::string const& image_data, int resolution);
    bool save_screenshot(std::string const& image_path, uint8_t* img_ptr, int32_t width, int32_t height, int32_t channels);

    static std::string sanitize_string(std::string name);
    static std::string desanitize_string(std::string name);
};

#endif // LOCAL_STORAGE_INCLUDE_H
