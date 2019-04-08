/*
 * TestbenchDataLogger.cpp
 *
 *  Created on: 08/04/2019
 *      Author: new-mauro
 */

#include "SweepProcessing.h"

int main()
{
	FreqValueSet sweep;
	DataLogger logger;

	for(uint8_t i=0; i<10; i++)
	{
		sweep.frequencies.push_back(1001e6 + i*1e6);
		sweep.values.push_back(-60.1 + i*10.0);
	}

	logger.SetSweep(sweep);
	logger.SetAntennaData(45.0, "horizontal");
	logger.SaveData();

	sweep.Clear();

	for(uint8_t i=0; i<10; i++)
	{
		sweep.frequencies.push_back(1011e6 + i*1e6);
		sweep.values.push_back(-70.1 + i*10.0);
	}

	logger.SetSweep(sweep);
	logger.SetAntennaData(45.0, "vertical");
	logger.SaveData();

	return 0;
}
