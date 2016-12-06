///////////////////////////////////////////////////////////////////////////
//
// Created by istoffa on 11/23/16.
//
//               SIMLIB/C++
//

#include <iostream>
#include <string>
#include <simlib.h>
#include <cmath>

#include <vector>
#include <map>

#include "utils.h"

using namespace std;

// TODO: rado... srsly ?
// Simulation inputs //defaults
double plane_size = 230.22; //km squared
int population = 377028; //people
double garbage_k = 280; //Kg garbage per people/year
double industry_k = 3635845000; //Kg per year total
int houses_n = 51797; //Spread evenly across the city
int factories_n = 10; //Spread evenly across the city

// Calculated from inputs
// Garbage production
float house_prod; //Kg odpadu za tyzden na dom
float fact_prod = industry_k/factories_n;
// Exponential distribution calculated on travel time avg_speed/avg_distance -> avg time
int move_collect; //sec Distance between houses_n
int move_depo; //sec Distance to depo
int move_stor; //sec Distance to landfill

// Constants -notice capitals
// Truck default settings
const int TRUCK_CAP = 3000; // Kg nosnost
const int UNLOAD = 600; // Time to unload the car
const int SERVICE = 60; // Time to load a trash can
const int TRUCK_AVG_SPEED = 10; //Meters/sec
const int TRUCK_MAX_SPEED = 35/3.6; //Meters/sec

const unsigned SHIFT = 7*3600;
const unsigned RECALL = 8*3600 - SHIFT;

// Constants form statistics

// Simulation control global variables, defaults
int truck_min = 1;
int truck_max = 50;
int truck_n = 0;
int optimize = false;
int days = 21;

string outputfile;

// Optimize function
// TODO: Check how simulating works

// Flag for work shift
int work;
double day_start;

class Store landfill("Skladka info", 3);
class Store households("Zariadenia produkujuce komunalny odpad", houses_n);
//class Store missedHH();
class Store factories("Fabriky", factories_n);

//TODO: add suitable statistics
Histogram gathH("Per day vybrane popelnice", 0, 100, 25); //Garbage gathered per day
Stat uncleaned("Per week neupratane domy");
Histogram fieldHours("Pracovne hodiny vodicov", 0, 0.5, 20); //Find out utilisation of trucks
Stat dist("Najazdene vzdialenosti");       //Accumulate distance travelled by trucks
Histogram unloads("Vahy vysypok na skladkach", 0, (TRUCK_CAP*1.2)/20 , 20);
// Class describing act of collecting garbage from houses_n
// Code mode profi ;)

class ProducerUnit /* : public Facility*/ {
	float per_day;
	int cap;
	int last_gather;

public:
	float gather(Process* ptr, float &over_cap)
	{
		float g = (per_day * (Time - last_gather))/(24*3600);
		over_cap = g > cap ? g - cap : 0;
		last_gather = Time;
		return g;
	}

	ProducerUnit() : per_day(house_prod), cap(20), last_gather(Time) { ; }
	ProducerUnit(int capacity, float prod = house_prod)
	{
		last_gather = Time;
		per_day = prod;
		cap = capacity;
	}
};

vector<ProducerUnit*> section;
auto top = section.begin();

class Truck : public Process {
	enum {
		depo,
		collecting,
		storage,
		begin
	} state;
	//parameters
	int remaining;
	int plan_houses;
	//int remain_days;
	//int plan_days;
	//stats
	int capacity;
	int per_day;
	//control
	int per_stop;
	int x;

	void Behavior() {
		//Implemented as state automaton
		while(1) {
			switch (state) {

			case begin:
			break;

			case collecting:

				if (work) {
					// If it is daytime
					if (!households.Full() && remaining > 0) {
						//TODO: na jedno zastavenie mozno obsluzit viac domov 1,2-5 ?
						//TODO: prirobit fabriky
						//TODO: prechod na ProducerUnit
						// And houses_n not cleared, compensate if ungathered houses_n

						per_stop = 1 + Exponential(4);
						for (x = 0; x < per_stop ; x++) {
							if (households.Full() or remaining ==0) {
								break;
							}
							Enter(households, 1);
							remaining--;
						}

						serviceWait(x);

						for (int z = 0; z <= x ; z++) {
							auto H = section.at(households.Used()-1-z);
							float hoarding;
							//Seize(*H);
							garbage += H->gather(this, hoarding);
							overprod += hoarding;
							//Release(*H);
						}

						per_day += x;

						if (garbage > capacity) {
							// Go to landfill if +- full
							state = storage;
							break;
						}

						moveCollectWait();
						break;
					}
					// Else dump trash and wait for next cleaning
				}

				if (garbage > 0) {
					// Dump trash to start clean
					state = storage;
					break;
				}

				if (households.Empty()) {
					// Go wait for next collecting window;
					state = depo;
					break;
				}

				// Wait for another work shift in depo
				moveDepoWait();
				fieldHours((Time - day_start)/3600);
				gathH(per_day);
				per_day = 0;
				Passivate();
				break;

			case storage:

				//Coutns for way to garbage and back
				moveLandfillWait();

				Enter(landfill, 1);
				unloadWait();
				unloads(garbage);
				garbage = 0;
				Leave(landfill, 1);

				state = collecting;
				break;

			case depo:

				moveDepoWait();
				// Restart state;
				fieldHours((Time - day_start)/3600);
				gathH(per_day);
				per_day = 0;
				state = collecting;
				Passivate();
				break;
			}
		}
	};

public:
	float garbage;
	float odometer;
	float overprod;

