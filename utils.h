//
// Created by istoffa on 12/5/16.
//

#ifndef DISCRETE_SIMS_UTILS_H
#define DISCRETE_SIMS_UTILS_H

#include <string>

#include <cstdlib>


extern const int TRUCK_CAP;
extern const int UNLOAD;
extern const int SERVICE;
extern const int TRUCK_AVG_SPEED;
extern const int TRUCK_MAX_SPEED;

extern double plane_size;
extern int population;
extern double garbage_k;
extern double industry_k;
extern int houses_n;
extern int factories_n;

// Calculated from inputs
// Garbage production
extern float house_prod; //Kg odpadu za tyzden na dom
extern float fact_prod;
// Exponential distribution calculated on travel time avg_speed/avg_distance -> avg time

extern int move_collect; //sec Distance between houses_n
extern int move_depo; //sec Distance to depo
extern int move_stor; //sec Distance to landfill
extern int truck_min;
extern int truck_max;
extern int truck_n;
extern int optimize;
extern int days;

extern std::string outputfile;

const std::string HELP_MESSAGE = "help, no_argument, 0, h\n"
		"output-file, required_argument, 0, f\n"
		"city-size, required_argument, 0, c\n"
		"person-production, required_argument, 0, p\n"
		"industry-production, required_argument, 0, i\n"
		"houses_n, required_argument, 0, u\n"
		"factories_n, required_argument, 0, t\n"
		"population, required_argument, 0, l\n";

int initParams(int argc, char* argv[]);

#endif //DISCRETE_SIMS_UTILS_H
