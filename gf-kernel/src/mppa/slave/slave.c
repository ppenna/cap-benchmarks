/*
 * Copyright(C) 2014 Matheus M. Queiroz <matheus.miranda.queiroz@gmail.com>, 
 *                   Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <global.h>
#include <mppaipc.h>
#include <omp.h>
#include "slave.h"

/* Gaussian filter parameters. */
static int masksize;
static double mask[MASK_SIZE*MASK_SIZE];
static unsigned char chunk[CHUNK_SIZE*CHUNK_SIZE];

/*
 * Gaussian filter.
 */
void gauss_filter(void)
{
	int i, j;
	int half;
	double new_element;
	int imgI, imgJ, maskI, maskJ;
	
	#define MASK(i, j) \
		mask[(i)*masksize + (j)]
	
	#define CHUNK(i, j) \
		chunk[(i)*CHUNK_SIZE + (j)]
	
	i = 0; j = 0;
	half = CHUNK_SIZE >> 1;
	
	#pragma omp parallel for  default(shared) \
	private(imgI, imgJ, maskI, maskJ, new_element)
	for (imgI = 0; imgI < CHUNK_SIZE; imgI++)
	{		
		for (imgJ = 0; imgJ < CHUNK_SIZE; imgJ++)
		{
			new_element = 0.0;
			for (maskI = 0; maskI < CHUNK_SIZE; maskI++)
			{	
				for (maskJ = 0; maskJ < CHUNK_SIZE; maskJ++)
				{
					i = (imgI - half < 0) ? CHUNK_SIZE-1 - maskI : imgI - half;
					j = (imgJ - half < 0) ? CHUNK_SIZE-1 - maskJ : imgJ - half;

					new_element += CHUNK(i, j)*MASK(maskI, maskJ);
				}
			}
               
			CHUNK(imgI, imgJ) = (new_element > 255) ? 255 : (int)new_element;
		}
	}
}


int main(int argc, char **argv)
{
	int msg;

	((void)argc);

	rank = atoi(argv[0]);	
	
	/* Setup interprocess communication. */
	open_noc_connectors();
	sync_master();
	
	/* Receives filter mask.*/
	data_receive(infd, &masksize, sizeof(int));
	data_receive(infd, mask, sizeof(double)*masksize*masksize);
    
	/* Process chunks. */
    while (1)
	{
		data_receive(infd, &msg, sizeof(int));

		/* Parse message. */
		switch (msg)
		{
			case MSG_CHUNK:
				data_receive(infd, chunk, CHUNK_SIZE*CHUNK_SIZE);
				gauss_filter();
				data_send(outfd, chunk, CHUNK_SIZE*CHUNK_SIZE);
				break;
			
			default:
				goto out;
		}
	}

out:
	
	close_noc_connectors();
	
	mppa_exit(0);
	return (0);
}
	