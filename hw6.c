//Ankita Tank
//CS361
//Concurrent Elevator Problem
//sources/citation ---> 
//https://github.com/ecurri3/CS361HW5/blob/master/hw5.c (this is just a resource I DID NOT COPY, this git solution doesnot work! Cross check if you want)
//Computer Systems- A programmer's Persfective
//This is the logic I refered to http://laser.cs.umass.edu/verification-examples/elevator_con/elevator_con_1.html
//https://www.cs.rochester.edu/~kshen/csc256-spring2007/assignments/threadtest.cc
//reference the fucntion passenger_request -> LABWORKSHEET 11 PAGE NUMBER -> 5 &&  LABWORKSHEET 12 PAGE NUMBER -> 2,3  && LABWORKSHEET 12 PAGE NUMBER -> 1,2,3
//reference the fucntion elevator_ready -> LABWORKSHEET 11 PAGE NUMBER -> 6   &&  LABWORKSHEET 12 PAGE NUMBER -> 2,3  && LABWORKSHEET 12 PAGE NUMBER -> 1,2,3
//reference the struct elevator (E in my case)  -> LABWORKSHEET 11 PAGE NUMBER -> 3
//reference the struct Passenger ( P in my case)  -> LABWORKSHEET 11 PAGE NUMBER -> 3

#include "hw6.h"
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

pthread_mutex_t lock_Passenger;
pthread_mutex_t lock;
pthread_barrier_t Elevator_Barrier;
int Passenger_count;
int i;

//Elevator Struct
static struct E
{
	enum
	{
		ELEVATOR_ARRIVED = 1, ELEVATOR_OPEN = 2, ELEVATOR_CLOSED = 3
	} state;
	pthread_barrier_t Elevator_Barrier;
	int Next_Pickup;   //next person to be pickedup from the elevator
	int current_floor; // current floor of the elevator
	int Space;		   // Is there space for one person
	int elev;		  // elevator check
	int direction;	 // which way is the elevator going? up==1 down ==-1
	int occupancy;	// same as space just one person already included
	int Match;    // location of elevator and location of passernger should be =
	pthread_mutex_t lock;


}
Elevator_Position[ELEVATORS];


//Passenger Struct
static struct P
{
	int Pickup;    //passenger pickup status
	int Elev_for_passenger;  //is elevavtor ready for passenger ? 1 for yes 0 for no
	int accept;		// will the passenger be allowed inteh elevator ? 
	int Dropoff;	// is the passenger dropedoff ? 1 for yes -1 for no
	int Counter;   // 
	int Match;    // location of elevator and location of passernger should be =
	pthread_mutex_t lock_Next_Passenger;
	pthread_cond_t Cond_t;
}
Travelling_Passenger[PASSENGERS];



void scheduler_init()
{
	//initialzing elevator struct  --> refer to main.c for the strcuct [ELEVATOR]
	for (i = 0; i < ELEVATORS; i++)
	{
		pthread_mutex_init(&Elevator_Position[i].lock, 0);
		pthread_barrier_init(&Elevator_Position[i].Elevator_Barrier, NULL, 2);
		Elevator_Position[i].direction = -1;
		Elevator_Position[i].Next_Pickup = -1;
		Elevator_Position[i].occupancy = 0;
		Elevator_Position[i].current_floor = 0;
		Elevator_Position[i].state = ELEVATOR_ARRIVED;
		Elevator_Position[i].Space = 0;
		Elevator_Position[i].elev = 0;
	}

	for (int i = 0; i < PASSENGERS; i++)
	{
		pthread_cond_init(&Travelling_Passenger[i].Cond_t, 0);						//initialse all the cond_t for the thread
		pthread_mutex_init(&Travelling_Passenger[i].lock_Next_Passenger, 0);     //lock all the next passengers 
	}

	//Lock the current 1 passenger
	pthread_mutex_init(&lock_Passenger, 0);

}

