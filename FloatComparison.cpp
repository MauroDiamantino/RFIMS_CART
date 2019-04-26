/*
 * FloatComparison.cpp
 *
 *  Created on: 26/04/2019
 *      Author: new-mauro
 */

#include "RFIMS_CART.h"

//! Function to compare floating-point numbers, taking into account the rounding errors.
/*! This function was copied and modified from the following link:
 * https://www.learncpp.com/cpp-tutorial/35-relational-operators-comparisons/
 * It is based in the Knuth’s algorithm but it uses two epsilons, an absolute epsilon (ABS_EPSILON) which is very small
 * and is intended to compare near-zero floating-point numbers and a relative epsilon (REL_EPSILON, which is a percentage
 * of the biggest operand) to compare the rest of the floating-point numbers. The function returns true if the difference
 * between a and b is less than ABS_EPSILON, or within REL_EPSILON percent of the larger of a and b.
 */

const float ABS_EPSILON = 1e-12;
const float REL_EPSILON = 1e-5;

bool approximatelyEqual(float a, float b)
{
    // Check if the numbers are really close -- needed when comparing numbers near zero.
    float diff = fabs(a - b);
    if (diff <= ABS_EPSILON)
        return true;

    // Otherwise fall back to Knuth's algorithm
    return diff <= ( (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * REL_EPSILON);
}

//! Function to compare vectors containing floating-point numbers, taking into account the rounding errors.
/*! This function is based on the function presented in the following link:
 * https://www.learncpp.com/cpp-tutorial/35-relational-operators-comparisons/
 * It is based in the Knuth’s algorithm but it uses two epsilons, an absolute epsilon (ABS_EPSILON) which is very small
 * and is intended to compare near-zero floating-point numbers and a relative epsilon (REL_EPSILON, which is a percentage
 * of the biggest operand) to compare the rest of the floating-point numbers. The function returns true if the difference
 * between a and b is less than ABS_EPSILON, or within REL_EPSILON percent of the larger of a and b.
 */
bool approximatelyEqual(std::vector<float> vectorA, std::vector<float> vectorB)
{
	bool flagEqual = true;

	if( vectorA.size() != vectorB.size() )
		return false;
	else
	{
		auto itA=vectorA.begin();
		auto itB=vectorB.begin();
		do
		{
			// Check if the numbers are really close -- needed when comparing numbers near zero.
			float diff = fabs(*itA - *itB);
			if (diff > ABS_EPSILON)
			{
				// Otherwise fall back to Knuth's algorithm
				if( diff > ( ( fabs(*itA) < fabs(*itB) ? fabs(*itB) : fabs(*itA) ) * REL_EPSILON) )
					flagEqual = false;
			}

			itA++; itB++;
		}while( flagEqual && itA!=vectorA.end() && itB!=vectorB.end() );

		return flagEqual;
	}
}


