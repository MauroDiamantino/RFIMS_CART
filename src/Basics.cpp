/*! \file Basics.cpp
 *	\brief This file contains the definitions of the functions and classes' methods which have been declared in file Basics.h.
 *	\author Mauro Diamantino
 */

//! Inclusion of header file which has the declarations of the functions and class' methods which are defined in this file.
#include "Basics.h"

//! This constant is used in function approximatelyEqual() and it defines the absolute epsilon.
/*! It defines a small difference between two floating-point numbers, near to zero, which is used
 * 	to consider the numbers approximately equal.
 */
static const float ABS_EPSILON = 1e-5;

//! This constant is used in function approximatelyEqual() and it defines the relative epsilon.
/*! It represents a percentage of the biggest operand when two floating-point numbers, not near to zero, are compared,
 * 	and this percentage defines the maximum difference to consider the numbers approximately equal.
 */
static const float REL_EPSILON = 2e-5;

/*! This function was copied from this [link](https://www.learncpp.com/cpp-tutorial/35-relational-operators-comparisons/),
 * 	but it was modified. It is based in the Knuth’s algorithm but it uses two epsilons, an absolute epsilon (ABS_EPSILON)
 * 	which is very small and is intended to compare near-zero floating-point numbers and a relative epsilon (REL_EPSILON,
 * 	which is a percentage of the biggest operand) to compare the rest of the floating-point numbers. The function returns
 * 	true if the difference between a and b is less than ABS_EPSILON, or within REL_EPSILON percent of the larger of a and b.
 * 	\param [in] a The left-hand side argument.
 * 	\param [in] b The right-hand side argument.
 */
bool approximatelyEqual(float a, float b)
{
    // Check if the numbers are really close -- needed when comparing numbers near zero.
    float diff = fabs(a - b);
    if (diff <= ABS_EPSILON)
        return true;

    // Otherwise fall back to Knuth's algorithm
    return diff <= ( ( fabs(a) < fabs(b) ? fabs(b) : fabs(a) ) * REL_EPSILON );
}

/*! This function is based on the function presented in this
 * [link](https://www.learncpp.com/cpp-tutorial/35-relational-operators-comparisons/).
 *
 * It is based in the Knuth’s algorithm but it uses two epsilons, an absolute epsilon (ABS_EPSILON) which is very small
 * and is intended to compare near-zero floating-point numbers and a relative epsilon (REL_EPSILON, which is a percentage
 * of the biggest operand) to compare the rest of the floating-point numbers. The function returns true if the difference
 * between a and b is less than ABS_EPSILON, or within REL_EPSILON percent of the larger of a and b.
 * 	\param [in] vectorA The left-hand side argument.
 * 	\param [in] vectorB The right-hand side argument.
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

void WaitForKey()
{
    cin.clear();
    cin.ignore(std::cin.rdbuf()->in_avail());
    cin.get();
}

/*! \param [in] vect A `std::vector<float>` container which must be negated.	*/
std::vector<FreqValues::value_type> operator-(const std::vector<FreqValues::value_type> & vect)
{
	std::vector<FreqValues::value_type> result;
	for(auto& value : vect)
		result.push_back( -value );
	return result;
}

void InitializeGPIO()
{
#ifdef RASPBERRY_PI
	//Initializing the Wiring Pi library
	wiringPiSetup();

	pinMode(piPins.NOISE_SOURCE, OUTPUT);
	digitalWrite(piPins.NOISE_SOURCE, LOW);

	pinMode(piPins.SWITCH, OUTPUT);
	digitalWrite(piPins.SWITCH, SWITCH_TO_NS);

	pinMode(piPins.LNAS, OUTPUT);
	digitalWrite(piPins.LNAS, LOW);

	pinMode(piPins.SPECTRAN, OUTPUT);
	digitalWrite(piPins.SPECTRAN, LOW);

	pinMode(piPins.LED_SWEEP_CAPTURE, OUTPUT);
	digitalWrite(piPins.LED_SWEEP_CAPTURE, LOW);

	pinMode(piPins.LED_SWEEP_PROCESS, OUTPUT);
	digitalWrite(piPins.LED_SWEEP_PROCESS, LOW);

	pinMode(piPins.LED_INIT_POS, OUTPUT);
	digitalWrite(piPins.LED_INIT_POS, LOW);

	pinMode(piPins.BUTTON_ENTER, INPUT);
	pullUpDnControl(piPins.BUTTON_ENTER, PUD_UP);

	pinMode(piPins.LED_NEXT_POS, OUTPUT);
	digitalWrite(piPins.LED_NEXT_POS, LOW);

	pinMode(piPins.LED_POLARIZ, OUTPUT);
	digitalWrite(piPins.LED_POLARIZ, LOW);
#endif
}

void TurnOffLeds()
{
#ifdef RASPBERRY_PI
	digitalWrite(piPins.LED_SWEEP_CAPTURE, LOW);
	digitalWrite(piPins.LED_SWEEP_PROCESS, LOW);
	digitalWrite(piPins.LED_INIT_POS, LOW);
	digitalWrite(piPins.LED_NEXT_POS, LOW);
	digitalWrite(piPins.LED_POLARIZ, LOW);
#endif
}

/*! \details To turn on the RF front-end elements in a sequential manner, first, this function turns the spectrum
 * 	analyzer on, then it waits 5 seconds, it turns the LNAs on, it waits again but this time just
 * 	1 second and, finally, it switches the input to the antenna.
 */
void TurnOnFrontEnd()
{
#ifdef RASPBERRY_PI
	digitalWrite(piPins.SPECTRAN, HIGH);
	sleep(5);
	digitalWrite(piPins.LNAS, HIGH);
	sleep(1);
	digitalWrite(piPins.SWITCH, SWITCH_TO_ANTENNA);
#endif
}

/*! \details To turn off the RF front-end elements sequentially, this function begins ensuring the noise source
 * 	is turned off, then it switches the input to that device, it waits 1 second, it turns the LNAs off,
 * 	it waits again 1 second and, finally, it turns the spectrum analyzer off.
 */
void TurnOffFrontEnd()
{
#ifdef RASBPERRY_PI
	digitalWrite(piPins.NOISE_SOURCE, LOW);
	digitalWrite(piPins.SWITCH, SWITCH_TO_NS);
	sleep(1);
	digitalWrite(piPins.LNAS, LOW);
	sleep(1);
	digitalWrite(piPins.SPECTRAN, LOW);
#endif
}
