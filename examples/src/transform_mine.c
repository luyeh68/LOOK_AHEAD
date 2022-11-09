//   _____                     __                      
//  |_   _| __ __ _ _ __  ___ / _| ___  _ __ _ __ ___  
//    | || '__/ _` | '_ \/ __| |_ / _ \| '__| '_ ` _ \
//    | || | | (_| | | | \__ \  _| (_) | |  | | | | | |
//    |_||_|  \__,_|_| |_|___/_|  \___/|_|  |_| |_| |_|
// 
// Roto-translation of a point in 3D
// Rotation is about Z axis on XY plane
// Build with:
//    clang -lgsl -lm -o transform src/transform.c
// If you want to use gcc, the order MATTERS:
//    gcc src/transform.c -lgsl -lm -o transform
#include <gsl/gsl_matrix.h> // to deal with LA problems
#include <gsl/gsl_vector.h>
#include <gsl/gsl_blas.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif


typedef struct {
    double x, y, z;
} point_t;

int transform(point_t *point, point_t *direction, double angle) // to transform (rotate (on x, y axis so about z axis) and translate) our rotated and translated vector
{
    gsl_matrix *T  = gsl_matrix_alloc(4, 4);
    gsl_vector *p  = gsl_vector_alloc(4);
    gsl_vector *p1 = gsl_vector_alloc(4);

    double theta     = angle / 180.0 * M_PI;
    double sin_theta = sin(theta);
    double cos_theta = cos(theta);

    // Fill gsl_vector p with point_t point 
    gsl_vector_set_all(p, 1); // I want all the elements of vector p1 = 1 then I change the first 3: I want the elements of point_t point inside p
    gsl_vector_set(p, 0, point->x);
    gsl_vector_set(p, 1, point->y);
    gsl_vector_set(p, 2, point->z);

    gsl_matrix_set_identity(T); // setting identity matrix

    // ----First line----
    gsl_matrix_set(T, 0, 0, cos_theta);
    gsl_matrix_set(T, 0, 1, -sin_theta);
    //gsl_matrix_set(T, 0, 2, 0);
    gsl_matrix_set(T, 0, 3, direction->x);
    
    // ----Second line----
    gsl_matrix_set(T, 1, 0, sin_theta);
    gsl_matrix_set(T, 1, 1, cos_theta);
    //gsl_matrix_set(T, 1, 2, 0);
    gsl_matrix_set(T, 1, 3, direction->y);
    
    // ----Third  line----
    gsl_matrix_set(T, 2, 3, direction->z);
    //gsl_matrix_set(T, 2, 0, 0);
    //gsl_matrix_set(T, 2, 1, 0);
    //gsl_matrix_set(T, 2, 2, 1);
    
    // ----Fourth  line----
    //gsl_matrix_set(T, 3, 0, 0);
    //gsl_matrix_set(T, 3, 1, 0);
    //gsl_matrix_set(T, 3, 2, 0);
    //gsl_matrix_set(T, 3, 3, 1);

    int returnValue = gsl_blas_dgemv(CblasNoTrans, 1, T, p, 0, p1); // this is a second level general matrix - vector - multiplication
    
    //copy the value of point_t p1 inside our point_t point
    point->x = gsl_vector_get(p1, 0);
    point->y = gsl_vector_get(p1, 1);
    point->z = gsl_vector_get(p1, 2);

    // free matrix - vector allocator
    gsl_vector_free(p);
    gsl_vector_free(p1);
    gsl_matrix_free(T);

    return returnValue;
}


int main(){
    //input point
    point_t point     = {.x = 10, .y =3, .z = 7};

    point_t direction = {.x = 10, .y = 0, .z = 5}; // Translate
    double angle      = 45.0f; //Rotate

    fprintf(stdout, "point     = (%6.3f - %6.3f - %6.3f)\n", point.x, point.y, point.z);

    fprintf(stdout, "direction = (%6.3f - %6.3f - %6.3f)\n", direction.x, direction.y, direction.z);
    
    fprintf(stdout, "angle     = %6.3f\n", angle);

    int returnValue = transform(&point, &direction, angle); //storing the value of transformation we will do

    fprintf(stdout, "The rotated vector is  = (%6.3f - %6.3f - %6.3f)\n", point.x, point.y, point.z);

}