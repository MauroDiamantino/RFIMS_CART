/*
 * TestbenchDataLogger.cpp
 *
 *  Created on: 08/04/2019
 *      Author: new-mauro
 */

#include "SweepProcessing.h"

int main() throw(std::exception)
{
	FreqValueSet sweep;
	DataLogger logger;

	for(uint8_t i=0; i<10; i++)
	{
		sweep.frequencies.push_back(1e9 + i*1e6);
		sweep.values.push_back(-60.1 + i*1.0);
	}
	sweep.timestamp.date="08-04-2019";
	sweep.timestamp.time="19:08:55";

	logger.SetSweep(sweep);
	logger.SetAntennaData(45.5, "horizontal");
	logger.SaveData();

	sweep.values.clear();

	for(uint8_t i=0; i<10; i++)
		sweep.values.push_back(-70.1 + i*1.0);

	sweep.timestamp.date="08-04-2019";
	sweep.timestamp.time="19:10:55";

	logger.SetSweep(sweep);
	logger.SetAntennaData(45.5, "vertical");
	logger.SaveData();

	return 0;
}