//reference the fucntion passenger_request -> LABWORKSHEET 11 PAGE NUMBER -> 5
void passenger_request(int passenger, int Pickup, int Dropoff, void(*enter)(int, int), void(*exit)(int, int))
{

	//ready to accept the passenger in the elevator
	Travelling_Passenger[passenger].accept = 1;

	pthread_mutex_lock(&lock_Passenger);

	pthread_mutex_lock(&Travelling_Passenger[passenger].lock_Next_Passenger);

	if (Passenger_count <= passenger)
		Passenger_count = passenger;

	//initialize values in passenger struct
	Travelling_Passenger[passenger].Pickup = Pickup;
	Travelling_Passenger[passenger].Dropoff = Dropoff;
	pthread_mutex_unlock(&lock_Passenger);
	pthread_cond_wait(&Travelling_Passenger[passenger].Cond_t, &Travelling_Passenger[passenger].lock_Next_Passenger); //initialize waiting condition for the passenger until an elevator has arrived for pickup
	pthread_mutex_unlock(&Travelling_Passenger[passenger].lock_Next_Passenger);

	int Elev_for_passenger = Travelling_Passenger[passenger].Elev_for_passenger;   // Have an elevator for every passenger thread.

	int waiting = 1;   // initialise it to waiting ahead of time
	int riding = 1;   // initialise it to riding ahead of time


	while (waiting)    // while the passenger is waiting..
	{

		pthread_barrier_wait(&Elevator_Position[Elev_for_passenger].Elevator_Barrier);  // wait until there is an empty elevator for the passenger
		pthread_mutex_lock(&Elevator_Position[Elev_for_passenger].lock);				  //lock the elevator until passenger found

		//if the elevator is in the current floor as the passenger  
		//&&  if the elevator door opens for the passenger for pickup 
		//&& if elevator is empty
		//----> then allow the passenger inside of the elevator
		//----> increment the number of passenger in the elevator i.e occupancy++  ---> to keep a track on the people in the elevator
		//----> keep a track of the drop off location (not sure if I implemented it right) 
		//---> making the waiting = 0 again as the passenger is dropped off
		if (Elevator_Position[Elev_for_passenger].current_floor == Pickup && Elevator_Position[Elev_for_passenger].state == ELEVATOR_OPEN && Elevator_Position[Elev_for_passenger].occupancy == 0)
		{
			enter(passenger, Elev_for_passenger);							//----> then allow the passenger inside of the elevator
			Elevator_Position[Elev_for_passenger].occupancy++;			    //----> increment the number of passenger in the elevator i.e occupancy++  ---> to keep a track on the people in the elevator
			Elevator_Position[Elev_for_passenger].Next_Pickup = Dropoff;      //----> keep a track of the drop off location (not sure if I implemented it right) 
			waiting = 0;													//---> making the waiting = 0 again as the passenger is dropped off
		}
		pthread_mutex_unlock(&Elevator_Position[Elev_for_passenger].lock);  //lock the direction of the elevator for the next passenger to enter
		pthread_barrier_wait(&Elevator_Position[Elev_for_passenger].Elevator_Barrier); // wait for an empty elevator thread for the passenger next to before.
	}







	while (riding)    //While the passenger is in the elevator
	{
		//Doing the same as above for the waiting criteria
		pthread_barrier_wait(&Elevator_Position[Elev_for_passenger].Elevator_Barrier);  // wait until there is an empty elevator for the passenger
		pthread_mutex_lock(&Elevator_Position[Elev_for_passenger].lock);		         //lock the elevator until passenger found

																					 //if (Elevator_Position[Elev_for_passenger].current_floor == Dropoff && Elevator_Position[Elev_for_passenger].state == ELEVATOR_OPEN && Elevator_Position[Elev_for_passenger].occupancy == 1)
																					 //{
																					 //	riding = 0;
																					 //	exit(passenger, Elev_for_passenger);
																					 //	Elevator_Position[Elev_for_passenger].occupancy--;
																					 //	Elevator_Position[Elev_for_passenger].Next_Pickup = -1;
																					 //}
																					 //int curr_floor = Elevator_Position[Elev_for_passenger].current_floor;

																					 //not to accept the passenger in the elevator no space
		Travelling_Passenger[passenger].accept = 0;



		//if the evalator door opens 
		//&& if the passenger gets off from the elevator. 
		//----> pass the passenger to exit the elevator (function in main) 
		//---->reduce the occupancy i.e the capacity as the passenger is dropped off
		//---->Set Next pickup back to -1 so as to make it available for the next passenger.
		//---->make riding ==0 as there is no one in the elevator to be riden off
		if (Elevator_Position[Elev_for_passenger].current_floor == Dropoff && Elevator_Position[Elev_for_passenger].state == ELEVATOR_OPEN)
		{
			exit(passenger, Elev_for_passenger);
			Elevator_Position[Elev_for_passenger].occupancy--;
			Elevator_Position[Elev_for_passenger].Next_Pickup = -1;
			riding = 0;
		}
		pthread_mutex_unlock(&Elevator_Position[Elev_for_passenger].lock);              //lock the direction of the elevator for the next passenger to enter
		pthread_barrier_wait(&Elevator_Position[Elev_for_passenger].Elevator_Barrier);  // wait for an empty elevator thread for the passenger next to before.
	}
}


