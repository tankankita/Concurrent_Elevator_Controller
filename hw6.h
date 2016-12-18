/* Change these experiment settings to try different scenarios. Parameters can 
	 also be passed in using gcc flags, e.g. -DELEVATORS=5 */

#ifndef MAX_CAPACITY
#define MAX_CAPACITY 1
#endif

#ifndef ELEVATORS
#define ELEVATORS 4
#endif 

#ifndef FLOORS
#define FLOORS 28
#endif 

#ifndef PASSENGERS
#define PASSENGERS 50
#endif 

#ifndef TRIPS_PER_PASSENGER
#define TRIPS_PER_PASSENGER 1
#endif

// these settings affect only the 'looks', will be tested at log level 1
#ifndef DELAY
#define DELAY 10000
#endif 

#ifndef LOG_LEVEL
#define LOG_LEVEL 9
#endif
#define log(level,format,...) do{ if(level<=LOG_LEVEL) fprintf(stderr,format,__VA_ARGS__); }while(0);

/* called once on initialization */
void scheduler_init();

/* called whenever a passenger pushes a button in the elevator lobby. 
	 call enter / exit to move passengers into / out of elevator
	 return only when the passenger is delivered to requested floor
*/
void passenger_request(int passenger, int from_floor, int to_floor, void (*enter)(int, int), void(*exit)(int, int));

/* called whenever the doors are about to close. 
	 call move_direction with direction -1 to descend, 1 to ascend.
	 must call door_open before letting passengers in, and door_close before moving the elevator 
*/
void elevator_ready(int elevator, int at_floor, void(*move_direction)(int, int), void(*door_open)(int), void(*door_close)(int));

