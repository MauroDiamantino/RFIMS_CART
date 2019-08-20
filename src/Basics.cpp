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
std::vector<float> operator-(const std::vector<float> & vect)
{
	std::vector<float> result;
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

void PrintHelp()
{
	cout << "Usage: rfmis-cart [--plot] [--no-frontend-cal] [--rfi={ska-mode1,ska-mode2,itu-ra769}] [--num-meas-cycles='number'] [--no-upload] [--help | -h]" << endl;
	cout << "\nThis software was designed to capture RF power measurements from a spectrum analyzer Aaronia Spectran V4, using an antenna" << endl;
	cout << "which could be rotated to point the horizon in different azimuth angles and whose polarization could be changed between" << endl;
	cout << "horizontal and vertical. A sweep from 1 GHz (or maybe less) to 9.4 GHz is captured in each antenna position and then it is calibrated," << endl;
	cout << "it is processed to identify RF interference (RFI), it is saved into memory, it is plotted with the detected RFI and finally" << endl;
	cout << "the measurements are sent to a remote server. The software's arguments can be put in any order." << endl;
	cout << "\nThe arguments' descriptions are presented in the following:" << endl;
	cout << "\t--plot\t\t\t\t\t\tEnable the plotting of the different RF data which are got by the software." << endl;
	cout << "\t\t\t\t\t\t\t  If this argument is not given no plot is produced." << endl;
	cout << "\t--no-frontend-cal\t\t\t\tDisable the front end calibration, i.e. the estimation of the front end's parameters," << endl;
	cout <<	"\t\t\t\t\t\t\t  total gain and total noise figure, using a noise generator. Instead of that," << endl;
	cout << "\t\t\t\t\t\t\t  default front end's parameters curves are used to calibrated the sweeps." << endl;
	cout << "\t\t\t\t\t\t\t  If this argument is not given the front end calibration is performed normally," << endl;
	cout << "\t\t\t\t\t\t\t  turning the noise generator on and off." << endl;
	cout << "\t--rfi={ska-mode1,ska-mode2,itu-ra769-2-vlbi}\tEnable the identifying of RF interference (RFI). The user has to provide the norm" << endl;
	cout << "\t\t\t\t\t\t\t  (or protocol) which must be taken into account to define the harmful levels of RFI:" << endl;
	cout << "\t\t\t\t\t\t\t  The SKA protocol Mode 1, The SKA protocol Mode 2 or the ITU's recommendation." << endl;
	cout << "\t\t\t\t\t\t\t  RA.769-2. If this argument is not given the RFI identifying is not performed." << endl;
	cout << "\t--num-meas-cycles='number'\t\t\tDetermine the number of measurements cycles which must be performed. A measurement" << endl;
	cout << "\t\t\t\t\t\t\t  cycle is formed by all the sweeps which are captured while the antenna goes over" << endl;
	cout << "\t\t\t\t\t\t\t  the 360° of azimuth angle. If this argument is not given the measurement" << endl;
	cout << "\t\t\t\t\t\t\t  cycles are performed indefinitely." << endl;
	cout << "\t--no-upload\t\t\t\t\tDisable the uploading of data, i.e., the sending of collected data to the remote server." << endl;
	cout << "\t-h, --help\t\t\t\t\tShow this help and finish there." << endl;
}

/*! \details This function determines the values of the behavior flags (flagCalEnabled, flagPlot, flagRFI, etc.) taking
 * into account the arguments that were received and its values. The function returns a `true` value if the arguments
 * were processed correctly and a `false` value if there was an argument which could not be recognized, and in that case
 * it presents a message, in the `stdout`, explaining the correct use of the software arguments.
 * \param [in] argc The number of arguments that were received by the software.
 * \param [in] argv An array of C strings (`char*`) where each one is a software's argument.
 */
bool ProcessMainArguments (int argc, char * argv[])
{
	if(argc>1)
	{
		unsigned int argc_aux=argc;
		std::list<std::string> argList;
		for(unsigned int i=1; i<argc_aux; i++)
			argList.push_back( argv[i] );

		//Searching the argument --help
		auto argIter = argList.cbegin();
		while( argIter!=argList.cend() && *argIter!="--help" )	argIter++;
		if( argIter!=argList.cend() )
		{
			//The argument was found
			PrintHelp();
			return false;
		}

		//Searching the argument -h
		argIter = argList.cbegin();
		while( argIter!=argList.cend() && *argIter!="-h" )	argIter++;
		if( argIter!=argList.cend() )
		{
			//The argument was found
			PrintHelp();
			return false;
		}

		//Searching the argument --no-frontend-cal
		argIter = argList.cbegin();
		while( argIter!=argList.cend() && *argIter!="--no-frontend-cal" )	argIter++;
		if( argIter!=argList.cend() )
		{
			//The argument was found
			flagCalEnabled=false;
			argList.erase(argIter);
		}

		//Searching the argument --plot
		argIter = argList.cbegin();
		while( argIter!=argList.cend() && *argIter!="--plot" )	argIter++;
		if( argIter!=argList.cend() )
		{
			//The argument was found
			flagPlot=true;
			argList.erase(argIter);
		}

		//Searching the argument --rfi=xxxxx
		argIter = argList.cbegin();
		size_t equalSignPos=0;
		while( argIter!=argList.cend() && argIter->find("--rfi=")==std::string::npos )	argIter++;
		if( argIter!=argList.cend() )
		{
			//The argument was found
			flagRFI=true;
			equalSignPos = argIter->find('=');
			std::string rfiNormStr = argIter->substr(equalSignPos+1);
			if( rfiNormStr=="ska-mode1" )
				rfiNorm = RFI::SKA_MODE1;
			else if( rfiNormStr=="ska-mode2" )
				rfiNorm = RFI::SKA_MODE2;
			else if( rfiNormStr=="itu-ra769-2-vlbi" )
				rfiNorm = RFI::ITU_RA769_2_VLBI;
			else
			{
				cout << "rfims-cart: unrecognized argument '" << *argIter << '\'' << endl;
				cout << "Usage: rfmis-cart [--plot] [--no-frontend-cal] [--rfi={ska-mode1,ska-mode2,itu-ra769}] [--num-meas-cycles='number'] [--help | -h]" << endl;
				return false;
			}

			argList.erase(argIter);
		}

		//Searching the argument --num-meas-cycles=xx
		argIter = argList.cbegin();
		while( argIter!=argList.cend() && argIter->find("--num-meas-cycles=")==std::string::npos )		argIter++;
		if( argIter!=argList.cend() )
		{
			//The argument was found
			flagInfiniteLoop=false;
			equalSignPos = argIter->find('=');
			std::istringstream iss;
			std::string numString = argIter->substr(equalSignPos+1);
			iss.str(numString);
			iss >> numOfMeasCycles;
			argList.erase(argIter);
		}

		//Searching the argument --no-upload
		argIter = argList.cbegin();
		while( argIter!=argList.cend() && argIter->find("--no-upload")==std::string::npos )		argIter++;
		if( argIter!=argList.cend() )
		{
			//The argument was found
			flagUpload=false;
			argList.erase(argIter);
		}

		//Checking if there were arguments which were not recognized
		if(!argList.empty())
		{
			cout << "rfims-cart: the following arguments were not recognized:";
			for(argIter = argList.cbegin(); argIter != argList.cend(); argIter++)
				cout << " \'" << *argIter << '\'';
			cout << endl;
			cout << "Usage: rfmis-cart [--plot] [--no-frontend-cal] [--rfi={ska-mode1,ska-mode2,itu-ra769}] [--num-meas-cycles='number'] [--no-upload] [--help | -h]" << endl;
			return false;
		}
	}
	return true;
}
