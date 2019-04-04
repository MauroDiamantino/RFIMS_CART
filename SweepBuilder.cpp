/*
 * SweepBuilder.cpp
 *
 *  Created on: 31/03/2019
 *      Author: new-mauro
 */

#include "Spectran.h"

/////////////////////////Definitions of FreqValueSet struct's methods////////////////////

//! The overloading of the operation "sum", which is a friend function of the struct FreqValueSet
/*! This function takes two arguments of type FreqValueSet and returns a different object of the same type
 * whose "values" vector will have the sum of the corresponding elements of the "values" vectors of the arguments.
 * Before performing the sum, the function checks if the frequencies match and if the "values" vectors have the same
 * sizes as the "frequencies" vectors. The other attributes of the returned object (type, index and timestamp) will
 * take the contents of the corresponding attributes of the left-hand argument.
 */
FreqValueSet operator+(const FreqValueSet & lhs, const FreqValueSet & rhs)
{
	if(lhs.frequencies != rhs.frequencies)
	{
		CustomException exc("The sum could not be performed because the frequencies do not match");
		throw(exc);
	}

	if( lhs.frequencies.size()!=lhs.values.size() && rhs.frequencies.size()!=rhs.values.size() )
	{
		CustomException exc("The sum could not be performed because one (or both) \"values\" vector has a different size with respect to the \"frequencies\" vector.");
		throw(exc);
	}

	FreqValueSet result;

	auto it1=lhs.values.begin();
	auto it2=rhs.values.begin();
	auto it3=lhs.frequencies.begin();
	for( ; it1!=lhs.values.end(); it1++, it2++, it3++)
	{
		result.values.push_back( *it1 + *it2 );
		result.frequencies.push_back( *it3 );
	}

	result.type = lhs.type;
	result.index = lhs.index;
	result.timestamp = lhs.timestamp;

	return result;
}

//! The overloading of the assignment operator.
const FreqValueSet& FreqValueSet::operator=(const FreqValueSet & freqSet)
{
	type = freqSet.type;
	index = freqSet.index;
	values = freqSet.values;
	frequencies = freqSet.frequencies;
	timestamp = freqSet.timestamp;

	return *this;
}

//! The overloading of the += operator.
const FreqValueSet& FreqValueSet::operator+=(const FreqValueSet& rhs)
{
	if(rhs.frequencies != frequencies)
	{
		CustomException exc("The sum could not be performed because the frequencies do not match");
		throw(exc);
	}

	if( frequencies.size()!=values.size() && rhs.frequencies.size()!=rhs.values.size() )
	{
		CustomException exc("The sum could not be performed because one (or both) \"values\" vector has a different size with respect to the \"frequencies\" vector.");
		throw(exc);
	}

	auto it1=values.begin();
	auto it2=rhs.values.begin();
	for( ; it1!=values.end(); it1++, it2++)
		*it1 += *it2;

	return *this;
}

void FreqValueSet::PushBack(const FreqValueSet& freqValueSet)
{
	values.insert(values.end(), freqValueSet.values.begin(), freqValueSet.values.end());
	frequencies.insert(frequencies.end(), freqValueSet.frequencies.begin(), freqValueSet.frequencies.end());
}

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
const FreqValueSet& SweepBuilder::CaptureOneSweep()
{
	bool flagSweepReady=false;
	SweepReply swReply;
	float frequency, power;
	pair< SweepMap::iterator, bool> mapReply;
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
