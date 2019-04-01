/*
 * TestbenchSpectranConfigurator.cpp
 *
 *  Created on: 31/03/2019
 *      Author: new-mauro
 */

#include "SpectranInterface.h"

int main()
{
	SpectranInterface specInterface;

	specInterface.Initialize();

	SpectranConfigurator specConfigurator(specInterface);

	try
	{
		for(auto i=0; i<2; i++)
			specConfigurator.LoadParameters();
	}
	catch(exception & exc)
	{
		cerr << exc.what() << endl;
		exit(EXIT_FAILURE);
	}
}



