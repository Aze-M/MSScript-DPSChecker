#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <regex>
#include <chrono>
#include <map>
#include <random>
#include <filesystem>

namespace fs = std::filesystem;
namespace ch = std::chrono;

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

std::string lookup_info(const std::map<std::string, std::string> locals_map, const std::map<std::string, std::string> consts_map, const std::string info, const int attack_nr);

fs::path find_file(std::string filename, fs::path directory);

std::vector<std::string> split_string(std::string str_in, char split_at);

std::string find_in_map_ss(std::string key_to_find, std::map<std::string, std::string> map_to_search);

float find_in_map_sf(std::string key_to_find, std::map<std::string, float> map_to_search);

attack_data_t map_attack_data_melee(std::map<std::string, std::string> consts, std::map<std::string, std::string> locals, int skill_level, int attack_nr);

int do_attack(int health, float resist, attack_data_t attack_data);