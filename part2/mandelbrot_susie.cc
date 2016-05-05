/**
 *  \file mandelbrot_susie.cc
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

 int susie(int i){
 	int P, rank; 
 	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
 	MPI_Comm_size(MPI_COMM_WORLD, &P);
 	return (i - rank) % (P) == 0
 }

 int joe(int i){
 	int P, rank; 
 	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
 	MPI_Comm_size(MPI_COMM_WORLD, &P);
 	return (i > (rank - 1) * P) && (i <= rank * P)
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

 	y = minY;
 	for (int i = 0; i < height; ++i) {
 		x = minX;
 		for (int j = 0; j < width; ++j) {
 			img_view(j, i) = render(mandelbrot(x, y)/512.0);
 			x += jt;
 		}
 		y += it;
 	}

 	double row[width];
 	if (rank == 0) {
 		for (int i = 1; i < height + 1; i++) {
 			MPI_Recv(&row, width, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
 			for (int j =0; j < width; j++) {
 				img_view(j, i) = render(row[j]/512.0);
 			}
 		}
 		gil::png_write_view("mandelbrot.png", const_view(img));
 	} else {
 		y = minY;
 		for (int i = 0; i < height; i++) {
 			x = minX;
 			if (susie(i)){
 				for (int j = 0; j < width; j++) {
 					row[j] = render(mandelbrot(x,y));
 					x += jt;
 				}
 				MPI_Send(&row, width, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
 			}
 			y += jt;
 		}
 	}
 	MPI_Finalize();
 	return 0;
 }

