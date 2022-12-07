#ifndef __INCLUDED_TRIANGLE_H
#define __INCLUDED_TRIANGLE_H

struct coordinate {
	int x;
	int y;
};

struct triangle {
	struct coordinate point[3];
};


/**
 * Given a triangle with all corners on integer coordinates, countPoints()
 * counts the number of points (on integer coordinates) on the boundary of the
 * triangle and the number of points inside the triangle.
 *
 * This function continuously calls the given callback function to signal that
 * an additional number of points has been found. The callback function should
 * increment the number of found points by the given amount and signal the
 * output thread to update the progress report on stdout.
 */
void countPoints(struct triangle *tri, void (*callback)(int boundary, int interior));

#endif
