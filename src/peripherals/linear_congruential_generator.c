/**
 * @file linear_congruential_generator.c
 * @author Medad Newman (medad@medadnewman.co.uk)
 * @brief 
 * @version 0.1
 * @date 2021-09-11
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "linear_congruential_generator.h"
#include <math.h>
#include "stdbool.h"
#include <stdlib.h>


int mapping(int i, int start, int step);
int generate_random(int l, int r);
int next_LCG(void);


struct LGC_params
{
	int stop;
	int start;
	int maximum;
	int value;
	int offset;
	int step;
	int multiplier;
	int modulus;
	int found;
} LGC_current_params;




/**
 * \brief Initialise the LGC function
 * 
 * \param start
 * \param stop
 * \param step
 * 
 * \return void
 */
void init_LGC(int start, int stop, int step)
{
	if (start >= stop)
	{
		start = 0;
		stop = 1;
	}

	LGC_current_params.found = 0;
	LGC_current_params.stop = stop;
	LGC_current_params.start = start;
	LGC_current_params.step = step;

	LGC_current_params.maximum = (int)floor((LGC_current_params.stop - LGC_current_params.start) / LGC_current_params.step);
	LGC_current_params.value = generate_random(0, LGC_current_params.maximum);

	LGC_current_params.offset = generate_random(0, LGC_current_params.maximum) * 2 + 1;
	LGC_current_params.multiplier = 4 * (int)floor(LGC_current_params.maximum / 4) + 1;
	LGC_current_params.modulus = (int)pow(2, ceil(log2(LGC_current_params.maximum)));
}

/**
 * \brief Adapted from https://stackoverflow.com/a/53551417
 * 
 * \param start
 * \param stop
 * \param step
 * 
 * \return int
 */
int next_LCG()
{
	bool done = false;
	int res;
	while (1)
	{
		// If this is a valid value, yield it in generator fashion.
		if (LGC_current_params.value < LGC_current_params.maximum)
		{
			res = mapping(LGC_current_params.value, LGC_current_params.start, LGC_current_params.step);
			done = true;
		}
		// Calculate the next value in the sequence.
		LGC_current_params.value = (LGC_current_params.value * LGC_current_params.multiplier + LGC_current_params.offset) % LGC_current_params.modulus;

		if (done == true)
		{
			return res;
		}
	}
}

/**
 * \brief Return a value, within the bounds of lower_val(inclusive) and upper_val(not inclusive)
 * 
 * \param lower_val
 * \param upper_val
 * 
 * \return int
 */
int LCG(int lower_val, int upper_val)
{
	if ((LGC_current_params.start != lower_val) || (LGC_current_params.stop != upper_val))
	{
		init_LGC(lower_val, upper_val, 1);
	}

	return next_LCG();
}

int mapping(int i, int start, int step)
{
	return (i * step) + start;
}



/**
 * \brief This will generate random number in range l and r, inclusive of both
 * 
 * \param l: lower bound
 * \param r: upper bound
 * 
 * \return int random number
 */
int generate_random(int l, int r)
{
	int rand_num = (rand() % (r - l + 1)) + l;

	return rand_num;
}
