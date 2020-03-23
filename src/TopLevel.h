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

#include <cstddef>
#include <signal.h>
#include <boost/timer/timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>


//#///////////////Constants///////////////////
//! The number of azimuth positions where the system will perform sweeps.
const unsigned int DEF_NUM_AZIM_POS = 6;


/////////////////////////DECLARATIONS OF GLOBAL VARIABLES///////////////////////

//Declarations of global variables which are defined in TopLevel.cpp
extern bool flagCalEnabled;
extern bool flagPlot;
extern bool flagInfiniteLoop;
extern bool flagRFI;
extern bool flagUpload;
extern unsigned int numOfMeasCycles;
extern RFI::ThresholdsNorm rfiNorm;
extern unsigned int numOfAzimPos;
extern boost::timer::cpu_timer timer;

////////////////////////////////////////////////////////////////


///////////////////GLOBAL FUNCTIONS/////////////////////////

//! This function prints a message, in the `stdout`, with a software description and the descriptions of its arguments.
void PrintHelp();

//! This function process the software's arguments, which define the behavior of this one.
bool ProcessMainArguments(int argc, char * argv[]);

//! A function which extracts data from a timer and returns it as a string in a human-readable format.
std::string GetTimeAsString(boost::timer::cpu_timer & timer);

///////////////////////////////////////////////////////////////


//! The class _SignalHandler_ is intended to handle the interprocess signals (IPC) which terminates the software.
/*! The signals which are handled by this class are SIGINT and SIGTERM. When one these signals arrived, the destructors
 * of all the high-level objects are called, to ensure a clean closing of the software and an adequate turning off the
 * RF front end.
 */
class SignalHandler
{
public:
	//! The SignalHandler constructor which set up the function which handles the signals.
	SignalHandler()
	{
		if( signal(int(SIGINT), (__sighandler_t) SignalHandler::ExitSignalHandler)==SIG_ERR )
			cerr << "The SIGINT signal handler could not be set." << endl;

		if( signal(int(SIGTERM),  (__sighandler_t) SignalHandler::ExitSignalHandler)==SIG_ERR )
			cerr << "The SIGTERM signal handler could not be set." << endl;
	}
	//! This method is intended to set the pointers to the high-level objects and to set handler functions.
	void SetAllPointers(SpectranInterface * specInterfPt, SpectranConfigurator * specConfiguratorPt, SweepBuilder * sweepBuilderPt,
			CurveAdjuster * adjusterPt, FrontEndCalibrator * calibratorPt, RFIDetector * rfiDetectorPt, DataLogger * dataLoggerPt,
			GPSInterface * gpsInterfacePt, AntennaPositioner * antPositionerPt, RFPlotter * sweepPlotterPt,
			RFPlotter * gainPlotterPt, RFPlotter * nfPlotterPt)
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
	}

	//////////////Static methods and objects////////////////

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
		cout << "\nA signal which terminates the program was captured. Signal number: " << signum << endl;

		if(specInterfPtr!=nullptr)
			specInterfPtr->~SpectranInterface();
		if(specConfiguratorPtr!=nullptr)
			specConfiguratorPtr->~SpectranConfigurator();
		if(sweepBuilderPtr!=nullptr)
			sweepBuilderPtr->~SweepBuilder();
		if(adjusterPtr!=nullptr)
			adjusterPtr->~CurveAdjuster();
		if(calibratorPtr!=nullptr)
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
		TurnOffLeds();

		if( !timer.is_stopped() )
			cout << "\nThe elapsed time since the beginning is: " << GetTimeAsString(timer) << endl;

		cout << "\nExiting from the software..." << endl;

		std::exit(signum);
	}
};

#endif /* TOPLEVEL_H_ */
