/*
 * AntennaPositioner.cpp
 *
 *  Created on: 15/04/2019
 *      Author: new-mauro
 */

#include "AntennaPositioning.h"

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
#endif
	cout << "Enter the initial azimuth angle: ";
	cin >> azimuthAngle;
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
#endif
	}
	else
	{
		cout << "\nRotate the antenna 45° counterclockwise and press the button to continue..." << endl;
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
	cout << "\nChange the antenna polarization and press the button to continue..." << endl;
#ifdef RASPBERRY_PI
	digitalWrite(piPins.LED_POLARIZ, HIGH);
	while( digitalRead(piPins.BUTTON_POLARIZ)==HIGH );
	digitalWrite(piPins.LED_POLARIZ, LOW);
#endif
	polarization = (polarization==Polarization::HORIZONTAL ? Polarization::VERTICAL : Polarization::HORIZONTAL);
	return true;
}

std::string AntennaPositioner::GetPolarization()
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
