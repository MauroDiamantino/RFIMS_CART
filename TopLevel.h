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
#include <boost/timer/timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

extern boost::timer::cpu_timer timer;

class SignalHandler
{
public:
	void SetupSignalHandler(SpectranInterface * specInterfPt, SpectranConfigurator * specConfiguratorPt,
			SweepBuilder * sweepBuilderPt, CurveAdjuster * adjusterPt, FrontEndCalibrator * calibratorPt,
			RFIDetector * rfiDetectorPt=nullptr, DataLogger * dataLoggerPt=nullptr, GPSInterface * gpsInterfacePt=nullptr,
			AntennaPositioner * antPositionerPt=nullptr, RFPloter * sweepPloterPt=nullptr,
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
		if(rfiDetectorPtr!=nullptr)
			rfiDetectorPtr->~RFIDetector();
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

		if( !timer.is_stopped() )
		{
			//Showing the elapsed time since the beginning
			timer.stop();
			boost::timer::cpu_times times = timer.elapsed();
			//double hours = double(times.wall)/(1e9*3600.0);
			//cout << "\nThe elapsed time since the beginning is: " << hours << " hours" << endl;
			boost::posix_time::time_duration td = boost::posix_time::microseconds(times.wall/1000);
			cout << "\nThe elapsed time since the beginning is: " << boost::posix_time::to_simple_string(td) << endl;
		}

		std::exit(signum);
	}
};

#endif /* TOPLEVEL_H_ */
