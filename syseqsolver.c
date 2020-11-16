
// System of equations solver
// Alejandro Gilson Campillo
// 07/11/2020

#include <stdio.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_permutation.h>
#include <gsl/gsl_linalg.h>
#include "math.h"

int main (int argc, char *argv[])
{

    int    numPoints = 11;
    double start     = 0;
    double end       = 1;
    double th        = 0.5 * (end- start) + start;
    int    signum;

    if (argc == 5)
    {
        numPoints = atoi(argv[1]);
        start     = atof(argv[2]);
        end       = atof(argv[3]);
        th        = atof(argv[4]) * (end- start) + start;
    }


    // printf("Number of points: %i\n",numPoints);
    
    double step    = (end - start)/numPoints;
    double *b_data = calloc(numPoints, sizeof(double));
    double *a_data = calloc(numPoints*numPoints, sizeof(double));
    double *a      = calloc(numPoints, sizeof(double));

    // Open files
    FILE  *coeffsFile  = fopen("polyCoeff.csv", "w");
    FILE  *aDataFile   = fopen("aData.csv"    , "w");
    FILE  *bDataFile   = fopen("bData.csv"    , "w");
    FILE  *pointsXFile = fopen("pointsX.csv"  , "w");
    FILE  *pointsYFile = fopen("pointsY.csv"  , "w");

    for(int k = 0; k < numPoints; k++)
        a[k] = step*k + start;

    // Build the matrix a_data and vector b_data
    for (int n = 0; n < numPoints; n++)
    {
        if(fabs(a[n]) < th)
            b_data[n] = a[n];

        else
            b_data[n] = (a[n] < 0) ? -th : th; 

        for(int k = numPoints - 1 ; k > -1; k--)
            a_data[n*numPoints + k] = pow(a[n], numPoints - 1 - k);
    }

    // Allocate matrix memory
    gsl_matrix_view m   = gsl_matrix_view_array (a_data, numPoints, numPoints);
    gsl_vector_view b   = gsl_vector_view_array (b_data, numPoints);
    gsl_vector *x       = gsl_vector_alloc (numPoints);
    gsl_permutation * p = gsl_permutation_alloc (numPoints);

    gsl_matrix_fprintf (aDataFile, &m.matrix, "%f");
    gsl_vector_fprintf (bDataFile, &b.vector, "%f");

    // Solve system of equations
    gsl_linalg_LU_decomp (&m.matrix, p, &signum);
    gsl_linalg_LU_solve  (&m.matrix, p, &b.vector, x);

    // Print points and solution
    for(int i = 0; i < numPoints; i++)
    {
        fprintf(pointsXFile, "%f\n", a[i]);
        fprintf(pointsYFile, "%f\n", b_data[i]);
    }

    // Print solution to file
    gsl_vector_fprintf (coeffsFile, x, "%g");

    // Deallocate matrixes and arrays
    gsl_permutation_free (p);
    gsl_vector_free (x);
    free(a_data);    
    free(b_data);    
    free(a);
    
    // Close files
    fclose(coeffsFile);
    fclose(pointsXFile);
    fclose(pointsYFile);

    return 0;
}