/*
 * AntennaPositioner.cpp
 *
 *  Created on: 15/04/2019
 *      Author: new-mauro
 */

#include "AntennaPositioning.h"

AntennaPositioner::AntennaPositioner()
{
	//Initializing the Wiring Pi library
	wiringPiSetup();

	//Setting the Raspberry Pi pins
	pinMode(PIN_LED_AZIMUTHAL, OUTPUT);
	pinMode(PIN_SW_AZIMUTHAL, INPUT);
	pullUpDnControl(PIN_SW_AZIMUTHAL, PUD_UP);
	pinMode(PIN_LED_POLARIZACION, OUTPUT);
	pinMode(PIN_SW_POLARIZATION, INPUT);
	pullUpDnControl(PIN_SW_POLARIZATION, PUD_UP);

	//Setting the initial values
	azimuthAngle=0.0;
	polarization=PolarizationType::UNKNOWN;
	positionIndex=255;
	digitalWrite(PIN_LED_AZIMUTHAL, LOW);
	digitalWrite(PIN_LED_POLARIZACION, LOW);
}

bool AntennaPositioner::Initialize()
{
	return true;
}

bool AntennaPositioner::NextAzimPosition()
{
	digitalWrite(PIN_LED_AZIMUTHAL, HIGH);
	while( digitalRead(PIN_SW_AZIMUTHAL)==HIGH );
	digitalWrite(PIN_LED_AZIMUTHAL, LOW);
	if( ++positionIndex >= NUM_OF_POSITIONS )
	{
		positionIndex=0;
		azimuthAngle=0.0;
	}
	else
	{
		azimuthAngle+=45.0;
	}
	return true;
}

bool AntennaPositioner::ChangePolarization()
{
	digitalWrite(PIN_LED_POLARIZACION, HIGH);
	while( digitalRead(PIN_SW_POLARIZATION)==HIGH );
	digitalWrite(PIN_LED_POLARIZACION, LOW);
	polarization = polarization==PolarizationType::HORIZONTAL ? PolarizationType::VERTICAL : PolarizationType::HORIZONTAL;
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
