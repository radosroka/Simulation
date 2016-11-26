///////////////////////////////////////////////////////////////////////////
//
// Created by istoffa on 11/23/16.
//
//               SIMLIB/C++
//

#include <vector>
#include <string>
#include <set>
#include <simlib.h>

using namespace std;

// Garbage production
const int HOUSE_PROD = 70; //Litrov odpadu za tyzden na dom
const int FACT_PROD = 1000; 

// Truck default settings
const int TRUCK_CAP = 1000;
const int UNLOAD = 600;
const int SERVICE = 30;

// Distance delays in seconds
const int MOVE_COLLECT = 60;
const int MOVE_DEPO = 60;
const int MOVE_STOR = 60;

// Network size
const int HOUSES = 10000;
const int FACTORIES = 10;

// Number of trucks available
int TRUCK_N = 3;

int work;
class Store Landfill;

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

	void Behavior() {

		while(1) {
			switch (state) {

			case collecting:

				if (work) {
					// If it is daytime
					if (remaining > 0) {
						// And houses not cleared
						Wait(SERVICE);
						garbage += Exponential(HOUSE_PROD);
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
				Passivate();
				break;
			}
		}

	};

public:
	int garbage;

	void SetWeekly(int houses)
	{
		remaining = houses;
		state = collecting;
	}

	Truck(int cap = TRUCK_CAP)
	{
		capacity = cap;
		state = collecting;
	}

};

class Dispatcher : public Event {
	enum {
		new_window = 0,
		work_shift,
		rest
	} next_state = new_window;

	class Truck* cars;
	int day = 0;

	void Behavior() {

		switch (next_state) {

		case new_window:
				for (int x = 0; x < TRUCK_N; x++)
					// Set quantities and prepare trucks
					cars[x].SetWeekly(HOUSES/x);

		case work_shift:

				for (int x = 0; x < TRUCK_N; x++)
					// Modulate dispatch time
					cars[x].Activate(Time + Exponential(MOVE_DEPO));

				next_state = rest;
				work = Work();
				Activate(Time + 8 * 3600);

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
	Dispatcher(int num = TRUCK_N, Priority_t pri = 0) : Event(pri)
	{
		cars = new Truck[num];
	}
	~Dispatcher() {
		delete[] cars;
	}
};

class Dispatcher Depo;

//TODO: Add statistics
// popis experimentu s modelem
int main() {
	// DebugON();

	Landfill.SetName("Skladka");
	Landfill.SetCapacity(3);

	// Print("model2-timeout -- příklad SIMLIB/C++\n");
	SetOutput("zvoz.out");

	// inicializace experimentu, čas bude 0..2 tyzdne
	Init(0,14*24*3600);

	// Release the hounds !
	Depo.Activate();
	// Who let the dogs out ?
	Run();

	// Vypis statistiky
	return 0;
}

