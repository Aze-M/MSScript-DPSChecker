
#pragma once

#include <iostream>
#include <thread>
#include <mutex>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <regex>
#include <map>
#include <random>
#include <filesystem>
#include <atomic>

namespace fs = std::filesystem;

struct attack_data_t {
	float damage;
	float damage_range;
	float charge_multi;
	float crit_threshold;
	float crit_multi;
	float skill_level;
	float attack_duration;
	int conv_errors;
};

//std::string lookup_info_arr_map(const std::vector<std::string> info_names, const std::map<std::string, std::string> info_map);

fs::path find_file_threaded(std::string file_name, fs::path directory);

void find_file_thread(std::string file_name, const fs::path directory, fs::path& path_storage, std::mutex& path_mutex);

std::string lookup_info(const std::map<std::string, std::string>& locals_map, const std::map<std::string, std::string>*& consts_map, std::string local_name, const int& attack_nr);

fs::path find_file(std::string filename, fs::path directory);

std::vector<std::string> split_string(std::string& str_in, char split_at);

std::string find_in_map_ss(const std::string& key_to_find, std::map<std::string, std::string>& map_to_search);
std::string find_in_map_ss(const std::string& key_to_find, std::map<std::string, std::string>*& map_to_search);

float find_in_map_sf(const std::string& key_to_find, std::map<std::string, float>*& map_to_search);

attack_data_t map_attack_data_melee(std::map<std::string, std::string>*& consts, std::vector<std::map<std::string, std::string>> attacks_vec, const int skill_level, const int attack_nr);

int do_attack(int& health, float& resist, attack_data_t& attack_data);

void do_attack_thread(int& health, float& resist, attack_data_t& attack_data, int& attack_count);

int multi_thread_atk(int& health, float& resist, attack_data_t& attack_data, int thread_count);