
#pragma once
#include "Helpers.h"


int main()
{
	//init variables
	std::string filename = "";
	std::string weapon_type = "";
	int attack_nr;
	int skill_level = 0;

	//stream for file
	std::ifstream file;

	//npos
	auto nfound = std::string::npos;

	//global lists to use with dps simulation
	//std::regex whitespaces("[[:space:]*]");
	std::regex whitespaces("[ \\t]+");
	std::vector<std::string>* consts = new std::vector<std::string>;
	std::vector<std::string>* includes = new std::vector<std::string>;
	std::vector<std::string>* locals = new std::vector<std::string>;

	//key - value map for consts and locals
	std::map<std::string, std::string> consts_map;
	std::map<std::string, std::string> locals_map;

	//const to store attack data.
	attack_data_t attack_map;

	//parameters for dps simulation
	int health, health_init;
	float resist;
	bool valid_input = false;

	fs::path directory = fs::current_path();
	fs::path file_path;

	//request input of script file
	std::cin.clear();
	std::cout << "Insert script name (or quit):" << std::endl;
	std::cin >> filename;
	std::cin.clear();
	std::cin.ignore();

	//request input of m r or s and make sure it is valid;
	std::cout << "Enter m r or s for melee, ranged, or spell:" << std::endl;
	std::cin >> weapon_type;
	if (weapon_type == "m" || weapon_type == "r" || weapon_type == "s" || std::cin.fail()) {
		valid_input = true;
	}
	std::cin.clear();
	std::cin.ignore();

	//request input of skill level as number and check if it is valid
	std::cout << "Enter skill level as whole number:" << std::endl;
	std::cin >> skill_level;
	if (skill_level < 1 || std::cin.fail()) {
		valid_input = false;
	}
	std::cin.clear();
	std::cin.ignore();

	//request info on attack_nr
	std::cout << "Enter attack register (0 for default hits):" << std::endl;
	std::cin >> attack_nr;
	if (attack_nr < 0 || std::cin.fail()) {
		valid_input = false;
	}
	std::cin.clear();
	std::cin.ignore();

	std::cout << "---------------" << std::endl;

	if (filename == "quit") {
		return 0;
	}

	if (!valid_input) {
		std::cout << "Invalid inputs. Restarting." << std::endl;
		std::cout << "---------------" << std::endl;
		main();
	}

	//append .script if it does not exist
	if (filename.find(".script") == nfound) {
		std::cout << "Appending missing .script." << std::endl;
		filename.append(".script");
	}

	//look for the file
	std::cout << "Looking for script: " << filename << std::endl;

	file_path = find_file(filename, directory);

	//if no file found just exit
	if (file_path.empty()) {
		main();
	}

	file.open(file_path);

	//if file found open and start looking for useful data.
	if (file.is_open()) {
		std::string cur_line_ws;

		//get includes consts and locals from local script
		while (std::getline(file, cur_line_ws)) {

			std::string cur_line = regex_replace(cur_line_ws, whitespaces, " ");

			if (cur_line.find("const") != nfound) {

				consts->push_back(cur_line);

			}

			if (cur_line.find("#include") != nfound) {

				includes->push_back(cur_line);

			}

			if ((cur_line.find("local reg.attack") != nfound) || (cur_line.find("registerattack") != nfound)) {

				locals->push_back(cur_line);

			}

		}

		std::cout << "Found all includes consts and locals." << std::endl;
		std::cout << "Consts:" << consts->size() << std::endl;
		std::cout << "Locals: " << locals->size() << std::endl;
		std::cout << "Includes: " << includes->size() << std::endl;

		std::cout << "Mapping constants from " << filename << std::endl;

		//map import constant immediately so they can be reserved.
		for (int idx = 0; idx < consts->size(); idx++) {
			std::string cur = consts->at(idx);
			std::vector cur_split = split_string(cur, ' ');

			for (int idx_2 = 0; idx_2 < cur_split.size(); idx_2++) {
				if (cur_split[idx_2].size() == 0) {
					cur_split.erase(cur_split.begin() + idx_2);
					idx_2--;
				}
			}

			consts_map.insert({ cur_split[1], cur_split[2] });
		};

		std::cout << "Mapping locals from " << filename << std::endl;
		int reg_increment = 0;
		//map import locals immediately so they can be reserved.
		for (int idx = 0; idx < locals->size(); idx++) {
			std::string cur = locals->at(idx);
			std::vector cur_split = split_string(cur, ' ');


			for (int idx_2 = 0; idx_2 < cur_split.size(); idx_2++) {
				if (cur_split[idx_2].size() == 0) {
					cur_split.erase(cur_split.begin() + idx_2);
					idx_2--;
				}
			}

			if (cur_split[0] == "registerattack") {
				reg_increment++;
				continue;
			}

			locals_map.insert({ std::to_string(reg_increment) + "_" + cur_split[1] , cur_split[2] });

		};


		consts->clear();
		consts->shrink_to_fit();

		locals->clear();
		locals->shrink_to_fit();

		std::cout << "---------------" << std::endl;
		std::cout << "Attempting to load includes..." << std::endl;

		for (int idx = 0; idx < includes->size(); idx++) {
			//split include string
			std::vector<std::string> include_split = split_string(includes->at(idx), ' ');
			//prepare vars to load include file
			std::ifstream include;
			fs::path include_path;

			//distill file name out of path
			while (include_split[1].find('/') != std::string::npos)
			{
				include_split[1] = split_string(include_split[1], '/')[1];
			}

			std::cout << "Attempting to load #" << idx + 1 << ". Filename: " << include_split[1] << ".script..." << std::endl;

			//try to find file
			include_path = find_file(include_split[1], directory);

			include.open(include_path);

			if (include.is_open()) {
				while (std::getline(include, cur_line_ws)) {

					std::string cur_line = regex_replace(cur_line_ws, whitespaces, " ");

					if (cur_line.find("const") != nfound) {

						consts->push_back(cur_line);

					}

					if (cur_line.find("#include") != nfound) {

						includes->push_back(cur_line);

					}

					if ((cur_line.find("local reg.attack") != nfound) || (cur_line.find("registerattack") != nfound)) {

						locals->push_back(cur_line);

					}

				}

				include.close();

				std::cout << "Done loading include #" << idx + 1 << std::endl;
				std::cout << "Extra Consts:" << consts->size() << std::endl;
				std::cout << "Extra Locals:" << locals->size() << std::endl;
				std::cout << "Includes: " << includes->size() << std::endl;

				std::cout << "Mapping constants from " << include_split[1] << ".script" << std::endl;

				for (int idx = 0; idx < consts->size(); idx++) {
					std::string cur = consts->at(idx);
					std::vector cur_split = split_string(cur, ' ');

					for (int idx_2 = 0; idx_2 < cur_split.size(); idx_2++) {
						if (cur_split[idx_2].size() == 0) {
							cur_split.erase(cur_split.begin() + idx_2);
							idx_2--;
						}
					}

					if (find_in_map_ss(cur_split[1], consts_map) == "") {
						consts_map.insert({ cur_split[1], cur_split[2] });
					}
				};

				std::cout << "Mapping locals from " << include_split[1] << ".script" << std::endl;

				int reg_increment = 0;
				for (int idx = 0; idx < locals->size(); idx++) {
					std::string cur = locals->at(idx);
					std::vector cur_split = split_string(cur, ' ');

					for (int idx_2 = 0; idx_2 < cur_split.size(); idx_2++) {
						if (cur_split[idx_2].size() == 0) {
							cur_split.erase(cur_split.begin() + idx_2);
							idx_2--;
						}
					}

					if (cur_split[0] == "registerattack") {
						reg_increment++;
						continue;
					}

					if (find_in_map_ss(std::to_string(reg_increment) + "_" + cur_split[1], locals_map) == "") {
						locals_map.insert({ std::to_string(reg_increment) + "_" + cur_split[1], cur_split[2] });
					}
				};

				consts->clear();
				consts->shrink_to_fit();

				locals->clear();
				locals->shrink_to_fit();

				std::cout << "Closing " << include_split[1] << ".script" << std::endl;
				std::cout << "---------------" << std::endl;
			};

		}

		std::cout << "Closing " << filename << std::endl;
		file.close();
		std::cout << "---------------" << std::endl;

	}

	std::cout << "Done mapping Consts." << std::endl;

	//no longer mapping locals instead looking for usable vars in consts.
	/*
	std::cout << "Mapping Locals..." << std::endl;


	for (int idx = 0; idx < locals.size(); idx++) {
		std::string cur = locals[idx];
		std::vector cur_split = split_string(cur, ' ');

		for (int idx_2 = 0; idx_2 < cur_split.size(); idx_2++) {
			if (cur_split[idx_2].size() == 0) {
				cur_split.erase(cur_split.begin() + idx_2);
				idx_2--;
			}
		}

		//if this is part of a longer statement
		while (cur_split[0] != "local") {
			cur_split.erase(cur_split.begin());
		}

		//if we already have the value, skip to next loop
		if (auto search = locals_map.find(cur_split[1]); search != locals_map.end()) {
			continue;
		}

		//if the have the referred consts, set the value to the consts value
		if (auto search = consts_map.find(cur_split[2]); search != consts_map.end()) {
			cur_split[2] = search->second;
		}
		else {
			continue;
		};

		std::cout << cur_split[1] << " " << cur_split[2] << std::endl;
		locals_map.insert({ cur_split[1], cur_split[2] });
	};

	std::cout << "Done mapping Locals." << std::endl;
	*/

	std::cout << "Discarding cached unneeded data." << std::endl;

	//clear memory used by import.
	delete includes;
	delete consts;
	delete locals;

	std::cout << "---------------" << std::endl;

	std::cout << "Enter dummy HP (whole numbers only):" << std::endl;
	std::cin >> health;
	std::cin.clear();
	std::cin.ignore();
	health_init = health;
	std::cout << "Enter dummy resistance in decimal (e.g. 0.4 for 60% resistance);" << std::endl;
	std::cin >> resist;
	std::cin.clear();
	std::cin.ignore();
	std::cout << "---------------" << std::endl;

	//begin grabbing attack stats from locals
	if (weapon_type == "m") {
		attack_map = map_attack_data_melee(consts_map,locals_map, skill_level, attack_nr);
	}
	else {
		std::cout << "Support for non melee not yet implemented" << std::endl;
		main();
	}

	if (attack_map.conv_errors > 0) {
		std::cout << "---------------" << std::endl;
		std::cout << "Something went wrong, restarting." << std::endl;
		std::cout << "Please make sure your script complies with the inputs given." << std::endl;
		std::cout << "---------------" << std::endl;
		main();
	};

	std::cout << "Starting basic attack 1 simulation.." << std::endl;

	//get attack interval for attack1's
	std::uint32_t attacks = 0;
	float attack_interval = attack_map.attack_duration;

	while (health > 0) {
		health = do_attack(health, resist, attack_map);
		attacks++;
		std::cout << "Remaining health: " << health << std::endl;
	}

	float time_passed = attacks * attack_interval;

	std::cout << "Killed a " << health_init << "HP target in " << time_passed << " Seconds" << std::endl;
	std::cout << "This is a dps of " << (health_init / time_passed) << std::endl;

	std::cout << "---------------" << std::endl;
	main();

	return 0;
}
