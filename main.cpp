///////////////////////////////////////////////////////////////////////////
//
// Created by istoffa on 11/23/16.
//
//               SIMLIB/C++
//

#include <string>
#include <simlib.h>
#include <cmath>
#include <getopt.h>
#include <unistd.h>

using namespace std;

// Simulation inputs //defaults
double plane_size = 230.22; //km squared
int popluation = 377028; //people
double garbage_k = 280; //Kg garbage per people/year
double industry_k = 3635845000; //Kg per year total
int houses_n = 174162; //Spread evenly across the city
int factories_n = 10; //Spread evenly across the city

// Calculated from inputs
// Garbage production
float house_prod; //Kg odpadu za tyzden na dom
float fact_prod;
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
int truck_n = 4;

// Optimize function
// TODO: Check how simulating works

// Flag for work shift
int work;
double day_start;

class Store landfill;
class Store households;
class Store missedHH;
class Store factories;

//TODO: add suitable statistics
Histogram gathH; //Garbage gathered per day
TStat gath;      //
Stat fieldHours; //Find out utilisation of trucks
Stat dist;       //Accumulate distance travelled by trucks

// Class describing act of collecting garbage from houses_n
// Code mode profi ;)

class Truck : public Process {
	enum {
		depo,
		collecting,
		storage
	} state;

	int capacity;
	int per_day;
	double fuel; //Count somehow from travel time

	void Behavior() {
		//Implemented as state automaton
		while(1) {
			switch (state) {

			case collecting:

				if (work) {
					// If it is daytime
					//if (remaining > 0) {
					if (households.Capacity() > 0) {
						//TODO: na jedno zastavenie mozno obsluzit viac domov 1,2-5 ?
						//TODO: prirobit fabriky
						// And houses_n not cleared, compensate if ungathered houses_n
						Enter(households, 1);

						Wait(SERVICE);

						if (missedHH.Capacity() > 0 ) {
							// Collect extra from last week
							Enter(missedHH, 1);
							garbage += Exponential(house_prod);
						}
						garbage += Exponential(house_prod);

						per_day++;

						if (garbage > capacity) {
							// Go to landfill if +- full
							state = storage;
							break;
						}

						Wait(move_collect);
						break;
					}
					// Else dump trash and wait for next cleaning
				}

				if (garbage > 0) {
					// Dump trash to start clean
					state = storage;
					break;
				}

				if (!households.Capacity()) {
					// Go wait for next collecting window;
					state = depo;
					break;
				}

				// Wait for another work shift in depo
				Wait(Exponential(move_depo));
				fieldHours((Time - day_start)/3600);
				gathH(per_day);
				per_day = 0;
				Passivate();
				break;

			case storage:

				Wait(Exponential(move_stor));

				Enter(landfill, 1);
				Wait(UNLOAD);
				garbage = 0;
				Leave(landfill, 1);

				state = collecting;
				break;

			case depo:

				Wait(Exponential(move_depo));
				// Restart state;
				fieldHours((Time-day_start)/3600);
				gathH(per_day);
				per_day = 0;
				state = collecting;
				Passivate();
				break;
			}
		}
	};

public:
	int garbage;
	int odometer;

	Truck(int cap = TRUCK_CAP)
	{
		capacity = cap;
		per_day = 0;
		state = collecting;
	}

};

class Dispatcher : public Event {
	enum {
		new_window = 0,
		work_shift,
		rest
	} next_state;

	class Truck* cars;
	int day = 0;

	void Behavior() {
		//TODO: Prirob fabriky

		switch (next_state) {
		default:
		case new_window:

			missedHH.SetCapacity(missedHH.Capacity()+households.Capacity());
			households.SetCapacity(houses_n);

		case work_shift:

			for (int x = 0; x < truck_n; x++)
				// Modulate dispatch time
				cars[x].Activate(Time + Exponential(move_depo));

			next_state = rest;
			work = Work();
			day_start = Time;
			Activate(Time + 8 * 3600);
			break;

		case rest:
			day++;
			next_state = work_shift;

			if (day == 7) {
				day = 0;
				next_state = new_window;
			}
			work = Work();

			Activate(Time + 16 * 3600);
		}
	}
public:
	int Work()  { return next_state == rest; }
	Dispatcher(int num = truck_n, Priority_t pri = 0) : Event(pri)
	{
		cars = new Truck[num];
		next_state = new_window;
	}
	~Dispatcher() {
		delete[] cars;
	}
};

class Dispatcher Depo;


// Spocitaj simulacne parametre
// TODO: Rado, urobi spracovanie parametrov
void initParams(int argc, char* argv[])
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

		c = getopt_long(argc, argv, "ho:c:p:i:u:t:l:", long_options, &option_index);
		if (c == -1) break;
		switch (c) {
		//TODO: fill cases
		default:
			printf("?? getopt returned character code 0%o ??\n", c);
		}
	}

	if (optind < argc) {
		printf("non-option ARGV-elements: ");
		while (optind < argc)
			printf("%s ", argv[optind++]);
		printf("\n");
	}

	int ds_houses = sqrt((plane_size*1000000)/houses_n); // metre Vzdialenost medzi domami
	int ppl_per_house = popluation/houses_n;

// Garbage production
	house_prod = (ppl_per_house*garbage_k*7)/(365.25); // Kg odpadu za tyzden na dom - priemer
	fact_prod = industry_k/(365.25*factories_n); // Kg odpadu za den na fabriku

// Distance delays in seconds
	//TODO: Over statisticke vzidalenosti, predpokldame stvorcovu siet, mozme si to vobec dovolit ?
	//FIXME: Vypocet rozlozenia vzdialenosti
	move_collect = TRUCK_AVG_SPEED/ds_houses; // sec plati priemer pri optimalnom presune !

	// sec Priemerna vzdialenost k domom pokial je v strede mesta sqrt(((a/2)^2*2^0.5)/pi) = r, plocha ohranicena stvocom za kruznicou a pred kruznicou s polomerom r je rovnaka //for 20 should give result 9.488
	move_depo = TRUCK_MAX_SPEED/(sqrt(((plane_size / 4) * sqrt(2)) / 3.1418)); //depo is in the middle of square
	move_stor = TRUCK_MAX_SPEED/20000; // sec ?? nejake dalsia priemerna vzidalenost podobnym sposobom
}

// Popis experimentu s modelem
int main(int argc, char* argv[]) {
	//DebugON();

	gath.Clear();
	fieldHours.Clear();
	gathH.Clear();
	fieldHours.SetName("Pracovnych hodin aut");
	gathH.Init(500, 25, 20);
	gathH.SetName("Pocet upratanych domov za den na auto");

	landfill.SetName("Skladka");
	landfill.SetCapacity(3);
	households.SetName("Domacnosti");
	households.SetCapacity(houses_n);


	Print("Brno-mesto -- SHO zvoz odpadu SIMLIB/C++\n");
	SetOutput("zvoz.out");

	// inicializace experimentu, čas bude 0..2 tyzdne...
	Init(0,3*24*3600); //10 tyzdnov

	//initParams(argc, argv);
	// Release the hounds !
	Depo.Activate();
	// Who let the dogs out ?
	// Optimize some function of efficiency
	Run();

	// Vypis statistiky
	households.Output();
	missedHH.Output();
	landfill.Output();

	fieldHours.Output();
	gathH.Output();
	return 0;
}

