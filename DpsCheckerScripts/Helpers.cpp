
#include "Helpers.h"

fs::path find_file(std::string filename, fs::path directory) {
	fs::path file_path;
	fs::path file_initial = directory;

	for (const fs::directory_entry& entry : fs::recursive_directory_iterator(directory)) {
		file_path = entry.path();

		if (file_path.u8string().find(filename) != std::string::npos) {
			std::cout << "Found " << filename << " @ " << file_path.u8string() << std::endl;
			return file_path;
		}

	};

	std::cout << "Could not find specified script file." << std::endl;
	std::cout << "---------------" << std::endl;
	fs::path emptypath;
	return emptypath;
}

//path multithreading
fs::path find_file_threaded(std::string file_name, fs::path directory) {
	std::vector<std::thread> threads;
	fs::path out;
	std::mutex path_mutex;
	int thread_count = std::thread::hardware_concurrency();

	for (int idx = 0; idx < thread_count; idx++) {
		threads.emplace_back(find_file_thread, std::ref(file_name), std::ref(directory), std::ref(out), std::ref(path_mutex));
	}

	for (auto& thread : threads)
	{
		thread.join();
	}

	if (!out.empty()) {
		std::cout << "Found " << file_name << " @ " << out.u8string() << std::endl;
		return out;
	}
	else {
		std::cout << "Could not find specified script file." << std::endl;
		std::cout << "---------------" << std::endl;
		fs::path emptypath;
		return emptypath;
	}
}


void find_file_thread(std::string file_name, const fs::path directory, fs::path& path_storage, std::mutex& path_mutex) {
	fs::path file_path;
	fs::path path_initial = directory;

	for (const fs::directory_entry& entry : fs::directory_iterator(path_initial)) {
		if (!path_storage.empty()) {
			break;
		};

		file_path = entry.path();

		if (entry.is_directory()) {
			find_file_thread(file_name, entry.path(), path_storage, path_mutex);
		}
		else if (file_path.u8string().find(file_name) != std::string::npos) {
			std::unique_lock<std::mutex> lock(path_mutex);
			path_storage = file_path;
			break;
		}
	}
}

//now handled by reading locals for the bound statics.
/*
std::string lookup_info_vec_map(const std::vector<std::string> info_names, const std::map<std::string, std::string>& info_map) {
	std::string out = "";
	int idx = 0;

	while (out == "" && idx < info_names.size()) {
		out = find_in_map_ss(info_names[idx], info_map);
		idx++;
	}

	return out;
}
*/

//new handling
std::string lookup_info(std::map<std::string, std::string>& locals_map, std::map<std::string, std::string>*& consts_map, std::string local_name, const int& attack_nr) {
	std::string out = "";

	std::string bound_const = "";

	bound_const = find_in_map_ss(local_name, locals_map);

	out = find_in_map_ss(bound_const, consts_map);

	return out;
}

std::vector<std::string> split_string(std::string& str_in, char split_at) {
	//turn string into stream
	std::stringstream ss;
	ss.str(str_in);
	//output string buffer
	std::string str_out_tmp;
	//output vec
	std::vector<std::string> str_out;

	while (getline(ss, str_out_tmp, split_at)) {
		str_out.push_back(str_out_tmp);
	}

	return str_out;
}

std::string find_in_map_ss(const std::string& key_to_find, std::map<std::string, std::string>& map_to_search) {
	std::string out = "";

	if (auto search = map_to_search.find(key_to_find); search != map_to_search.end()) {
		out = search->second;
	}
	return out;
}

std::string find_in_map_ss(const std::string& key_to_find, std::map<std::string, std::string>*& map_to_search) {
	std::string out = "";

	if (auto search = map_to_search->find(key_to_find); search != map_to_search->end()) {
		out = search->second;
	}
	return out;
}



float find_in_map_sf(const std::string& key_to_find, std::map<std::string, float>& map_to_search) {
	auto search = map_to_search.find(key_to_find);
	return (search != map_to_search.end()) ? search->second : 0.0;
}

