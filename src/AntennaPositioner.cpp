/*! \file AntennaPositioner.cpp
 * 	\brief This file contains the definitions of several methods of the class _AntennaPositioner_.
 * 	\author Emanuel Asencio
 * 	\author Mauro Diamantino
 */

#include "AntennaPositioning.h"

/*! \param gpsInterf A reference to the object which is responsible for the communication with the Aaronia GPS receiver. */
AntennaPositioner::AntennaPositioner(GPSInterface & gpsInterf) : gpsInterface(gpsInterf)
{
	azimuthAngle=0.0;
	polarization=Polarization::UNKNOWN;
	positionIndex=255;
}

/*!	This initialization implies to move the antenna to its initial position, to capture the initial azimuth
 * angle and to ensure the antenna polarization is horizontal.
 * \return A `true` if the initialization was successful or a `false` otherwise.
 */
bool AntennaPositioner::Initialize()
{
	cout << "\nRotate antenna to the initial position, make sure the polarization is horizontal and press the button to continue.." << endl;

#ifdef BUTTON //implied that RASPBERRY_PI is defined
	digitalWrite(piPins.LED_INIT_POS, HIGH);
	while( digitalRead(piPins.BUTTON_INIT_POS)==HIGH );
	digitalWrite(piPins.LED_INIT_POS, LOW);
#else
	WaitForKey();
#endif

	bool flagSuccess=false;
	unsigned int numOfErrors=0;
	do
	{
		try
		{
			gpsInterface.ReadOneDataSet();
			azimuthAngle = gpsInterface.GetYaw();
			flagSuccess = true;
		}
		catch(rfims_exception & exc)
		{
			if(++numOfErrors < 3)
				cerr << "Warning: an error occurred when it was tried to read the initial azimuth angle: " << exc.what() << endl;
			else
			{
				exc.Prepend("the initial azimuth angle could not be read");
				throw;
			}
		}
	}while(!flagSuccess);

	polarization=Polarization::HORIZONTAL;
	positionIndex=0;

	return true;
}

/*!	To move the antenna to the next position, this is rotated an angle determined by the number of positions
 * (360/number of positions) and in a clockwise way. If the current position is the last one, then the antenna
 * is move to the initial position, rotating this one counterclockwise to avoid the cables to tangle or stretch.
 *
 * Taking into account the method Initialize(), it is waited this method to be called when the antenna polarization
 * changes from vertical to horizontal.
 * \return A `true` if the operation was successful or a `false` otherwise.
 */
bool AntennaPositioner::NextAzimPosition()
{
	if( ++positionIndex >= NUM_OF_POSITIONS )
	{
		positionIndex=0;
		cout << "Rotate antenna to the initial position, make sure the polarization is horizontal and press the button to continue.." << endl;
#ifdef BUTTON //implied that RASPBERRY_PI is defined
		digitalWrite(piPins.LED_INIT_POS, HIGH);
		while( digitalRead(piPins.BUTTON_ENTER)==HIGH );
		digitalWrite(piPins.LED_INIT_POS, LOW);
#else
		WaitForKey();
#endif
	}
	else
	{
		cout << "Rotate the antenna " << ROTATION_ANGLE << "° clockwise and press the button to continue..." << endl;
	#ifdef BUTTON //implied that RASPBERRY_PI is defined
		digitalWrite(piPins.LED_NEXT_POS, HIGH);
		while( digitalRead(piPins.BUTTON_ENTER)==HIGH );
		digitalWrite(piPins.LED_NEXT_POS, LOW);
	#else
		WaitForKey();
	#endif
	}

	azimuthAngle+=45.0;
	if(azimuthAngle>=360.0) azimuthAngle-=360.0;

	return true;
}

/*! If the current polarization is horizontal, then it is changed to vertical, and vice versa.
 * \return A `true` if the operation was successful or a `false` otherwise.
 */
bool AntennaPositioner::ChangePolarization()
{
	double absRoll;
	bool flagSuccess=false;
	do
	{
		cout << "Change the antenna polarization and press the button to continue..." << endl;
#ifdef BUTTON //implied that RASPBERRY_PI is defined
		digitalWrite(piPins.LED_POLARIZ, HIGH);
		while( digitalRead(piPins.BUTTON_ENTER)==HIGH );
		digitalWrite(piPins.LED_POLARIZ, LOW);
#else
		WaitForKey();
#endif

		bool flagSuccessRead=false;
		unsigned int numOfErrors=0;
		do
		{
			try
			{
				gpsInterface.ReadOneDataSet();
				absRoll = fabs( gpsInterface.GetRoll() );
				if( polarization==Polarization::HORIZONTAL )
					if( 80.0 < absRoll && absRoll < 100.0 )
					{
						flagSuccess=true;
						polarization = Polarization::VERTICAL;
					}
					else
						cout << "\nWarning: The antenna polarization was not changed" << endl;
				else
					if( absRoll < 10.0 )
					{
						flagSuccess = true;
						polarization = Polarization::HORIZONTAL;
					}
					else
						cout << "\nWarning: The antenna polarization was not changed" << endl;

				flagSuccessRead=true;
			}
			catch(rfims_exception & exc)
			{
				if(++numOfErrors < 3)
					cerr << "Warning: an error occurred when it was tried to read the roll angle: " << exc.what() << endl;
				else
				{
					exc.Prepend("the roll angle could not be read");
					throw;
				}
			}
		}while(!flagSuccessRead);

	}while(!flagSuccess);

	return true;
}

/*! \return A `std::string` object with the current antenna polarization.	*/
std::string AntennaPositioner::GetPolarizationString() const
{
	switch(polarization)
	{
	case Polarization::HORIZONTAL:
		return "horizontal";
	case Polarization::VERTICAL:
		return "vertical";
	case Polarization::UNKNOWN:
	default:
		return "unknown";
	}
}
