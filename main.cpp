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
int houses = 174162; //Spread evenly across the city
int factories = 10; //Spread evenly across the city

// Calculated from inputs
// Garbage production
float house_prod; //Kg odpadu za tyzden na dom
float fact_prod;
// Exponential distribution calculated on travel time avg_speed/avg_distance -> avg time
int move_collect; //sec Distance between houses
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

class Store Landfill;
class Store Households;

//TODO: add suitable statistics
Histogram Gathh; //Garbage gathered per day
TStat Gath;      //
Stat FieldHours; //Find out utilisation of trucks
Stat Dist;       //Accumulate distance travelled by trucks

// Class describing act of collecting garbage from houses
// Code mode profi ;)
class Truck : public Process {
	enum {
		depo,
		collecting,
		storage
	} state;

	int capacity;
	int remaining;
	int unattended;
	double fuel;

	void Behavior() {
		//Implemented as state automaton
		while(1) {
			switch (state) {

			case collecting:

				if (work) {
					// If it is daytime
					if (remaining > 0) {
						//TODO: prerobit na Store, na jedno zastavenie mozno obsluzit viac domov 1,2-5 ?
						//TODO: prirobit fabriky
						// And houses not cleared, compensate if ungathered houses
						Wait(SERVICE);
						garbage += Exponential(
									(!unattended ? house_prod : house_prod*2, unattended--));
						remaining--;

						if (garbage > capacity)	{
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

				if (!remaining) {
					// Go wait for next collecting window;
					state = depo;
					break;
				}

				// Wait for another work shift in depo
				Wait(Exponential(move_depo));
				FieldHours((Time - day_start)/3600);
				Passivate();
				break;

			case storage:

				Wait(Exponential(move_stor));

				Enter(Landfill, 1);
				Wait(UNLOAD);
				garbage = 0;
				Leave(Landfill, 1);

				state = collecting;
				break;

			case depo:

				Wait(Exponential(move_depo));
				// Restart state;
				//state = collecting;
				FieldHours((Time-day_start)/3600);
				Passivate();
				break;
			}
		}

	};

public:
	int garbage;
	int odometer;

	int GetRemaining() { return remaining; }

	void SetWeekly(int houses)
	{
		// Remember uncollected houses
		unattended = remaining;
		remaining = houses;
		state = collecting;
	}

	Truck(int cap = TRUCK_CAP)
	{
		capacity = cap;
		unattended = 0;
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
	int cleaned = 0;
	int perDay = 0;

	void Behavior() {
		//TODO: Prirob fabriky

		switch (next_state) {
		default:
		case new_window:

			for (int x = 0; x < truck_n; x++)
				// Set quantities and prepare trucks
				cars[x].SetWeekly(houses/truck_n);


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

			for (int x = 0; x < truck_n; x++)
				perDay = + cars[x].GetRemaining();

			perDay = (houses - cleaned) - perDay;
			//Log houses served per Day per Truck
			Gath(perDay / truck_n);

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
			{"houses", required_argument, 0, 'u'},
			{"factories", required_argument, 0, 't'},
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

	int ds_houses = sqrt((plane_size*1000000)/houses); // metre Vzdialenost medzi domami
	int ppl_per_house = popluation/houses;

// Garbage production
	house_prod = (ppl_per_house*garbage_k*7)/(365.25); // Kg odpadu za tyzden na dom - priemer
	fact_prod = industry_k/(365.25*factories); // Kg odpadu za den na fabriku

// Distance delays in seconds
	//TODO: Over statisticke vzidalenosti, predpokldame stvorcovu siet, mozme si to vobec dovolit ?
	move_collect = TRUCK_AVG_SPEED/ds_houses; // sec plati priemer pri optimalnom presune !

	// sec Priemerna vzdialenost k domom pokial je v strede mesta sqrt(((a/2)^2*2^0.5)/pi) = r, plocha ohranicena stvocom za kruznicou a pred kruznicou s polomerom r je rovnaka //for 20 should give result 9.488
	move_depo = TRUCK_MAX_SPEED/(sqrt(((plane_size / 4) * sqrt(2)) / 3.1428)); //depo is in the middle of square
	move_stor = TRUCK_MAX_SPEED/20000; // sec ?? nejake dalsia priemerna vzidalenost podobnym sposobom
}

// Popis experimentu s modelem
int main(int argc, char* argv[]) {
	// DebugON();

	Gath.Clear();
	FieldHours.Clear();
	Gathh.Clear();
	FieldHours.SetName("Pracovnych hodin aut");
	Gath.SetName("Pocet upratanych domov za den na auto");

	Landfill.SetName("Skladka");
	Landfill.SetCapacity(3);
	Households.SetName("Domacnosti");
	Households.SetCapacity(houses);


	Print("Brno-mesto -- SHO zvoz odpadu SIMLIB/C++\n");
	SetOutput("zvoz.out");

	// inicializace experimentu, čas bude 0..2 tyzdne...
	Init(0,70*24*3600); //10 tyzdnov

	//initParams(argc, argv);
	// Release the hounds !
	Depo.Activate();
	// Who let the dogs out ?
	// Optimize some function of efficiency
	Run();

	// Vypis statistiky
	FieldHours.Output();
	Gath.Output();
	return 0;
}

