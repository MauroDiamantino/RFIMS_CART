/*
 * TopLevel.h
 *
 *  Created on: 22/05/2019
 *      Author: new-mauro
 */

#ifndef TOPLEVEL_H_
#define TOPLEVEL_H_

#include "Spectran.h"
#include "SweepProcessing.h"
#include "AntennaPositioning.h"
#include <signal.h>

class SignalHandler
{
public:
	void SetupSignalHandler(SpectranInterface * specInterfPt, SpectranConfigurator * specConfiguratorPt,
			SweepBuilder * sweepBuilderPt, CurveAdjuster * adjusterPt, FrontEndCalibrator * calibratorPt,
			DataLogger * dataLoggerPt, RFPloter * sweepPloterPt, RFPloter * gainPloterPt, RFPloter * nfPloterPt,
			AntennaPositioner * antPositionerPt)
	{
		specInterfPtr=specInterfPt;
		specConfiguratorPtr=specConfiguratorPt;
		sweepBuilderPtr=sweepBuilderPt;
		adjusterPtr=adjusterPt;
		calibratorPtr=calibratorPt;
		dataLoggerPtr=dataLoggerPt;
		sweepPloterPtr=sweepPloterPt;
		gainPloterPtr=gainPloterPt;
		nfPloterPtr=nfPloterPt;
		antPositionerPtr=antPositionerPt;

		if( signal(int(SIGINT), (__sighandler_t) SignalHandler::ExitSignalHandler)==SIG_ERR )
		{
			CustomException exc("The SIGINT signal handler could not be set.");
			throw(exc);
		}

		if( signal(int(SIGTERM),  (__sighandler_t) SignalHandler::ExitSignalHandler)==SIG_ERR )
		{
			CustomException exc("The SIGTERM signal handler could not be set.");
			throw(exc);
		}
	}
	//Static methods and objects
	static SpectranInterface * specInterfPtr;
	static SpectranConfigurator * specConfiguratorPtr;
	static SweepBuilder * sweepBuilderPtr;
	static CurveAdjuster * adjusterPtr;
	static FrontEndCalibrator * calibratorPtr;
	static DataLogger * dataLoggerPtr;
	static RFPloter * sweepPloterPtr;
	static RFPloter * gainPloterPtr;
	static RFPloter * nfPloterPtr;
	static AntennaPositioner * antPositionerPtr;
	static void ExitSignalHandler(int signum)
	{
		cout << "\nA signal which terminates the program was captured. Signal number: " << signum << endl;
		specInterfPtr->~SpectranInterface();
		specConfiguratorPtr->~SpectranConfigurator();
		sweepBuilderPtr->~SweepBuilder();
		adjusterPtr->~CurveAdjuster();
		calibratorPtr->~FrontEndCalibrator();
		exit(signum);
	}
};

#endif /* TOPLEVEL_H_ */
