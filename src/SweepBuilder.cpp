/*! \file SweepBuilder.cpp
 * 	\brief This file contains the definitions of several methods of the class _SweepBuilder_.
 * 	\author Mauro Diamantino
 */

#include "Spectran.h"

/////////////////////////Definitions of SweepBuilder class' methods///////////////////////

void SweepBuilder::BuildSweep()
{
	sweep.Clear();

	for(auto it=partialSweep.begin(); it!=partialSweep.end(); it++)
	{
		sweep.frequencies.push_back( it->first );
		sweep.values.push_back( it->second );
	}
}

/*!	The method receives a _BandParameters_ structure, where the parameters of the current frequency band are stored, and it uses this
 * 	structure to check if the frequency values are coherent and it corrects the number of sweep points of the structure.
 *
 * 	First, the method sends a command to reset the current sweep, it waits a moment and then it enables the streaming of sweep points.
 * 	Later, the method enters in a loop where each sweep point is captured and inserted in the `std::map` container. That kind of
 * 	container are ordered and unique-key, so automatically the container orders the sweep points, taking into account the frequency,
 * 	and it does not allow to insert two points with the same frequency. When that happens, the container states that and the loop finishes.
 * 	Later, the number of sweep points is checked and stored in the given _BandParameters_ structure, the streaming of sweep points is
 * 	disabled and, finally, the sweep is moved to a _Sweep_ structure, which is more optimum to perform mathematical operations, and this
 * 	structure is returned.
 * 	\param bandParam [in,out] The parameters of the current frequency band.
 */
const Sweep& SweepBuilder::CaptureSweep(BandParameters & bandParam)
{
	bool flagSweepReady=false;
	SweepReply swReply;
	float power;
	std::uint_least64_t frequency;
	std::pair< SweepMap::iterator, bool> mapReply;
	unsigned int errorTimeCount=0, errorFreqCount=0;
	unsigned long samplesCount=0;

	interface.ResetSweep();

	partialSweep.clear();

	/////////
//	cout << "\t\tFrecuencia\t\tPotencia" << endl;
//	cout.setf(std::ios::fixed, std::ios::floatfield);
	////////

	usleep(300000);

	interface.EnableSweep();

	cout << "Capturing a sweep..." << endl;

	while (flagSweepReady==false)
	{
		swReply.Clear();
		try
		{
			interface.Read(swReply);
		}
		catch(rfims_exception & exc)
		{
			cerr << "\nWarning: " << exc.what() << endl;
			if(++errorTimeCount < 3)
				continue;
			else
			{
				interface.DisableSweep();
				rfims_exception exc("the capture of the last sweep was interrupted because the reading of the measurements failed three times.");
				throw(exc);
			}
		}

		frequency=swReply.GetFrequency();
		/////////
//		cout << "\t\t" << std::setprecision(3) << frequency/1e6 << " MHz";
		//////////
		if( frequency<(0.95*bandParam.startFreq) || frequency>(1.05*bandParam.stopFreq) )
		{
			if(++errorFreqCount < 3)
			{
				cerr << "\nWarning: a out-of-range frequency value was captured, the current sweep will be reset and the sweep capture will start again." << endl;

				interface.DisableSweep();

				interface.ResetSweep();

				partialSweep.clear();
				usleep(300000);

				interface.EnableSweep();

				continue;
			}
			else
			{
				interface.DisableSweep();
				rfims_exception exc("too much out-of-range frequency values were captured.");
				throw(exc);
			}
		}

		power=swReply.GetValue();
		///////////
//		cout << "\t\t" << std::setprecision(1) << power << " dBm" << endl;
		//////////
		mapReply = partialSweep.insert( SweepMap::value_type(frequency, power) );
		flagSweepReady = !mapReply.second;
		++samplesCount;

	}

	--samplesCount;
	bandParam.samplePoints = samplesCount;

	interface.DisableSweep();

	BuildSweep();

	return sweep;
}
