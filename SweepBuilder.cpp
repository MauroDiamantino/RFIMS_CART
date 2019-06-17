/*
 * SweepBuilder.cpp
 *
 *  Created on: 31/03/2019
 *      Author: new-mauro
 */

#include "Spectran.h"

/////////////////////////Definitions of SweepBuilder class' methods///////////////////////

//! A method which build the definite sweep object, which is of type *FreqValues* (struct), from the partial sweep which is map container.
void SweepBuilder::BuildSweep()
{
	sweep.Clear();

	for(auto it=partialSweep.begin(); it!=partialSweep.end(); it++)
	{
		sweep.frequencies.push_back( it->first );
		sweep.values.push_back( it->second );
	}
}

//void SweepBuilder::SoundNewSweep()
//{
//	Command comm(Command::SETSTPVAR, SpecVariable::STDTONE, 100.0);
//	Reply reply(Reply::SETSTPVAR);
//	try
//	{
//		interface.Write(comm);
//		interface.Read(reply);
//		if(reply.IsRight()!=true)
//		{
//			CustomException exc("The reply to the command to make the sound which indicates the start of a new sweep was wrong.");
//			throw(exc);
//		}
//	}
//	catch(CustomException & exc)
//	{
//		cerr << "Warning: " << exc.what() << endl;
//		cerr << "The sound to show that a new sweep will be captured could not be made." << endl;
//	}
//}


//! The aim of this method is to capture one sweep from the Spectran Interface and return it.
const Sweep& SweepBuilder::CaptureSweep(BandParameters & bandParam)
{
	bool flagSweepReady=false;
	SweepReply swReply;
	float power;
	//float frequency;
	std::uint_least64_t frequency;
	std::pair< SweepMap::iterator, bool> mapReply;
//	__useconds_t deltaTime = 1000*(bandParam.sweepTime/bandParam.samplePoints); //theoretical time interval between sweep points
	unsigned int errorTimeCount=0, errorFreqCount=0;
	unsigned long samplesCount=0;

	interface.ResetSweep();

	partialSweep.clear();

//	cout << "\t\tFrecuencia\t\tPotencia" << endl;
//	cout.setf(std::ios::fixed, std::ios::floatfield);

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
		catch(CustomException & exc)
		{
			cerr << "Warning: " << exc.what() << endl;
			if(++errorTimeCount < 3)
			{
//				deltaTime *= 2;
//				cout << "The time interval (delta time) which is waited before capture a sweep point has been doubled. The new value is " << deltaTime << endl;
				continue;
			}
			else
			{
				interface.DisableSweep();
				CustomException exc("The capture of one sweep failed because the reading of measurements failed three times.");
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
				CustomException exc("Too much out-of-range frequency values were captured.");
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

		//usleep( deltaTime );
	}

	--samplesCount;
	bandParam.samplePoints = samplesCount;

	interface.DisableSweep();

	BuildSweep();

	return sweep;
}
