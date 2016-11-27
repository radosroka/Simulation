///////////////////////////////////////////////////////////////////////////
//
// Created by istoffa on 11/23/16.
//
//               SIMLIB/C++
//

#include <string>
#include <simlib.h>

using namespace std;

// Garbage production
const int HOUSE_PROD = 70; //Litrov odpadu za tyzden na dom
const int FACT_PROD = 1000; 

// Truck default settings
const int TRUCK_CAP = 3000;
const int UNLOAD = 600; // Time to unload the car
const int SERVICE = 30; // Time to load a trash can

// Distance delays in seconds
const int MOVE_COLLECT = 60; // Time delay between houses
const int MOVE_DEPO = 120; // Time delay to depo
const int MOVE_STOR = 300; // Time delay to landfill

// Network size
const int HOUSES = 10000;
const int FACTORIES = 10;

// Number of trucks available
int TRUCK_N = 4;

// Flag for work shift
int work;
double day_start;

class Store Landfill;
TStat Gath;
Stat FieldHours;
Stat Dist;

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

	void Behavior() {
		//Implemented as state automaton
		while(1) {
			switch (state) {

			case collecting:

				if (work) {
					// If it is daytime
					if (remaining > 0) {
						//TODO: prerobit na Store
						// And houses not cleared, compensate if ungathered houses
						Wait(SERVICE);
						garbage += Exponential(
									(!unattended ? HOUSE_PROD : HOUSE_PROD*2, unattended--));
						remaining--;

						if (garbage > capacity)	{
							// Go to landfill if +- full
							state = storage;
							break;
						}

						Wait(MOVE_COLLECT);
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
				Wait(Exponential(MOVE_DEPO));
				FieldHours((Time - day_start)/3600);
				Passivate();
				break;

			case storage:

				Wait(Exponential(MOVE_STOR));

				Enter(Landfill, 1);
				Wait(UNLOAD);
				garbage = 0;
				Leave(Landfill, 1);

				state = collecting;
				break;

			case depo:

				Wait(Exponential(MOVE_DEPO));
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

		switch (next_state) {
		default:
		case new_window:

			for (int x = 0; x < TRUCK_N; x++)
				// Set quantities and prepare trucks
				cars[x].SetWeekly(HOUSES/TRUCK_N);


		case work_shift:

			for (int x = 0; x < TRUCK_N; x++)
				// Modulate dispatch time
				cars[x].Activate(Time + Exponential(MOVE_DEPO));

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

			for (int x = 0; x < TRUCK_N; x++)
				perDay = + cars[x].GetRemaining();

			perDay = (HOUSES - cleaned) - perDay;
			//Log houses served per Day per Truck
			Gath(perDay / TRUCK_N);

			Activate(Time + 16 * 3600);
		}
	}
public:
	int Work()  { return next_state == rest; }
	Dispatcher(int num = TRUCK_N, Priority_t pri = 0) : Event(pri)
	{
		cars = new Truck[num];
		next_state = new_window;
	}
	~Dispatcher() {
		delete[] cars;
	}
};

class Dispatcher Depo;

// popis experimentu s modelem
int main() {
	// DebugON();

	Gath.Clear();
	FieldHours.Clear();
	FieldHours.SetName("Pracovnych hodin aut");
	Gath.SetName("Pocet upratanych domov za den na auto");

	Landfill.SetName("Skladka");
	Landfill.SetCapacity(3);

	// Print("model2-timeout -- příklad SIMLIB/C++\n");
	SetOutput("zvoz.out");

	// inicializace experimentu, čas bude 0..2 tyzdne
	Init(0,70*24*3600);

	// Release the hounds !
	Depo.Activate();
	// Who let the dogs out ?
	Run();

	// Vypis statistiky
	FieldHours.Output();
	Gath.Output();
	return 0;
}

