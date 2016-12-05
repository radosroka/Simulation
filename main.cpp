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
#include <getopt.h>
#include <unistd.h>
#include <cstdlib>

#include <vector>
#include <map>

using namespace std;

// TODO: rado... srsly ?
const string HELP_MESSAGE = "help, no_argument, 0, h\n"
			"output-file, required_argument, 0, f\n"
			"city-size, required_argument, 0, c\n"
			"person-production, required_argument, 0, p\n"
			"industry-production, required_argument, 0, i\n"
			"houses_n, required_argument, 0, u\n"
			"factories_n, required_argument, 0, t\n"
			"population, required_argument, 0, l\n";

// Simulation inputs //defaults
double plane_size = 230.22; //km squared
int population = 377028; //people
double garbage_k = 280; //Kg garbage per people/year
double industry_k = 3635845000; //Kg per year total
int houses_n = 174162; //Spread evenly across the city
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
const int SERVICE = 30; // Time to load a trash can
const int TRUCK_AVG_SPEED = 10; //Meters/sec
const int TRUCK_MAX_SPEED = 90/3.6; //Meters/sec

// Constants form statistics

// Variables
int truck_n = 17;

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
Stat unloads("Vahy vysypok na skladkach");
// Class describing act of collecting garbage from houses_n
// Code mode profi ;)

class ProducerUnit : public Facility {
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
		storage
	} state;

	int remaining;
	int plan_houses;
	int remain_days;
	int plan_days;

	int capacity;
	int per_day;

	int per_stop;
	int x;

	void Behavior() {
		//Implemented as state automaton
		while(1) {
			switch (state) {

			case collecting:

				if (work) {
					// If it is daytime
					if (!households.Full() /*&& remaining > 0*/) {
						//TODO: na jedno zastavenie mozno obsluzit viac domov 1,2-5 ?
						//TODO: prirobit fabriky
						//TODO: prechod na ProducerUnit
						// And houses_n not cleared, compensate if ungathered houses_n

						per_stop = 1 + Exponential(6);
						for (x = 0; x < per_stop ; x++) {
							if (households.Full() /*or remaining < x*/) {
								break;
							}
							Enter(households, 1);
						}

						serviceWait(x);

						for (int z = 0; z <= x ; z++) {
							auto H = section.at(households.Used()-1-z);
							float hoarding;
							Seize(*H);
							garbage += H->gather(this, hoarding);
							overprod += hoarding;
							Release(*H);
						}

						/*if (missedHH.Capacity() > 0 ) {
							// Collect extra from last week
							Enter(missedHH, 1);
							garbage += Exponential(house_prod);
						}*/

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

	void setPlan(int houses, int interval)
	{
		plan_days = interval;
		plan_houses = houses;
	}

	// TODO: Vyber spravne rozlozenia, mozno bude nutny experiment
	void moveDepoWait() { Wait(move_depo + Exponential(move_depo/3)); }

	void moveLandfillWait() { Wait(move_stor + Exponential(move_stor/3)); }

	void moveCollectWait() { Wait(move_collect+Exponential(move_collect)); }

	void serviceWait(int x) { Wait(SERVICE + x*SERVICE*Random() ); }

	void unloadWait() { Wait(Exponential(UNLOAD)); }

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

	// TODO: rozlisuj dni pre zber pondelok utorok... etc. + ich plany
	void Behavior() {

		switch (next_state) {
		default:
		case new_window:

			// Set new week
			uncleaned(households.Capacity()-(households.Used()));
			households.Leave(households.Used());

		case work_shift:

			for (int x = 0; x < truck_n; x++)
				// Modulate dispatch time
				cars[x].Activate(Time + Exponential(move_depo));

			next_state = recall;
			work = true;
			day_start = Time;
			Activate(Time + 7 * 3600);
			break;

		case no_shift:

			next_state = rest_no_shift;
			Activate(Time + 8 * 3600);
			break;

		case recall:

			next_state = rest;
			Activate(Time + 3600);

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



// Spocitaj simulacne parametre
// TODO: Rado, prida ako parameter poced dni simulacie
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
			{"houses_n", required_argument, 0, 'u'},
			{"factories_n", required_argument, 0, 't'},
			{"population", required_argument, 0, 'l'},
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

	int min = 1;
	int max = 50;
	class Dispatcher* depo;

	Print("Brno-mesto -- SHO zvoz odpadu SIMLIB/C++\n");
	SetOutput("zvoz.out");

	// inicializace experimentu, čas bude 0..2 tyzdne...
	int x = 1;

	Print("Bisect trucks from %d to %d\n", min, max);
	Print("'Run num'\t'trucks'\t'uncleaned houses'\n");

	do {
		Init(0, 21 * 24 * 3600);
		clean_stats();

		truck_n = (min+max) >> 1;
		// Release the hounds !
		(depo = new Dispatcher(truck_n))->Activate();
		// Who let the dogs out ?
		// Optimize some function of efficiency
		Run();


		delete depo;

		Print("%d\t%d\t%f\n", x++, truck_n, uncleaned.MeanValue());

		if (uncleaned.Max() > 100) {
			min = truck_n;
		} else {
			max = truck_n;
		}

	} while (max-min > 1);

	// Vypis statistik

	fieldHours.Output();
	gathH.Output();
	uncleaned.Output();
	unloads.Output();
	return 0;
}

