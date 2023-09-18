#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

//Define the number of dimensions and a struct for points
#define dimensions 2

typedef struct Position_t {
	double height;
	double coords[dimensions];	
} Position;

//All the functions used for this exercise. Explanations below
Position findCentroid(Position points[]);
Position reflectExpand(Position x, Position y, double(func)(Position));
Position contractReplace(Position x, Position y, double(func)(Position));
bool hasMinReached(Position points[], double limit);
int compare(const void *a, const void *b);
void calcHeights(Position points[], double (func)(Position));
void hundredValues(double (func)(Position), double start, double end);
void arrayToStruct(double *pointerPoints[], Position points[]);
void loop(double (func)(Position), Position points[]);

//The Rosenbrock function we'll find the minimum of
double Brock(Position p){
	double x0 = p.coords[0];
	double x1 = p.coords[1];
	
	return 100*pow(x1 - x0*x0, 2) + pow(1 - x0, 2);
}

int main(){
	//Create the initial list of points
	double p0[] = {0., 0.}, p1[] = {2., 0.}, p2[] = {0., 2.};
	double *allPoints[] = {p0, p1, p2};
	
	//Print the required hundred values to a file
	hundredValues(Brock, -2., 2.);
	
	//Convert points to the Position type and begin the Simplex algorithm
	Position points[dimensions + 1];
	arrayToStruct(allPoints, points);
	loop(Brock, points);
}	

//The downhill simplex method
void loop(double (func)(Position), Position points[]){
	//Inital variables for p*, p**, and the centroid
	Position pstar, p2star, pbar;
	
	//Counters for the while loop and the for loops
	int loopCounter = 0;
	
	while(loopCounter < 1000){
		
		loopCounter += 1;
		
		//Calculate y-values and sort via quick sort
		calcHeights(points, func);
		qsort(points, dimensions + 1, sizeof(Position), compare);

		//Calculate the centroid of p_l and p_i, then calculate p*
		pbar = findCentroid(points);
		pstar = reflectExpand(pbar, points[0], func);
		
		//Start the flow chart
		if(pstar.height < points[dimensions].height){
			//Calculate p** and LHS of decision tree
			p2star = reflectExpand(pstar, pbar, func);
			points[0] = (p2star.height < points[dimensions].height)? p2star:pstar;
		}else{
			if(pstar.height > points[1].height){
				//If y* > y_i, i != h
				if(pstar.height < points[0].height){
					//If y* < y_h
					points[0] = pstar;
				}
				p2star = contractReplace(points[0], pbar, func);
				if(p2star.height > points[0].height){
					//If y** > y_h, replace all P_i's as in chart
					for(int i = 0; i < dimensions + 1; i++){
						points[i] = contractReplace(points[i], points[dimensions], func);
					}
				}else{
					//If y** < y_h
					points[0] = p2star;
				}
			}else{
				//If y* < y_i
				points[0] = pstar;
			}
		}
		//Check if the minimum threshold has been reached
		if(hasMinReached(points, pow(10, -8))){break;}
	}
	//Print out the final points (highest y-value to lowest) and the number of loops taken
	for(int i = 0; i < dimensions + 1; i++){
		printf("Point %d: (%lf, %lf) with y = %lf\n", i, points[i].coords[0], points[i].coords[1], points[i].height);
	}
	printf("Number of loops: %d\n", loopCounter);
}

//Finds the centroid of all points in some array
Position findCentroid(Position points[]){
	Position centroid;
	
	for(int i = 1; i < dimensions + 1; i++){
		for(int j = 0; j < dimensions; j++){
			centroid.coords[j] += points[i].coords[j]/dimensions;
		}
	}
	
	return centroid;
}

//Reflects or expands as the flowchart dictates
Position reflectExpand(Position x, Position y, double(func)(Position)){
	Position reflectExpanded;

	for(int i = 0; i < dimensions; i++){
		reflectExpanded.coords[i] = 2*x.coords[i] - y.coords[i];
	}
	
	reflectExpanded.height = func(reflectExpanded);
	return reflectExpanded;
}

//Contracts or replaces as the flowchart dictates
Position contractReplace(Position x, Position y, double(func)(Position)){
	Position contractReplaced;

	for(int i = 0; i < dimensions; i++){
		contractReplaced.coords[i] = 0.5*(x.coords[i] + y.coords[i]);
	}

	contractReplaced.height = func(contractReplaced);
	return contractReplaced;
}

//Checks if the standard deviation is below the desired limit
bool hasMinReached(Position points[], double limit){
	double meanValue = 0, sumForMean = 0, sumForRoot = 0, result = 0;
	
	for(int i = 0; i < dimensions + 1; i++){
		sumForMean += points[i].height;
	}
	
	meanValue = sumForMean/(dimensions + 1);
	
	for(int i = 0; i < dimensions + 1; i++){
		sumForRoot += pow(points[i].height - meanValue, 2);
	}
	
	result = sqrt(sumForRoot/dimensions);
	return (result < limit);
}

//Compares values of height for two points. For use with qsort
int compare(const void *a, const void *b){
	Position *posA = (Position *)a;
	Position *posB = (Position *)b;
	
	return posB->height > posA->height;
}

//Function for calculating y-values for a given function
void calcHeights(Position points[], double (func)(Position)){
	for(int i = 0; i < dimensions + 1; i++){
		points[i].height = func(points[i]);
	}
}

//Turns an array of pointers to an array of type Position
void arrayToStruct(double *pointerPoints[], Position points[]){
	for(int i = 0; i < dimensions + 1; i++){
		for(int j = 0; j < dimensions; j++){
			points[i].coords[j] = pointerPoints[i][j];
		}
	}
}

//This function generates the values for part 2, though the range for x_0 is chosen when calling the function
void hundredValues(double (func)(Position), double start, double end){
	FILE *file;
	file = fopen("hundredValues.txt", "w");
	Position point;
	point.coords[0] = 0., point.coords[1] = 1.;
	double stepSize = (end-start)/100;
	
	for(int i = 0; i < 100; i++){
		point.coords[0] = start + (i * stepSize);
		fprintf(file, "%lf, %lf\n", point.coords[0], func(point));
	}
	fclose(file);
}