	Truck(int cap = TRUCK_CAP)
	{
		capacity = cap;
		per_day = 0;
		state = collecting;
	}

	void newRun()
	{
		remaining = plan_houses;
	}

	void setPlan(int houses)//, int interval)
	{
		plan_houses = houses;
		newRun();
	}

	// TODO: Vyber spravne rozlozenia, mozno bude nutny experiment
	void moveDepoWait() { Wait( fabs(Normal(move_depo, move_depo/5)) ); }

	void moveLandfillWait() { Wait( fabs(Normal(move_stor, move_stor/5)) );  }

	void moveCollectWait() { Wait( fabs(Normal(move_collect, move_collect/5)) ); }

	void serviceWait(int x) { Wait(Uniform(0.7*SERVICE, 1.3*SERVICE)); }

	void unloadWait() { Wait(Uniform(0.7*UNLOAD, 1.3*UNLOAD)); }

};

/*
class CanLoader : Truck {
	void Behavior()
	{
		//...
	}
};

class ContainerCarrier : Truck {
	void Behavior()
	{
		//...
	}
};
*/

class Dispatcher : public Event {
	enum {
		new_window = 0,
		work_shift,
		recall,
		no_shift,
		rest_no_shift,
		rest
	} next_state;

	class Truck* cars;
	int day = 0;

	int planned;
	int tmp;

	int x;
	// SUGGEST: rozlisuj dni pre zber pondelok utorok... etc. + ich plany
	// TODO: generuj pocty domov na pozbieranie pre auta na zaciatku smeny
	void Behavior() {

		switch (next_state) {
		default:
		case new_window:

			// Set new week
			uncleaned(households.Capacity()-(households.Used()));
			households.Leave(households.Used());

		case work_shift:

			planned = 0;
			for (x = 0; x < truck_n -1 ; x++) {
				// Modulate dispatch time
				// Pick a track
				tmp = Normal(518,180);
				// Trim value
				tmp = tmp < 0 ? 0 : tmp > 1200 ? 1200 : tmp;

				// Handle if too much planned small amount of cars
				if (planned + tmp > houses_n) {
					break;
				}
					planned += tmp;
				cars[x].setPlan(tmp);
				cars[x].Activate(Time + Exponential(move_depo));
			}
			// Plan rest to one truck
			tmp = houses_n - planned;
			cars[x].setPlan(tmp);
			cars[x].Activate(Time + Exponential(move_depo));


			next_state = recall;
			work = true;
			day_start = Time;
			// Call in trucks an hour before end of shift
			Activate(Time + SHIFT);
			break;

		case no_shift:

			next_state = rest_no_shift;
			Activate(Time + 8 * 3600);
			break;

		case recall:

			next_state = rest;
			Activate(Time + RECALL);

		case rest:
		case rest_no_shift:
			day++;
			next_state = work_shift;

			if (day >= 5 && day < 7) {
				next_state = no_shift;

			} else if (day == 7) {
				day = 0;
				next_state = new_window;
			}

			work = false;
			Activate(Time + 16 * 3600);
		}
	}
public:
	int Work()  { return next_state == rest; }
	Dispatcher(int num = truck_n, Priority_t pri = 0) : Event(pri)
	{
		cars = new Truck[num];
		next_state = work_shift;
	}
	~Dispatcher() {
		delete[] cars;
	}
};

void clean_stats()
{
	fieldHours.Clear();
	gathH.Clear();
	uncleaned.Clear();
	unloads.Clear();

	landfill.Clear();
	households.Clear();

	gathH.Init(0, 100, 25); //Garbage gathered per day
	fieldHours.Init(0, 0.5, 20); //Find out utilisation of trucks
	unloads.Init(0, (TRUCK_CAP*1.2)/20 , 20);

	for (auto &x : section) {
		delete x;
	}
	section.resize(houses_n);
	for (auto &x : section) {
		x = new ProducerUnit();
	}

	return;
}

// Popis experimentu s modelem
int main(int argc, char* argv[]) {
	//DebugON();

	if (initParams(argc, argv)) return EXIT_FAILURE;
	
	landfill.SetCapacity(3);
	households.SetCapacity(houses_n);

	class Dispatcher* depo;

	Print("Brno-mesto -- SHO zvoz odpadu SIMLIB/C++\n");
	SetOutput("zvoz.out");

	// inicializace experimentu, čas bude 0..2 tyzdne...
	int x = 1;

	Print("Bisect trucks from %d to %d\n", truck_min, truck_max);
	Print("'Run num'\t'trucks'\t'uncleaned houses'\n");

	// Since i dont know of a way to simulate other parameters than time
	// I run multiple simulation to optimize parametes
	do {
		Init(0, days * 24 * 3600);
		clean_stats();

		//Init if not set
		if (truck_n == 0 && optimize)
			truck_n = (truck_min + truck_max) >> 1;
		else if (truck_n == 0) {
			truck_n = 27;
		}

		// Release the hounds !
		(depo = new Dispatcher(truck_n))->Activate();
		// Who let the dogs out ?
		// Optimize some function of efficiency
		Run();


		delete depo;

		Print("%d\t%d\t%f\n", x++, truck_n, uncleaned.MeanValue());

		if (uncleaned.Max() > 100) {
			truck_min = truck_n;
		} else {
			truck_max = truck_n;
		}
		truck_n = (truck_min + truck_max) >> 1;

	} while (truck_max-truck_min > 1 && optimize);

	// Vypis statistik

	fieldHours.Output();
	gathH.Output();
	uncleaned.Output();
	unloads.Output();
	return 0;
}

