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
			RFIDetector * rfiDetectorPt, DataLogger * dataLoggerPt, GPSInterface * gpsInterfacePt,
			AntennaPositioner * antPositionerPt, RFPloter * sweepPloterPt=nullptr,
			RFPloter * gainPloterPt=nullptr, RFPloter * nfPloterPt=nullptr )
	{
		specInterfPtr=specInterfPt;
		specConfiguratorPtr=specConfiguratorPt;
		sweepBuilderPtr=sweepBuilderPt;
		adjusterPtr=adjusterPt;
		calibratorPtr=calibratorPt;
		rfiDetectorPtr=rfiDetectorPt;
		dataLoggerPtr=dataLoggerPt;
		gpsInterfacePtr=gpsInterfacePt;
		antPositionerPtr=antPositionerPt;
		sweepPloterPtr=sweepPloterPt;
		gainPloterPtr=gainPloterPt;
		nfPloterPtr=nfPloterPt;

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
	static RFIDetector * rfiDetectorPtr;
	static DataLogger * dataLoggerPtr;
	static GPSInterface * gpsInterfacePtr;
	static AntennaPositioner * antPositionerPtr;
	static RFPloter * sweepPloterPtr;
	static RFPloter * gainPloterPtr;
	static RFPloter * nfPloterPtr;
	static void ExitSignalHandler(int signum)
	{
		cout << "A signal which terminates the program was captured. Signal number: " << signum << endl;
		specInterfPtr->~SpectranInterface();
		specConfiguratorPtr->~SpectranConfigurator();
		sweepBuilderPtr->~SweepBuilder();
		adjusterPtr->~CurveAdjuster();
		calibratorPtr->~FrontEndCalibrator();
		if(dataLoggerPtr!=nullptr)
			dataLoggerPtr->~DataLogger();
		if(gpsInterfacePtr!=nullptr)
			gpsInterfacePtr->~GPSInterface();
		if(antPositionerPtr!=nullptr)
			antPositionerPtr->~AntennaPositioner();
		if(sweepPloterPtr!=nullptr)
			sweepPloterPtr->~RFPloter();
		if(gainPloterPtr!=nullptr)
			gainPloterPtr->~RFPloter();
		if(nfPloterPtr!=nullptr)
			nfPloterPtr->~RFPloter();

		TurnOffFrontEnd();

		std::exit(signum);
	}
};

#endif /* TOPLEVEL_H_ */
