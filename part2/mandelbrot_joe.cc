/**
 *  \file mandelbrot_joe.cc
 *
 *  \brief Implement your parallel mandelbrot set in this file.
 */

#include <iostream>
#include <cstdlib>
#include <mpi.h>
#include "render.hh"

 int mandelbrot(double x, double y) {
 	int maxit = 511;
 	double cx = x;
 	double cy = y;
 	double newx, newy;

 	int it = 0;
 	for (it = 0; it < maxit && (x*x + y*y) < 4; ++it) {
 		newx = x*x - y*y + cx;
 		newy = 2*x*y + cy;
 		x = newx;
 		y = newy;
 	}
 	return it;
 }

 int susie_rank(int i){
 	int P; 
 	MPI_Comm_size(MPI_COMM_WORLD, &P);
 	int n = i / P;
 	return (i - n*P) ;
 }

int joe_rank(int i){
	int P;
 	MPI_Comm_size(MPI_COMM_WORLD, &P);
 	int n = i / P;
 	return n % P ;
 }

 int main (int argc, char* argv[])
 {
 	MPI_Init(NULL, NULL);
 	int rank;
 	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

 	int height, width;
 	if (argc == 3) {
 		height = atoi (argv[1]);
 		width = atoi (argv[2]);
 		assert (height > 0 && width > 0);
 	} else {
 		fprintf (stderr, "usage: %s <height> <width>\n", argv[0]);
 		fprintf (stderr, "where <height> and <width> are the dimensions of the image.\n");
 		return -1;
 	}

 	double minX = -2.1;
 	double maxX = 0.7;
 	double minY = -1.25;
 	double maxY = 1.25;

 	double it = (maxY - minY)/height;
 	double jt = (maxX - minX)/width;
 	double x, y;

 	gil::rgb8_image_t img(height, width);
 	auto img_view = gil::view(img);
 	double image[height][width];
 	double row[width];

 	for (int i = 0; i < height; i++) {
 		if (rank != joe_rank(i)){
 			y += jt;
 			continue;
 		} else {
 			x = minX;
 			for (int j = 0; j < width; j++) {
 				row[j] = mandelbrot(x,y);
 				x += jt;
 			}
 			y += jt;
 			MPI_Gather(row, width,MPI_DOUBLE,image[i],width,MPI_DOUBLE,0,MPI_COMM_WORLD);
 		}
 	}

 	MPI_Barrier(MPI_COMM_WORLD);
 	if (rank == 0){
 		for (int i = 0; i < height; i++){
 			for (int j = 0; j < width; j++){
 				img_view(j, i) = render(image[i][j]/512.0);
 			}
 		}
 		gil::png_write_view("mandelbrot.png", const_view(img));
 	}
 	MPI_Finalize();
 	return 0;
 }
