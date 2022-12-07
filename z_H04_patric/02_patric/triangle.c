#include <math.h>
#include <stdbool.h>
#include "triangle.h"

static const double eps = 1e-7;

/*
 * Quick'n'dirty check whether a given set of points is
 * collinear (i.e. whether it actually describes a triangle)
 */
static bool checkTriangle(struct triangle *tri) {
	// Q'n'D
	double a1 = tri->point[0].x - tri->point[1].x,
		   a2 = tri->point[0].y - tri->point[1].y;
	double b1 = tri->point[1].x - tri->point[2].x,
		   b2 = tri->point[1].y - tri->point[2].y;
	if(fabs((a1 / a2) - (b1 / b2)) < eps) return false;
	return true;
}

static int min(int a, int b) {
	return a < b ? a : b;
}
static int max(int a, int b) {
	return a > b ? a : b;
}
static bool int01(double d) {
	return d >= -eps && d <= 1 + eps;
}

void countPoints(struct triangle *tri, void (*callback)(int boundary, int interior)) {
	/* Refuse to count points for degenerate triangles */
	if(!checkTriangle(tri))
		return;

	/* Compute boundary box of all points to check */
	int xmin = min(tri->point[0].x, min(tri->point[1].x, tri->point[2].x));
	int xmax = max(tri->point[0].x, max(tri->point[1].x, tri->point[2].x));
	int ymin = min(tri->point[0].y, min(tri->point[1].y, tri->point[2].y));
	int ymax = max(tri->point[0].y, max(tri->point[1].y, tri->point[2].y));

	int boundary = 0, interior = 0,
		cur_boundary = 0, cur_interior = 0;

	/* Iterate over all points in the bounding box */
	for(int x = xmin; x <= xmax; ++x) {
		for(int y = ymin; y <= ymax; ++y) {
			/* Compute barycentric coordinates of a given point */
			double l1 = (double)
						((tri->point[1].y - tri->point[2].y) * (x - tri->point[2].x) +
						 (tri->point[2].x - tri->point[1].x) * (y - tri->point[2].y))
						/
						(double)
						((tri->point[1].y - tri->point[2].y) * (tri->point[0].x - tri->point[2].x) +
						 (tri->point[2].x - tri->point[1].x) * (tri->point[0].y - tri->point[2].y));
			double l2 = (double)
						((tri->point[2].y - tri->point[0].y) * (x - tri->point[2].x) +
						 (tri->point[0].x - tri->point[2].x) * (y - tri->point[2].y))
						/
						(double)
						((tri->point[1].y - tri->point[2].y) * (tri->point[0].x - tri->point[2].x) +
						 (tri->point[2].x - tri->point[1].x) * (tri->point[0].y - tri->point[2].y));
			double l3 = 1 - l1 - l2;

			/* Check position of given point relative to triangle */
			if((fabs(l1) < eps && int01(l2) && int01(l3)) ||
			   (fabs(l2) < eps && int01(l1) && int01(l3)) ||
			   (fabs(l3) < eps && int01(l1) && int01(l2))) {
				boundary++;
				cur_boundary++;
			} else if(l1 >= 0 && l2 >= 0 && l3 >= 0 &&
					  l1 <= 1 + eps && l2 <= 1 + eps && l3 <= 1 + eps) {
				interior++;
				cur_interior++;
			}
		}
		callback(cur_boundary, cur_interior);
		cur_boundary = 0;
		cur_interior = 0;
	}
}