attack_data_t map_attack_data_melee(std::map<std::string, std::string>*& consts, std::vector<std::map<std::string, std::string>> attacks_vec, const int skill_level, const int attack_nr) {
	// find the values in consts that are needed for attack simulation. map them to given map.
	int conv_errors = 0; // increment when conversion fails

	//init return struct
	attack_data_t attack_data;

	if (attack_nr > attacks_vec.size()) {
		std::cout << "Charge level " << attack_nr << " exceeds the amount found (" << attacks_vec.size() << ")." << std::endl;
		conv_errors++;
		attack_data.conv_errors = conv_errors;
		return attack_data;
	}

	//convenience rebind
	std::map<std::string, std::string>& locals = attacks_vec[attack_nr];

	//damage
	std::string attack_damage_string = lookup_info(locals, consts, "reg.attack.dmg", attack_nr);
	float attack_damage;
	try {
		attack_damage = std::stof(attack_damage_string);
	}
	catch (std::exception ex) {
		conv_errors++;
		attack_damage = 0;
		std::cout << "Error while converting Attack Damage: " << ex.what() << std::endl;
		if (attack_nr > 0) {
			std::string confirm = "";

			std::cout << "Fallback to attack 0? (y/n)" << std::endl;
			std::cin >> confirm;

			if (confirm == "y" && !std::cin.fail()) {
				conv_errors--;
				attack_damage = std::stof(lookup_info(locals, consts, "reg.attack.dmg", 0));
				std::cout << "---------------" << std::endl;
			}
		}
	}

	//damage_range (+ or - from damage)
	std::string attack_damage_range_string = lookup_info(locals, consts, "reg.attack.dmg.range", attack_nr);
	float attack_damage_range;
	try {
		attack_damage_range = std::stof(attack_damage_range_string);
	}
	catch (std::exception ex) {
		conv_errors++;
		attack_damage_range = 0;
		std::cout << "Error while converting Attack Damage Range: " << ex.what() << std::endl;
		if (attack_nr > 0) {
			std::string confirm = "";

			std::cout << "Fallback to attack 0? (y/n)" << std::endl;
			std::cin >> confirm;

			if (confirm == "y" && !std::cin.fail()) {
				conv_errors--;
				attack_damage_range = std::stof(lookup_info(locals, consts, "reg.attack.dmg.range", 0));
				std::cout << "---------------" << std::endl;
			}
		}
	}

	//hit delay end (how long the entire swing takes)
	std::string attack_delay_end_string = lookup_info(locals, consts, "reg.attack.delay.end", attack_nr);
	float attack_delay_end;
	try {
		attack_delay_end = std::stof(attack_delay_end_string);
	}
	catch (std::exception ex) {
		conv_errors++;
		attack_delay_end = 0;
		std::cout << "Error while converting Attack Delay End: " << ex.what() << std::endl;
		if (attack_nr > 0) {
			std::string confirm = "";

			std::cout << "Fallback to attack 0? (y/n)" << std::endl;
			std::cin >> confirm;

			if (confirm == "y" && !std::cin.fail()) {
				conv_errors--;
				attack_delay_end = std::stof(lookup_info(locals, consts, "reg.attack.delay.end", 0));
				std::cout << "---------------" << std::endl;
			}
		}
	}

	//crit - base 4%
	float attack_crit_threshold = 95;

	//critmulti - base 1.5
	float attack_crit_multi = 1.5;

	//pass on skill_level as is

	//map to struct and return.

	attack_data.damage = attack_damage;
	attack_data.damage_range = attack_damage_range;
	attack_data.charge_multi = attack_nr + 1;
	attack_data.crit_threshold = attack_crit_threshold;
	attack_data.crit_multi = attack_crit_multi;
	attack_data.skill_level = skill_level;
	attack_data.attack_duration = attack_delay_end < attack_nr ? attack_nr : attack_delay_end;
	attack_data.conv_errors = conv_errors;

	return attack_data;
}

int do_attack(int& health, float& resist, attack_data_t& attack_data) {
	//make rng;
	std::random_device sus;
	std::mt19937 rng_engine(sus());

	//get damage parameters
	float damage_min = attack_data.damage;
	float damage_range_data = attack_data.damage_range;

	//randomize damage by range
	std::uniform_int_distribution<int> rng(-damage_range_data, damage_range_data);
	int damage_range = rng(sus);

	//crit?
	std::string critstring = "";
	std::uniform_int_distribution<int> rng_crit(1, 99);
	int damage_crit = rng_crit(sus);
	float damage_crit_multi;

	if (damage_crit > attack_data.crit_threshold) {
		damage_crit_multi = attack_data.crit_multi;
		critstring = "Crit!";
	}
	else {
		damage_crit_multi = 1;
	}
	//charge?
	int damage_charge_multi = attack_data.charge_multi;

	//calculate damage fraction by skill level
	float damage_fraction;
	damage_fraction = attack_data.skill_level / 100;
	damage_fraction = std::max(damage_fraction, 0.001f);

	//calculate damage
	float damage = (((damage_min + damage_range) * damage_fraction) * damage_charge_multi) * damage_crit_multi;

	int new_health = health;

	std::cout << "Damage: " << (damage * resist) << " " + critstring << std::endl;

	if (damage > 0) {
		new_health = health - (damage * resist);
	}

	return new_health;
}

//multi threading helpers
std::mutex health_mutex;

void do_attack_thread(int& health, float& resist, attack_data_t& attack_data, int& attack_count) {

	while (true) {
		std::unique_lock<std::mutex> lock(health_mutex);

		if (health < 0) {
			break;
		}

		//make rng;
		std::random_device sus;
		std::mt19937 rng_engine(sus());

		//get damage parameters
		float damage_min = attack_data.damage;
		float damage_range_data = attack_data.damage_range;

		//randomize damage by range
		std::uniform_int_distribution<int> rng(-damage_range_data, damage_range_data);
		int damage_range = rng(sus);

		//crit?
		std::string critstring = "";
		std::uniform_int_distribution<int> rng_crit(1, 99);
		int damage_crit = rng_crit(sus);
		float damage_crit_multi;

		if (damage_crit > attack_data.crit_threshold) {
			damage_crit_multi = attack_data.crit_multi;
			critstring = "Crit!";
		}
		else {
			damage_crit_multi = 1;
		}
		//charge?
		int damage_charge_multi = attack_data.charge_multi;

		//calculate damage fraction by skill level
		float damage_fraction;
		damage_fraction = attack_data.skill_level / 100;
		damage_fraction = std::max(damage_fraction, 0.001f);

		//calculate damage
		float damage = (((damage_min + damage_range) * damage_fraction) * damage_charge_multi) * damage_crit_multi;

		std::cout << "Damage: " << (damage * resist) << " " + critstring << std::endl;

		if (damage > 0) {
			health = health - (damage * resist);
		}

		std::cout << "Remaining Health:" << health << std::endl;

		attack_count++;
	}
}

int multi_thread_atk(int& health, float& resist, attack_data_t& attack_data, int thread_count) {
	//threading
	std::vector<std::thread> dmg_calc_threads;

	//return value
	int attack_count = 0;

	for (int idx = 0; idx < thread_count; idx++) {
		dmg_calc_threads.emplace_back(do_attack_thread, std::ref(health), std::ref(resist), std::ref(attack_data), std::ref(attack_count));
	}

	for (auto& thread : dmg_calc_threads) {
		thread.join();
	}

	return attack_count;
}