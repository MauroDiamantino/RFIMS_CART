/*
 * AntennaPositioner.cpp
 *
 *  Created on: 15/04/2019
 *      Author: new-mauro
 */

#include "AntennaPositioning.h"

void WaitForKey();

AntennaPositioner::AntennaPositioner(GPSInterface & gpsInterf) : gpsInterface(gpsInterf)
{
#ifdef RASPBERRY_PI
	//Setting the Raspberry Pi pins
	pinMode(piPins.LED_INIT_POS, OUTPUT);
	pinMode(piPins.BUTTON_INIT_POS, INPUT);
	pullUpDnControl(piPins.BUTTON_INIT_POS, PUD_UP);
	pinMode(piPins.LED_NEXT_POS, OUTPUT);
	pinMode(piPins.BUTTON_NEXT_POS, INPUT);
	pullUpDnControl(piPins.BUTTON_NEXT_POS, PUD_UP);
	pinMode(piPins.LED_POLARIZ, OUTPUT);
	pinMode(piPins.BUTTON_POLARIZ, INPUT);
	pullUpDnControl(piPins.BUTTON_POLARIZ, PUD_UP);

	//Setting the initial values
	digitalWrite(piPins.LED_INIT_POS, LOW);
	digitalWrite(piPins.LED_NEXT_POS, LOW);
	digitalWrite(piPins.LED_POLARIZ, LOW);
#endif
	azimuthAngle=0.0;
	polarization=Polarization::UNKNOWN;
	positionIndex=255;
}

bool AntennaPositioner::Initialize()
{
	cout << "\nRotate antenna to the initial position, make sure the polarization is horizontal and press the button to continue.." << endl;
#ifdef RASPBERRY_PI
	digitalWrite(piPins.LED_INIT_POS, HIGH);
	while( digitalRead(piPins.BUTTON_INIT_POS)==HIGH );
	digitalWrite(piPins.LED_INIT_POS, LOW);
#else
	WaitForKey();
#endif
	cout << "Enter the initial azimuth angle: ";
	cin >> azimuthAngle;
	cin.get();
	//cout << endl;
	polarization=Polarization::HORIZONTAL;
	positionIndex=0;

	return true;
}

bool AntennaPositioner::NextAzimPosition()
{
	if( ++positionIndex >= NUM_OF_POSITIONS )
	{
		positionIndex=0;
		cout << "\nRotate antenna to the initial position, make sure the polarization is horizontal and press the button to continue.." << endl;
#ifdef RASPBERRY_PI
		digitalWrite(piPins.LED_INIT_POS, HIGH);
		while( digitalRead(piPins.BUTTON_INIT_POS)==HIGH );
		digitalWrite(piPins.LED_INIT_POS, LOW);
#else
		WaitForKey();
#endif
	}
	else
	{
		cout << "\nRotate the antenna 45° counterclockwise and press the button to continue..." << endl;
#ifdef RASPBERRY_PI
		digitalWrite(piPins.LED_NEXT_POS, HIGH);
		while( digitalRead(piPins.BUTTON_NEXT_POS)==HIGH );
		digitalWrite(piPins.LED_NEXT_POS, LOW);
#else
		WaitForKey();
#endif
	}

	azimuthAngle+=45.0;
	if(azimuthAngle>=360.0) azimuthAngle-=360.0;

	return true;
}

bool AntennaPositioner::ChangePolarization()
{
	double absRoll;
	bool flagSuccess=false;
	do
	{
		cout << "\nChange the antenna polarization and press the button to continue..." << endl;
	#ifdef RASPBERRY_PI
		digitalWrite(piPins.LED_POLARIZ, HIGH);
		while( digitalRead(piPins.BUTTON_POLARIZ)==HIGH );
		digitalWrite(piPins.LED_POLARIZ, LOW);
	#else
		WaitForKey();
	#endif
		gpsInterface.ReadOneDataSet();
		absRoll = abs( gpsInterface.GetRoll() );
		if( polarization==Polarization::HORIZONTAL )
		{
			if( 80.0 < absRoll && absRoll < 100.0 )
			{
				flagSuccess=true;
				polarization = Polarization::VERTICAL;
			}
			else
			{
				cout << "\nWarning: The antenna polarization was not changed" << endl;
			}
		}
		else
		{
			if( absRoll < 10.0 )
			{
				flagSuccess = true;
				polarization = Polarization::HORIZONTAL;
			}
			else
			{
				cout << "\nWarning: The antenna polarization was not changed" << endl;
			}
		}
	}while(!flagSuccess);

	return true;
}

std::string AntennaPositioner::GetPolarizationString()
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
