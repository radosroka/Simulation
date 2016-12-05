//
// Created by istoffa on 12/5/16.
//

#include "utils.h"

#include <iostream>
#include <cmath>
#include <simlib.h>
#include <getopt.h>
#include <unistd.h>
#include <string>

// Spocitaj simulacne parametre
// TODO: Rado, prida ako parameter poced dni simulacie


using namespace std;

int initParams(int argc, char* argv[])
{
	int c;
	//int digit_optind = 0;
	int arg_ok = 0;

	static struct option long_options[] = {
			{"help", no_argument, 0, 'h'},
			{"output-file", required_argument, 0, 'f'},
			{"city-size", required_argument, 0, 'c'},
			{"person-production", required_argument, 0, 'p'},
			{"industry-production", required_argument, 0, 'i'},
			{"houses-n", required_argument, 0, 'u'},
			{"factories-n", required_argument, 0, 't'},
			{"population", required_argument, 0, 'l'},

			{"optimize", no_argument, &optimize, 1},
			{"trucks-n", required_argument, 0, 0},
			{"trucks-truck_min", required_argument, 0, 1},
			{"trucks-truck_max", required_argument, 0, 2},
			{"days", required_argument, 0, 3},
			{0,0,0,0}
	};

	while(1) {
		//int this_option_optind = optind ? optind : 1;
		int option_index = 0;
		string tmp;
		size_t idx = 0;

		c = getopt_long(argc, argv, "hf:c:p:i:u:t:l:", long_options, &option_index);
		if (c == -1) break;
		switch (c) {
			//TODO: fill cases
		case 0:
			try {
				truck_n = stoll(tmp, &idx);
				if (tmp[idx] != '\0') throw invalid_argument("Bad value");
			} catch (const std::invalid_argument& ia) {
				Print("Error, truck value is invalid!\n");
				return EXIT_FAILURE;
			}
			break;
		case 1:
			try {
				truck_min = stoll(tmp, &idx);
				if (tmp[idx] != '\0') throw invalid_argument("Bad value");
			} catch (const std::invalid_argument& ia) {
				Print("Error, minimum trucks value is invalid!\n");
				return EXIT_FAILURE;
			}
			break;
		case 2:
			try {
				truck_max = stoll(tmp, &idx);
				if (tmp[idx] != '\0') throw invalid_argument("Bad value");
			} catch (const std::invalid_argument& ia) {
				Print("Error, maximium trucks value is invalid!\n");
				return EXIT_FAILURE;
			}
			break;
		case 3:
			try {
				days = stoll(tmp, &idx);
				if (tmp[idx] != '\0') throw invalid_argument("Bad value");
			} catch (const std::invalid_argument& ia) {
				Print("Error, simulation time in days value is invalid!\n");
				return EXIT_FAILURE;
			}
			break;
		case 'h':
			cout << HELP_MESSAGE;
			break;
		case 'f':
			outputfile = optarg;
			SetOutput(outputfile.c_str());
			break;
		case 'c':
			tmp = optarg;
			try {
				plane_size = stoll(tmp, &idx);
				if (tmp[idx] != '\0') throw invalid_argument("Bad value");
			} catch (const std::invalid_argument& ia) {
				Print("Error, city-size value is invalid!\n");
				return EXIT_FAILURE;
			}
			break;
		case 'p':
			tmp = optarg;
			try {
				garbage_k = stod(tmp, &idx);
				if (tmp[idx] != '\0') throw invalid_argument("Bad value");
			} catch (const std::invalid_argument& ia) {
				Print("Error, person-production value is invalid!\n");
				return EXIT_FAILURE;
			}
			break;
		case 'i':
			tmp = optarg;
			try {
				industry_k = stod(tmp, &idx);
				if (tmp[idx] != '\0') throw invalid_argument("Bad value");
			} catch (const std::invalid_argument& ia) {
				Print("Error, industry-production value is invalid!\n");
				return EXIT_FAILURE;
			}
			break;
		case 'u':
			tmp = optarg;
			try {
				houses_n = stoll(tmp, &idx);
				if (tmp[idx] != '\0') throw invalid_argument("Bad value");
			} catch (const std::invalid_argument& ia) {
				Print("Error, houses_n value is invalid!\n");
				return EXIT_FAILURE;
			}
			break;
		case 't':
			tmp = optarg;
			try {
				factories_n = stoll(tmp, &idx);
				if (tmp[idx] != '\0') throw invalid_argument("Bad value");
			} catch (const std::invalid_argument& ia) {
				Print("Error, factories_n value is invalid!\n");
				return EXIT_FAILURE;
			}
			break;
		case 'l':
			tmp = optarg;
			try {
				population = stoll(tmp, &idx);
				if (tmp[idx] != '\0') throw invalid_argument("Bad value");
			} catch (const std::invalid_argument& ia) {
				Print("Error, population value is invalid!\n");
				return EXIT_FAILURE;
			}
			break;
		default:
			Print("?? getopt returned character code 0%o ??\n", c);
		}
	}

	if (optind < argc) {
		Print("non-option ARGV-elements: ");
		while (optind < argc)
			Print("%s ", argv[optind++]);
		Print("\n");
	}

	int ds_houses = sqrt((plane_size*1000000)/houses_n); // metre Vzdialenost medzi domami
	int ppl_per_house = population/houses_n;

// Garbage production
	house_prod = (ppl_per_house*garbage_k)/(365.25); // Kg odpadu za den na dom - priemer
	fact_prod = industry_k/(365.25*factories_n); // Kg odpadu za den na fabriku

// Distance delays in seconds
	//TODO: Over statisticke vzidalenosti, predpokldame stvorcovu siet, mozme si to vobec dovolit ?
	//FIXME: Vypocet rozlozenia vzdialenosti
	move_collect = TRUCK_AVG_SPEED/ds_houses; // sec plati priemer pri optimalnom presune !

	// sec Priemerna vzdialenost k domom pokial je v strede mesta sqrt(((a/2)^2*2^0.5)/pi) = r, plocha ohranicena stvocom za kruznicou a pred kruznicou s polomerom r je rovnaka //for 20 should give result 9.488
	move_depo = TRUCK_MAX_SPEED/(sqrt(((plane_size / 4) * sqrt(2)) / 3.1418)); //depo is in the middle of square
	move_stor = TRUCK_MAX_SPEED/20000; // sec ?? nejake dalsia priemerna vzidalenost podobnym sposobom



	return EXIT_SUCCESS;
}

