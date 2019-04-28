/*
 * AntennaPositioner.cpp
 *
 *  Created on: 15/04/2019
 *      Author: new-mauro
 */

#include "AntennaPositioning.h"

void WaitForKey()
{
    cin.clear();
    cin.ignore(std::cin.rdbuf()->in_avail());
    cin.get();
}

AntennaPositioner::AntennaPositioner()
{
#ifdef RASPBERRY_PI
	//Initializing the Wiring Pi library
	wiringPiSetup();

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
	polarization=UNKNOWN;
	positionIndex=255;
}

bool AntennaPositioner::Initialize()
{
	cout << "Rotate antenna to the initial position, make sure the polarization is horizontal and press the button to continue.." << endl;
#ifdef RASPBERRY_PI
	digitalWrite(piPins.LED_INIT_POS, HIGH);
	while( digitalRead(piPins.BUTTON_INIT_POS)==HIGH );
	digitalWrite(piPins.LED_INIT_POS, LOW);
#endif
	cout << "Enter the initial azimuth angle: ";
	cin >> azimuthAngle;
	polarization=HORIZONTAL;
	positionIndex=0;

	return true;
}

bool AntennaPositioner::NextAzimPosition()
{
	if( ++positionIndex >= NUM_OF_POSITIONS )
	{
		positionIndex=0;
		cout << "Rotate antenna to the initial position, make sure the polarization is horizontal and press the button to continue.." << endl;
#ifdef RASPBERRY_PI
		digitalWrite(piPins.LED_INIT_POS, HIGH);
		while( digitalRead(piPins.BUTTON_INIT_POS)==HIGH );
		digitalWrite(piPins.LED_INIT_POS, LOW);
#endif
	}
	else
	{
		cout << "Rotate the antenna 45° counterclockwise and press the button to continue..." << endl;
#ifdef RASPBERRY_PI
		digitalWrite(PIN_LED_AZIMUTHAL, HIGH);
		while( digitalRead(PIN_SW_AZIMUTHAL)==HIGH );
		digitalWrite(PIN_LED_AZIMUTHAL, LOW);
#endif
	}

	azimuthAngle+=45.0;
	if(azimuthAngle>=360.0) azimuthAngle-=360.0;

	return true;
}

bool AntennaPositioner::ChangePolarization()
{
	cout << "Change the antenna polarization and press the button to continue..." << endl;
#ifdef RASPBERRY_PI
	digitalWrite(piPins.LED_POLARIZ, HIGH);
	while( digitalRead(piPins.BUTTON_POLARIZ)==HIGH );
	digitalWrite(piPins.LED_POLARIZ, LOW);
#endif
	polarization = (polarization==HORIZONTAL ? VERTICAL : HORIZONTAL);
	return true;
}

std::string AntennaPositioner::GetPolarization()
{
	switch(polarization)
	{
	case PolarizationType::HORIZONTAL:
		return "horizontal";
	case PolarizationType::VERTICAL:
		return "vertical";
	case PolarizationType::UNKNOWN:
	default:
		return "unknown";
	}
}
