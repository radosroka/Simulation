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

const int TIMEOUT = 20;
const int SERVICE = 10;

const unsigned GARBAGE_HOUSE = 3; //Litrov odpadu na osobu za den
const unsigned GARBAGE_FACTORY = 1000;
const unsigned TRUCK_MAXLOAD = 1000;


class TruckFailure : public Event {};
class GarbageGen : public Event {};

class Household : public Facility {
	int capacity;
	unsigned prodPerDay;
public:
	Household(unsigned cap, unsigned prod = GARBAGE_HOUSE) : Facility()	{
		//activate garbage generator
		prodPerDay = prod;
		capacity = cap;
	}
	int garbage;
};

class Truck : public Process {
	set<Household*> plan;
	int capacity;
	unsigned loadtime;
	void Behavior()
	{
		for (auto x = plan.begin(); x != plan.end(); x++) {
			if (! (*x)->Busy()) {
				Seize(**x);
				int load = capacity - garbage;
				load = ((load > (*x)->garbage) ? (*x)->garbage : load);
				Wait(loadtime);
				capacity += load;
				(*x)->garbage -= load;

			} else ;

			Wait(Exponential(60));
		}
	};
public:
	Truck(unsigned cap = TRUCK_MAXLOAD)
	{
		//activate failure generator
		capacity = cap;
	}
	int garbage;
};

int main() {                 // popis experimentu s modelem
//DebugON();
	//Print("model2-timeout -- příklad SIMLIB/C++\n");
	//SetOutput("model2-timeout.out");
	Init(0,1000);              // inicializace experimentu, čas bude 0..1000
	// Aktivuj vozy
	Run();                     // simulace

	// Vypis statistiky
	return 0;
}