//reference the fucntion elevator_ready -> LABWORKSHEET 11 PAGE NUMBER -> 6   &&  LABWORKSHEET 12 PAGE NUMBER -> 2,3  && LABWORKSHEET 12 PAGE NUMBER -> 1,2,3
void elevator_ready(int Elev_for_passenger, int at_floor, void(*move_direction)(int, int), void(*door_open)(int), void(*door_close)(int))
{
	//pthread_mutex_lock(&Elevator_Position[Elev_for_passenger].lock);
	int distance_toPickup = 0;
	int Next_Passenger = 0;
	int Elevator_To_passenger_Distance = 0;
	int i = 0;
	pthread_mutex_lock(&Elevator_Position[Elev_for_passenger].lock);		// lock the elevator thread for the passenger at the very beginning 


	//if the elevator door is open
   //----> close the door as the passenger is in there 
	//----> Change the state of the elevator door to 'close'

	if (Elevator_Position[Elev_for_passenger].state == ELEVATOR_OPEN)
	{
		Elevator_Position[Elev_for_passenger].state = ELEVATOR_CLOSED;	//----> Change the state of the elevator door to 'close'
		door_close(Elev_for_passenger);								     //----> close the door as the passenger is in there 
	}

	//if the elevator is arrived at a floor
	//&& the next passnger is in the same floor
	//----> open the door of teh elevator at that floor 
	//----> unlock the locked mutex caused for the passenger
	//----> create a barrier for the elevator twice
	else if (Elevator_Position[Elev_for_passenger].state == ELEVATOR_ARRIVED && Elevator_Position[Elev_for_passenger].Next_Pickup == at_floor)
	{
		Elevator_Position[Elev_for_passenger].state = ELEVATOR_OPEN;						 //----> open the door of teh elevator at that floor 
		door_open(Elev_for_passenger);												     //----> open the door of teh elevator at that floor 
		pthread_mutex_unlock(&Elevator_Position[Elev_for_passenger].lock);			     //----> unlock the locked mutex caused for the passenger
		pthread_barrier_wait(&Elevator_Position[Elev_for_passenger].Elevator_Barrier);  	 //----> create a barrier for the elevator twice
		pthread_barrier_wait(&Elevator_Position[Elev_for_passenger].Elevator_Barrier);
	}

	//there is no one in the evelator currently i.e  the door is not open
	//check for which floor to arrive next
	else if ((Elevator_Position[Elev_for_passenger].state != ELEVATOR_OPEN))
	{
		// if there is no one in the elevator i.e the elevator is empty and waiting the ride 
		if (Elevator_Position[Elev_for_passenger].Next_Pickup <= -1)
		{
			Next_Passenger = -1;								 // for each passenger who pressed the floor button
			Elevator_To_passenger_Distance = FLOORS;              // distance between current floor where the elevator is to the Floor elevator has to be to pick up the passenger 
			pthread_mutex_lock(&lock_Passenger);                 // passenger thread will be locked as the elevator is approaching the passenger 


			for (i = 0; i <= Passenger_count; i++)			// for all the passengers who are for the elevator 
			{

				distance_toPickup = Travelling_Passenger[i].Pickup - at_floor;   //this will be the distance between current floor where the elevator is to the Floor elevator has been picked up the passenger 

																				  //if the distance between the elevator and teh passenger is negative i.e if the elevator has to go up.
																				  //----> make the distance positive so it can be iterated through the floors
				if (distance_toPickup < 0)
					distance_toPickup = distance_toPickup * -1;


				//if  the distance is less than the passenger distance
				//&&  there is no match for the passenger 
				//----> set the  distance for passenger to the distance of elevator floors
				//----> then the passenger at that floor will be tracked.
				if (distance_toPickup < Elevator_To_passenger_Distance && Travelling_Passenger[i].Match == 0)
				{
					Elevator_To_passenger_Distance = distance_toPickup;  		//----> set the  distance for passenger to the distance of elevator floors
					Next_Passenger = i;										   //----> then the passenger at that floor will be tracked.
				}
			}


			//if there is passenger at the floor where elevator is at
			//----> update the match in the elevator struct
			//----> update the elevator for passenger to the elevator number found above
			//----> the next pickup(current) will be the assigned to the elevator for the passenger
			//----> signal the thread for the passenger so it can wait until the passenger thread is unlock so it can move the passenger to the desired floor
			if (Next_Passenger > -1)
			{
				Travelling_Passenger[Next_Passenger].Elev_for_passenger = Elev_for_passenger;							//----> update the elevator for passenger to the elevator number found above
				pthread_cond_signal(&Travelling_Passenger[Next_Passenger].Cond_t);										//----> signal the thread for the passenger so it can wait until the passenger thread is unlock so it can move the passenger to the desired floor
				Travelling_Passenger[Next_Passenger].Match = 1;														//----> update the match in the elevator struct
				Elevator_Position[Elev_for_passenger].Next_Pickup = Travelling_Passenger[Next_Passenger].Pickup;			//----> the next pickup(current) will be the assigned to the elevator for the passenger
			}

			//unlock the passenger thread to allow the elevator to move for the passenger
			pthread_mutex_unlock(&lock_Passenger);
		}



		//if the elevator is vacant for the next pick up
		//----> check the floor
		//----> move the elevator according to the request of next pickup
		//----> currentfloor = at_what_floor_elevator_was + incremented direction from the above statement
		if (Elevator_Position[Elev_for_passenger].Next_Pickup >= 0)
		{



			//BASE CASES FOR ELEVATOR LEVEL CHECK
			//if elevator the very bottom of the building
			//----> manupulate the direction
			if (at_floor == 0)
				Elevator_Position[Elev_for_passenger].direction = Elevator_Position[Elev_for_passenger].direction * -1;   	//----> manupulate the direction




			//if elevator the very Top of the building
			//----> manupulate the direction
			if (at_floor == FLOORS - 1)
				Elevator_Position[Elev_for_passenger].direction = Elevator_Position[Elev_for_passenger].direction * -1;   	//----> manupulate the direction



				//if we are moving up with an empty elevator
				//|| the pickup is in the range of all the possible floor we can go to
				//&& if the elevator has to move up i.e if our next pick up is on the floor above us 
				//----> increment the direction of elevator a floor above
				if ((Elevator_Position[Elev_for_passenger].Next_Pickup > -1 || Elevator_Position[Elev_for_passenger].Next_Pickup < FLOORS) && at_floor < Elevator_Position[Elev_for_passenger].Next_Pickup  )
				Elevator_Position[Elev_for_passenger].direction = 1;      	//----> make the direction of elevator a floor above as 1 for making it available




		    //if the elevator has to down up i.e if our next pick up is on the floor below us 
			//&& if we are moving down with an empty elevator
		   //|| the pickup is in the range of all the possible floor we can go to
			//----> Decrement  the direction of elevator a floor above
			if (at_floor > Elevator_Position[Elev_for_passenger].Next_Pickup && (Elevator_Position[Elev_for_passenger].Next_Pickup >= 0 || Elevator_Position[Elev_for_passenger].Next_Pickup <= FLOORS - 1))
				Elevator_Position[Elev_for_passenger].direction = -1;      //----> Decrement  the direction of elevator a floor above



			Elevator_Position[Elev_for_passenger].state = ELEVATOR_ARRIVED;												 // update state of the elevator 
			move_direction(Elev_for_passenger, Elevator_Position[Elev_for_passenger].direction);						     //move elevator acording to given direction and the floor number
			Elevator_Position[Elev_for_passenger].current_floor = at_floor + Elevator_Position[Elev_for_passenger].direction;   // update teh current_floor according to the passenger requests
		}
	}

	pthread_mutex_unlock(&Elevator_Position[Elev_for_passenger].lock);  // make elevators available for passengers for other trip
}



//resources ---> 
//https://github.com/ecurri3/CS361HW5/blob/master/hw5.c (this is just a resource I DID NOT COPY, this git solution doesnot work! Cross check if you want)
//Computer Systems- A programmer's Persfective
//This is the logic I refered to http://laser.cs.umass.edu/verification-examples/elevator_con/elevator_con_1.html
//https://www.cs.rochester.edu/~kshen/csc256-spring2007/assignments/threadtest.cc
//reference the fucntion passenger_request -> LABWORKSHEET 11 PAGE NUMBER -> 5 &&  LABWORKSHEET 12 PAGE NUMBER -> 2,3  && LABWORKSHEET 12 PAGE NUMBER -> 1,2,3
//reference the fucntion elevator_ready -> LABWORKSHEET 11 PAGE NUMBER -> 6   &&  LABWORKSHEET 12 PAGE NUMBER -> 2,3  && LABWORKSHEET 12 PAGE NUMBER -> 1,2,3



