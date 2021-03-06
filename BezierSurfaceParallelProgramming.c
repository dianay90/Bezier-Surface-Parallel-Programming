
#include <omp.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

#ifndef NUMT 
#define NUMT	         5
#endif

#ifndef NUMNODES
#define NUMNODES		50
#endif

#ifndef NUMTRIES 
#define NUMTRIES		6
#endif



#define XMIN	 0.
#define XMAX	 3.
#define YMIN	 0.
#define YMAX	 3.

#define TOPZ00  0.
#define TOPZ10  1.
#define TOPZ20  0.
#define TOPZ30  0.

#define TOPZ01  1.
#define TOPZ11  6.
#define TOPZ21  1.
#define TOPZ31  0.

#define TOPZ02  0.
#define TOPZ12  1.
#define TOPZ22  0.
#define TOPZ32  4.

#define TOPZ03  3.
#define TOPZ13  2.
#define TOPZ23  3.
#define TOPZ33  3.

#define BOTZ00  0.
#define BOTZ10  -3.
#define BOTZ20  0.
#define BOTZ30  0.

#define BOTZ01  -2.
#define BOTZ11  10.
#define BOTZ21  -2.
#define BOTZ31  0.

#define BOTZ02  0.
#define BOTZ12  -5.
#define BOTZ22  0.
#define BOTZ32  -6.

#define BOTZ03  -3.
#define BOTZ13   2.
#define BOTZ23  -8.
#define BOTZ33  -3.


//float
float height(int iu, int iv)	// iu,iv = 0 .. NUMNODES-1
{
	float u = (float)iu / (float)(NUMNODES - 1);
	float v = (float)iv / (float)(NUMNODES - 1);

	// the basis functions:

	float bu0 = (1. - u) * (1. - u) * (1. - u);
	float bu1 = 3. * u * (1. - u) * (1. - u);
	float bu2 = 3. * u * u * (1. - u);
	float bu3 = u * u * u;

	float bv0 = (1. - v) * (1. - v) * (1. - v);
	float bv1 = 3. * v * (1. - v) * (1. - v);
	float bv2 = 3. * v * v * (1. - v);
	float bv3 = v * v * v;

	// finally, we get to compute something:


	float top = bu0 * (bv0*TOPZ00 + bv1*TOPZ01 + bv2*TOPZ02 + bv3*TOPZ03)
		+ bu1 * (bv0*TOPZ10 + bv1*TOPZ11 + bv2*TOPZ12 + bv3*TOPZ13)
		+ bu2 * (bv0*TOPZ20 + bv1*TOPZ21 + bv2*TOPZ22 + bv3*TOPZ23)
		+ bu3 * (bv0*TOPZ30 + bv1*TOPZ31 + bv2*TOPZ32 + bv3*TOPZ33);

	float bot = bu0 * (bv0*BOTZ00 + bv1*BOTZ01 + bv2*BOTZ02 + bv3*BOTZ03)
		+ bu1 * (bv0*BOTZ10 + bv1*BOTZ11 + bv2*BOTZ12 + bv3*BOTZ13)
		+ bu2 * (bv0*BOTZ20 + bv1*BOTZ21 + bv2*BOTZ22 + bv3*BOTZ23)
		+ bu3 * (bv0*BOTZ30 + bv1*BOTZ31 + bv2*BOTZ32 + bv3*BOTZ33);

	
	return top - bot;	// if the bottom surface sticks out above the top surface
		//return 1; 				// then that contribution to the overall volume is negative
}






int main(int argc, char *argv[])
{
	//. . .
#ifndef _OPENMP
	fprintf(stderr, "OpenMP is not supported here -- sorry.\n");
	return 1;
#endif
		// the area of a single full-sized tile:
	omp_set_num_threads(NUMT);

		float fullTileArea = (((XMAX - XMIN) / (float)(NUMNODES - 1))  *
	((YMAX - YMIN) / (float)(NUMNODES - 1)));
	//int fullTileArea = 1; 

		double sumMegaHeights = 0., maxMegaHeights=0., volume =0.; 

		//for 

		for (int t = 0; t < NUMTRIES; t++)
		{
			double time0 = omp_get_wtime();
			volume = 0; 

			int i;
#pragma omp parallel for default(none), shared(fullTileArea),reduction(+:volume)
			
			for ( i = 0; i < NUMNODES*NUMNODES; i++)
			{
				int iu_edge=0, iv_edge=0; 

				int iu = i % NUMNODES;
				int iv = i / NUMNODES;

		
			
				if (iu == 0 || iu == NUMNODES - 1)
				{
					iu_edge=1;
				}

				if (iv == 0 || iv == NUMNODES - 1)
				{
					iv_edge=1;
				}
				
			
				//Volume equations 

				if (iu_edge  && iv_edge )
				{
					volume += (fullTileArea / 4.0) * height(iu, iv);
					
				}
				//full 
				else if (iu_edge == 0 && iv_edge == 0)
				{
					volume += (fullTileArea)* height(iu, iv);
				}
				//half 

				else if (iu_edge || iv_edge)
				{
					volume += (fullTileArea / 2.0)* height(iu, iv);
				
				}

			




			}

			double time1 = omp_get_wtime();

			double megaHeights = (double)(NUMNODES*NUMNODES) / (time1 - time0) / 1000000;
			sumMegaHeights += megaHeights;
			if (megaHeights > maxMegaHeights)
				maxMegaHeights = megaHeights;
			

		}
		

		double avgMegaHeights = sumMegaHeights / (double)NUMTRIES;
	//	prnintf("Number of Node %d", NUMNODES);
		printf("   Peak Performance = %8.2lf MegaHeights/Sec\n", maxMegaHeights);
		printf("Average Performance = %8.2lf MegaHeights/Sec\n", avgMegaHeights);
		printf("Volume= %lf", volume);
		
	return 0; 
}