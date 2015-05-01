#include "Path Finding.h"
#include <iostream>
#include <deque>
#include <string.h>
using namespace std;

bool isFound = false;
const int SIZE = 10;
const int MAXCOORDS = 4;


struct num
{
	char Array[SIZE];
};

Coord pathFinder(float &startX, float &startY, float &endX, float &endY);

void SetStartEnd( Coord* begin, Coord* finish);

void CalculateScore(Coord* tmp, int heuristic, num cost[], Coord* current);

void Search(Coord* current, int heuristic, num cost[], deque <Coord*> closedList, deque <Coord*> openList, Coord* tmp);

int Comparison(Coord* List[], int right, int left);

void Sorter(Coord* List[], int right, int left);

Coord* NewNorth(Coord* current, int& heuristic, num cost[], deque <Coord*> closedList, deque <Coord*> openList);

Coord* NewEast(Coord* current, int& heuristic, num cost[], deque <Coord*> closedList, deque <Coord*> openList);

Coord* NewSouth(Coord* current, int& heuristic, num cost[], deque <Coord*> closedList, deque <Coord*> openList);

Coord* NewWest(Coord* current, int& heuristic, num cost[], deque <Coord*> closedList, deque <Coord*> openList);

//the main function in the program to allow for the stl to dealocate memory before the memory leak detection
Coord pathFinder(float &startX, float &startY, float &endX, float &endY)
{
	int heuristic = 0;
	const int Size = 50;
	bool isOpenEmpty = false;
	num cost[SIZE];
	bool NA[MAXCOORDS] = {false,false,false,false};
	Coord* start = new (Coord);
	Coord* end = new (Coord);
	Coord* current = new (Coord);
	Coord* tmp = new (Coord);
	start->x = startX;
	start->y = startY;
	end->x = endX;
	end->y = endY;
	deque <Coord*> openList;
	deque <Coord*> closedList;
	start->score = 0;
	start->parent = 0;
	CalculateScore(start, heuristic,  cost, current);
	openList.push_back(start);

	if(openList.empty())
	{
		cout << "Failure" << endl;
		return *current;
	}
	else
	{
		//main loop for the algorithm
		while(!openList.empty())
		{
			current = openList.front();
			openList.pop_front();

			if(current->y != end->y)
			{
				if(current->y < end->y)
				{
					tmp = NewNorth(current, heuristic, cost, closedList, openList);
					if(isFound)
					{
						NA[0] = true;
					}
					else
					{
						if(tmp->y > 50 || tmp->score == NULL)
						{
							NA[0] = true;
						}
						else
						{
							openList.push_back(tmp);
							NA[0] = false;
						}
					}
				}
				else
				{
					tmp = NewSouth(current, heuristic, cost, closedList, openList);
					if(isFound)
					{
						NA[2] = true;
					}
					else
					{
						if(tmp->y < 0 || tmp->score == NULL)
						{
							NA[2] = true;
						}
						else
						{
							openList.push_back(tmp);
							NA[2] = false;
						}
					}
				}
			}
			if ( current->x != end->x)
			{
				if( current->x < end->x)
				{
					tmp = NewEast(current, heuristic, cost, closedList, openList);
					if(isFound)
					{
						NA[1] = true;
					}
					else
					{
						if( tmp->x > 50 || tmp->score == NULL)
						{
							NA[1] = true;
						}
						else
						{
							openList.push_back(tmp);
							NA[1] = false;
						}
					}
				}
				else
				{
					tmp = NewWest(current, heuristic, cost, closedList, openList);
					if(isFound )
					{
						NA[3] = true;
					}
					else
					{
						if(tmp->x < 0 || tmp->score == NULL)
						{
							NA[3] = true;
						}
						else
						{
							openList.push_back(tmp);
							NA[3] = false;
						}
					}
				}
			}
			if(	NA[0] == true && NA[1] == true && NA[2] == true && NA[3] == true)
			{
				current = current->parent;
				heuristic--;
			}
			int right = openList.size() -1;
			int left = 0;
			Coord* List[Size];
			for(int i = 0; i < Size; i++)
			{
				List[i] = NULL;
			}
			for(int i = 0; i < openList.size(); i++)
			{
				List[i] = openList[i];
			}
			Sorter(List, right, left);
			for(int i = 0; i < Size; i++)
			{
				if(List[i] == NULL)
				{
					
				}
				else
				{
					openList[i] = List[i];
				}
			}
			if(current->x == end->x && current->y == end->y)
			{
				return *current;
			}
			closedList.push_back(current);
		}
	if(openList.empty())
	{
		cout << "Failure" << endl;
		return *current;
	}
	}
	delete(current, end, start);
	
}
//this function calculates the score and sets the parent to current
//each time a new node is generated
void CalculateScore(Coord* tmp, int heuristic, num cost[], Coord* current)
{
	tmp->score = heuristic + current->score;
}
//generates the new north node
Coord* NewNorth(Coord* current, int& heuristic, num cost[], deque <Coord*> closedList, deque <Coord*> openList)
{
	heuristic++;
	Coord* tmp = new Coord;
	tmp->x = current->x;
	tmp->y = current->y +5;
	tmp->parent = current;
	CalculateScore(tmp, heuristic, cost, current);
	Search(current, heuristic, cost, closedList, openList, tmp);
	return tmp;
}

