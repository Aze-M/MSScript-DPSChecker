
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

	//vector for third rewrite of script reader
	std::vector < std::map<std::string, std::string> > attacks_vec;

	//key - value map for consts and locals
	std::map<std::string, std::string>* consts_map = new std::map<std::string, std::string>;

	//const to store attack data.
	attack_data_t attack_map;

	//parameters for dps simulation
	int health, health_init;
	float resist;
	bool valid_input = false;

	//parameters for multithreading
	int max_threads = std::thread::hardware_concurrency();

	fs::path directory = fs::current_path();
	fs::path file_path;

	//trying to prevent recursion skipping cin's
	if (std::cin.fail()) {
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}

	//request input of script file
	std::cout << "Insert script name (or quit):" << std::endl;
	std::cin >> filename;
	std::cin.clear();
	std::cin.ignore();
	if (filename == "quit") {
		return 0;
	}

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
	std::cout << "Enter charge level (0 for default hits, sometimes inaccurate due to reading algorithm):" << std::endl;
	std::cin >> attack_nr;
	if (attack_nr < 0 || std::cin.fail()) {
		valid_input = false;
	}
	std::cin.clear();
	std::cin.ignore();

	std::cout << "---------------" << std::endl;


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

	//file_path = find_file(filename, directory);
	file_path = find_file_threaded(filename, directory);

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

			if ((cur_line.find("register_normal") != nfound)) {

				locals->push_back(cur_line);

			}

			if ((cur_line.find("register_charge1") != nfound)) {

				locals->push_back(cur_line);

			}

			if ((cur_line.find("register_charge2") != nfound)) {

				locals->push_back(cur_line);

			}

			if ((cur_line.find("local reg.attack") != nfound) || (cur_line.find("registerattack") != nfound)) {

				locals->push_back(cur_line);

			}

			if (cur_line.find("multiply reg.attack") != nfound) {

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
			std::vector<std::string> cur_split = split_string(cur, ' ');

			for (int idx_2 = 0; idx_2 < cur_split.size(); idx_2++) {
				if (cur_split[idx_2].size() == 0) {
					cur_split.erase(cur_split.begin() + idx_2);
					idx_2--;
				}
			}

			consts_map->insert({ cur_split[1], cur_split[2] });
		};

		std::cout << "Mapping locals from " << filename << std::endl;
		int reg_increment = 0;

		//create a map for locals, attempt tracking atk type
		std::map<std::string, std::string> locals_map;
		int reg_atk = 0;

		//map import locals immediately so they can be reserved.
		for (int idx = 0; idx < locals->size(); idx++) {
			std::string cur = locals->at(idx);
			std::vector cur_split = split_string(cur, ' ');

			//filter out empties and brackets and callevent
			for (int idx_2 = 0; idx_2 < cur_split.size(); idx_2++) {
				if (cur_split[idx_2].size() == 0 || cur_split[idx_2] == "{" || cur_split[idx_2] == "callevent") {
					cur_split.erase(cur_split.begin() + idx_2);
					idx_2--;
				}
			}

			//read charge amt > headers
			if (cur_split.size() > 1 && cur_split[1] == "reg.attack.chargeamt") {
				std::string out = cur_split[2];
				//removie percentage sign so stof doesn't pee itself
				if (out != "" && out.back() == '%') {
					out.pop_back();
				}

				reg_atk = (stoi(out) / 100);
				locals_map.insert({ cur_split[1] , cur_split[2] });
				continue;
			}

			//try to read headers
			if (cur_split[0] == "register_normal") {
				reg_atk = 0;
				continue;
			}
			else if (cur_split[0] == "register_charge1") {
				reg_atk = 1;
				continue;
			}
			else if (cur_split[0] == "register_charge2") {
				reg_atk = 2;
				continue;
			}

			//save multiplier handles
			if (cur_split[0] == "multiply") {
				locals_map.insert({ "mult_" + cur_split[1] , cur_split[2] });
				continue;
			}

			if (cur_split[0] == "registerattack") {
				if (reg_atk > attacks_vec.size()) {
					attacks_vec.resize(reg_atk + 1);
					attacks_vec[reg_atk] = locals_map;
					locals_map.clear();
					continue;
				}
				else if (attacks_vec.at(reg_atk).size() == 0) {
					attacks_vec[reg_atk] = locals_map;
					locals_map.clear();
					continue;
				}
				//fallback in case a header is missing, increment charge value
				else {
					reg_atk++;
					attacks_vec.resize(reg_atk + 1);
					attacks_vec[reg_atk] = locals_map;
					locals_map.clear();
					continue;
				}
			}

			locals_map.insert({ cur_split[1] , cur_split[2] });

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
			include_path = find_file_threaded(include_split[1], directory);

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

					if ((cur_line.find("register_normal") != nfound)) {

						locals->push_back(cur_line);

					}

					if ((cur_line.find("register_charge1") != nfound)) {

						locals->push_back(cur_line);

					}

					if ((cur_line.find("register_charge2") != nfound)) {

						locals->push_back(cur_line);

					}

					if ((cur_line.find("local reg.attack") != nfound) || (cur_line.find("registerattack") != nfound)) {

						locals->push_back(cur_line);

					}

					if (cur_line.find("multiply reg.attack") != nfound) {

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
						consts_map->insert({ cur_split[1], cur_split[2] });
					}
				};

				std::cout << "Mapping locals from " << include_split[1] << ".script" << std::endl;

				//map import locals immediately so they can be reserved.
				for (int idx = 0; idx < locals->size(); idx++) {
					std::string cur = locals->at(idx);
					std::vector cur_split = split_string(cur, ' ');

					//filter out empties and brackets and callevent
					for (int idx_2 = 0; idx_2 < cur_split.size(); idx_2++) {
						if (cur_split[idx_2].size() == 0 || cur_split[idx_2] == "{" || cur_split[idx_2] == "callevent") {
							cur_split.erase(cur_split.begin() + idx_2);
							idx_2--;
						}
					}


					//read charge amt > headers
					if (cur_split.size() > 1 && cur_split[1] == "reg.attack.chargeamt") {
						std::string out = cur_split[2];
						//removie percentage sign so stof doesn't pee itself
						if (out != "" && out.back() == '%') {
							out.pop_back(); 
							reg_atk = (stoi(out) / 100);
							locals_map.insert({ cur_split[1] , cur_split[2] });
							continue;
						}
					}


					//try to read headers
					if (cur_split[0] == "register_normal") {
						reg_atk = 0;
						continue;
					}
					else if (cur_split[0] == "register_charge1") {
						reg_atk = 1;
						continue;
					}
					else if (cur_split[0] == "register_charge2") {
						reg_atk = 2;
						continue;
					}

					//save multiplier handles
					if (cur_split[0] == "multiply") {
						locals_map.insert({ "mult_" + cur_split[1] , cur_split[2] });
						continue;
					}

					//if an attack is registered take it with you
					if (cur_split[0] == "registerattack") {
						if (reg_atk >= attacks_vec.size()) {
							attacks_vec.resize(reg_atk + 1);
							attacks_vec[reg_atk] = locals_map;
							locals_map.clear();
							continue;
						}
						else if (attacks_vec.at(reg_atk).size() == 0) {
							attacks_vec[reg_atk] = locals_map;
							locals_map.clear();
							continue;
						}
						//fallback in case a header is missing, increment charge value
						else {
							reg_atk = attacks_vec.size() + 1;
							attacks_vec.resize(reg_atk + 1);
							attacks_vec[reg_atk] = locals_map;
							locals_map.clear();
							continue;
						}
					}

					locals_map.insert({ cur_split[1] , cur_split[2] });

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

	//local mapping was moved and redone.
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



	//begin grabbing attack stats from locals
	if (weapon_type == "m") {
		attack_map = map_attack_data_melee(consts_map, attacks_vec, skill_level, attack_nr);
	}
	else {
		std::cout << "Support for non melee not yet implemented" << std::endl;
		delete consts_map;
		main();
	}

	if (attack_map.conv_errors > 0) {
		std::cout << "---------------" << std::endl;
		std::cout << "Something went wrong, restarting." << std::endl;
		std::cout << "Please report the failed script if it should be supported." << std::endl;
		std::cout << "---------------" << std::endl;
		delete consts_map;
		main();
	};

	std::cout << "Mapping successful." << std::endl;
	std::cout << "---------------" << std::endl;

	std::cout << "Enter dummy HP (whole numbers only):" << std::endl;
	std::cin >> health;
	if (std::cin.fail()) {
		std::cout << "Invalid input." << std::endl;
		std::cin.clear();
		std::cin.ignore();
		std::cout << "---------------" << std::endl;
		main();
	}
	health_init = health;
	std::cout << "Enter dummy resistance in decimal (e.g. 0.4 for 60% resistance);" << std::endl;
	std::cin >> resist;
	if (std::cin.fail()) {
		std::cout << "Invalid input." << std::endl;
		std::cin.clear();
		std::cin.ignore();
		std::cout << "---------------" << std::endl;
		main();
	}
	std::cout << "---------------" << std::endl;

	std::cout << "Starting charge level " << attack_nr << " simulation.." << std::endl;

	//get attack interval for attack1's
	std::uint32_t attacks = 0;
	float attack_interval = attack_map.attack_duration;

	//now multithreading for faster calcs.
	attacks = multi_thread_atk(health, resist, attack_map, max_threads);

	/*
	while (health > 0) {
			health = do_attack(health, resist, attack_map);
			attacks++;
			std::cout << "Remaining health: " << health << std::endl;
		}
	*/

	float time_passed = attacks * attack_interval;

	std::cout << "Killed a " << health_init << "HP target in " << time_passed << " Seconds" << std::endl;
	std::cout << "This is a dps of " << (health_init / time_passed) << std::endl;

	std::cout << "---------------" << std::endl;

	delete consts_map;
	attacks_vec.clear();
	attacks_vec.shrink_to_fit();
	main();

	return 0;
}
