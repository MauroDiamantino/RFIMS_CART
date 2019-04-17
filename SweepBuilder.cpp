/*
 * SweepBuilder.cpp
 *
 *  Created on: 31/03/2019
 *      Author: new-mauro
 */

#include "Spectran.h"

/////////////////////////Definitions of SweepBuilder class' methods///////////////////////

//! The SweepBuilder class's constructor
SweepBuilder::SweepBuilder(SpectranInterface & interf) : interface(interf)
{
	//samplePoints=sweepTime=0;
	bandParam = {false, 0.0, 0.0, 0.0, 0.0, false, 0, 0};
	sweep.type = "sweep";
	sweep.index = 0;
}

//! A method which build the definite sweep object, which is of type *FreqValueSet* (struct), from the partial sweep which is map container.
void SweepBuilder::BuildSweep()
{
	sweep.Clear();

	for(auto it=partialSweep.begin(); it!=partialSweep.end(); it++)
	{
		sweep.frequencies.push_back( it->first );
		sweep.values.push_back( it->second );
	}
}

void SweepBuilder::SoundNewSweep()
{
	Command comm(Command::SETSTPVAR, SpecVariable::STDTONE, 100.0);
	Reply reply(Reply::SETSTPVAR);
	try
	{
		interface.Write(comm);
		interface.Read(reply);
		if(reply.IsRight()!=true)
		{
			CustomException exc("The reply to the command to make the sound which indicates the start of a new sweep was wrong.");
			throw(exc);
		}
	}
	catch(CustomException & exc)
	{
		cerr << "Warning: " << exc.what() << endl;
		cerr << "The sound to show that a new sweep will be captured could not be made." << endl;
	}
}

//! The aim of this method is to capture one sweep from the Spectran Interface and return it.
const FreqValueSet& SweepBuilder::CaptureSweep()
{
	bool flagSweepReady=false;
	SweepReply swReply;
	float frequency, power;
	std::pair< SweepMap::iterator, bool> mapReply;
	__useconds_t deltaTime = 1000*(bandParam.sweepTime/bandParam.samplePoints); //theoretical time interval between sweep points
	unsigned int errorTimeCount=0, errorFreqCount=0;

	SoundNewSweep();

	//Reseting the current sweep
	Command comm(Command::SETSTPVAR, SpecVariable::USBSWPRST, 1.0);
	Reply reply(Reply::SETSTPVAR);
	try
	{
		interface.Write(comm);
		interface.Read(reply);
		if(!reply.IsRight())
			throw( CustomException("A wrong reply was received.") );
	}
	catch(CustomException & exc)
	{
		exc.Append("\nThe command to reset the current sweep failed.");
		throw;
	}

	partialSweep.clear();

	interface.EnableSweep();

	usleep(10*deltaTime);

	while (flagSweepReady==false)
	{
		usleep( deltaTime );

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
				deltaTime *= 2;
				cout << "The time interval (delta time) which is waited before capture a sweep point has been doubled. The new value is " << deltaTime << endl;
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
		if( frequency<bandParam.startFreq || frequency>bandParam.stopFreq )
		{
			if(++errorFreqCount < 3)
			{
				cerr << "Warning: a out-of-range frequency value was captured, the current sweep will be reset and the sweep capture will start again." << endl;

				interface.DisableSweep();

				//Reseting the current sweep
				Command comm(Command::SETSTPVAR, SpecVariable::USBSWPRST, 1.0);
				Reply reply(Reply::SETSTPVAR);
				try
				{
					interface.Write(comm);
					interface.Read(reply);
					if(!reply.IsRight())
						throw( CustomException("A wrong reply was received.") );
				}
				catch(CustomException & exc)
				{
					exc.Append("\nThe command to reset the current sweep failed.");
					throw;
				}

				//Purging the input buffer
				//interface.Purge();
				partialSweep.clear();
				usleep(500000);

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
		mapReply = partialSweep.insert( SweepMap::value_type(frequency, power) );
		flagSweepReady = !mapReply.second;
	}

	interface.DisableSweep();

	BuildSweep();

	return sweep;
}