Coord* NewEast(Coord* current, int& heuristic, num cost[], deque <Coord*> closedList, deque <Coord*> openList)
{
	Coord* tmp = new Coord;
	tmp->x = current->x +5;
	tmp->y = current->y;
	tmp->parent = current;
	CalculateScore(tmp, heuristic, cost, current);
	Search(current, heuristic, cost, closedList, openList, tmp);
	return tmp;
}

Coord* NewSouth(Coord* current, int& heuristic, num cost[], deque <Coord*> closedList, deque <Coord*> openList)
{
	Coord* tmp = new Coord;
	tmp->x = current->x;
	tmp->y = current->y -5;
	tmp->parent = current;
	CalculateScore(tmp, heuristic, cost, current);
	Search(current, heuristic, cost, closedList, openList, tmp);
	return tmp;
}

Coord* NewWest(Coord* current, int& heuristic, num cost[], deque <Coord*> closedList, deque <Coord*> openList)
{
	Coord* tmp = new Coord;
	tmp->x = current->x -5;
	tmp->y = current->y;
	tmp->parent = current;
	CalculateScore(tmp, heuristic, cost, current);
	Search(current, heuristic, cost, closedList, openList, tmp);
	return tmp;
}

int Comparison(Coord* List[], int right, int left)
{
	int pivotValue = List[right]->score; 
	int stackTop = left; 
	
	for( int i = left; i < right ; i++)
	{
		if ( List[i]->score <= pivotValue )
		{
			swap(List[i], List[stackTop]);
			stackTop++;
		}
	}

	swap( List[stackTop], List[right] );
	return stackTop;
}
//partition function for sorting the openlist
void Sorter(Coord* List[], int right, int left)
{
	if ( right > left )
	{
		int pivot = Comparison(List, right, left);

		Sorter(List, pivot - 1,left );
		Sorter(List, right,pivot + 1 );
	}
}

// for searching the open and closed list
void Search(Coord* current, int heuristic, num cost[], deque <Coord*> closedList, deque <Coord*> openList, Coord* tmp)
{
	isFound = false;
	deque <Coord*>::iterator it;
	if(!closedList.empty())
	{
		for(it = closedList.begin(); it != closedList.end(); it++)
		{
			if((*it)->x == tmp->x && (*it)->y == tmp->y)
			{
				isFound = true;
				return;
			}
		}
		for(it = openList.begin(); it != openList.end(); it++)
		{
			if((*it)->x == tmp->x && (*it)->y == tmp->y)
			{
				isFound = true;
				return;
			}
		}
	}
}