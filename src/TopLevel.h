/*!	\file TopLevel.h
 * 	\brief This header file includes the rest of the header files and the class of the signal handler is declared here.
 *
 * 	This header file simplifies the include in the main.cpp file. The declaration of the class _SignalHandler_ must be
 * 	put in an high-level header file because this class must know the declarations of almost all the classes of the
 * 	software.
 * 	\author Mauro Diamantino
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

//! The class _SignalHandler_ is intended to handle the interprocess signals (IPC) which terminates the software.
/*! The signals which are handled by this class are SIGINT and SIGTERM. When one these signals arrived, the destructors
 * of all the high-level objects are called, to ensure a clean closing of the software and an adequate turning off the
 * RF front end.
 */
class SignalHandler
{
public:
	//! This method is intended to set the pointers to the high-level objects and to set handler functions.
	void SetupSignalHandler(SpectranInterface * specInterfPt, SpectranConfigurator * specConfiguratorPt,
			SweepBuilder * sweepBuilderPt, CurveAdjuster * adjusterPt, FrontEndCalibrator * calibratorPt,
			RFIDetector * rfiDetectorPt=nullptr, DataLogger * dataLoggerPt=nullptr, GPSInterface * gpsInterfacePt=nullptr,
			AntennaPositioner * antPositionerPt=nullptr, RFPlotter * sweepPlotterPt=nullptr,
			RFPlotter * gainPlotterPt=nullptr, RFPlotter * nfPlotterPt=nullptr )
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
		sweepPlotterPtr=sweepPlotterPt;
		gainPlotterPtr=gainPlotterPt;
		nfPlotterPtr=nfPlotterPt;

		if( signal(int(SIGINT), (__sighandler_t) SignalHandler::ExitSignalHandler)==SIG_ERR )
			throw rfims_exception("The SIGINT signal handler could not be set.");

		if( signal(int(SIGTERM),  (__sighandler_t) SignalHandler::ExitSignalHandler)==SIG_ERR )
			throw rfims_exception("The SIGTERM signal handler could not be set.");
	}
	//Static methods and objects
	static SpectranInterface * specInterfPtr; //!< A pointer to the _SpectranInterface_ object.
	static SpectranConfigurator * specConfiguratorPtr; //!< A pointer to the _SpectranConfigurator_ object.
	static SweepBuilder * sweepBuilderPtr; //!< A pointer to the _SweepBuilder_ object.
	static CurveAdjuster * adjusterPtr; //!< A pointer to the _CurveAdjuster_ object.
	static FrontEndCalibrator * calibratorPtr; //!< A pointer to the _FrontEndCalibrator_ object.
	static RFIDetector * rfiDetectorPtr; //!< A pointer to the _RFIDetector_ object.
	static DataLogger * dataLoggerPtr; //!< A pointer to the _DataLogger_ object.
	static GPSInterface * gpsInterfacePtr; //!< A pointer to the _GPSInterface_ object.
	static AntennaPositioner * antPositionerPtr; //!< A pointer to the _AntennaPositioner_ object.
	static RFPlotter * sweepPlotterPtr; //!< A pointer to the _RFPlotter_ object which is responsible for the plotting of the last captured sweep.
	static RFPlotter * gainPlotterPtr; //!< A pointer to the _RFPlotter_ object which is responsible for the plotting of the last estimated gain curve.
	static RFPlotter * nfPlotterPtr; //!< A pointer to the _RFPlotter_ object which is responsible for the plotting of the last estimated noise figure curve.

	//! This function is executed when a SIGINT or a SIGTERM signal arrives.
	/*! The function calls the destructor of almost all the high-level objects (defined in the main function)
	 * to ensure a clean closing of the software and an adequate turning off the RF front end.
	 * \param [in] signum This parameters must always be present and it states the number of the arrived signal.
	 */
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
		if(sweepPlotterPtr!=nullptr)
			sweepPlotterPtr->~RFPlotter();
		if(gainPlotterPtr!=nullptr)
			gainPlotterPtr->~RFPlotter();
		if(nfPlotterPtr!=nullptr)
			nfPlotterPtr->~RFPlotter();

		TurnOffFrontEnd();

		if( !timer.is_stopped() )
		{
			//Showing the elapsed time since the beginning
			timer.stop();
			boost::timer::cpu_times times = timer.elapsed();
			boost::posix_time::time_duration td = boost::posix_time::microseconds(times.wall/1000);
			cout << "\nThe elapsed time since the beginning is: " << boost::posix_time::to_simple_string(td) << endl;
		}

		std::exit(signum);
	}
};

#endif /* TOPLEVEL_H_ */
