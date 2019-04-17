/*
 * SpectranConfigurator.cpp
 *
 *  Created on: 28/03/2019
 *      Author: new-mauro
 */

#include "Spectran.h"

//! The only one constructor of class SpectranConfigurator
SpectranConfigurator::SpectranConfigurator(SpectranInterface& interf) : interface(interf)
{
	bandIndex=10000;
	lastWriteTimes[0]=lastWriteTimes[1]=0;
}

//! The destructor of class SpectranConfigurator
/*! The destructor just makes sure the input file stream (ifs) is closed.
 */
SpectranConfigurator::~SpectranConfigurator()
{
	ifs.close();
}

//! This method loads the Spectran's parameters from the corresponding files.
/*! The method opens the files with the Spectran's parameters, checks if they have been modified since the last
 * loading and if that is true reloads the parameters. The method returns a boolean value to indicate if the fixed
 * parameters have been updated so the initial configuration should be repeated.
 */
bool SpectranConfigurator::LoadFixedParameters()
{
	std::string paramName, line;
	char endChar;
	size_t definitionEndPos, endCharPos, equalPos;
	std::string pathAndName;

	pathAndName = FILES_PATH + "fixedparameters.txt";
	//Checking if the fixed parameters have been modified
	if( lastWriteTimes[0] < boost::filesystem::last_write_time(pathAndName) )
	{
		lastWriteTimes[0] = boost::filesystem::last_write_time(pathAndName);

		//Opening the files with the fixed parameters and loading these ones
		ifs.open(pathAndName);
		do
		{
			line.clear();
			getline(ifs, line);
			//The next instruction determines if there are comments and which position they start in.
			//If there are no comments, the position returned by the method find() will be std::string::npos.
			definitionEndPos = line.find("//");

			if( ( endCharPos = line.find(',') ) < definitionEndPos ||
					( endCharPos = line.find(';') ) < definitionEndPos )
			{
				//The definition has an end character (a comma or a semicolon) before the comment (if this exists)
				equalPos = line.find('=');
				paramName = line.substr(0, equalPos);
				endChar = line.at(endCharPos);
			}
			else
			{
				//The definition is incomplete
				std::string str = "The parameter's definition \"" + line + "\" does not have a comma or semicolon in the end.";
				CustomException exc(str);
				throw(exc);
			}

			//The parameter's name is transformed to lower case
			boost::algorithm::to_lower(paramName);

			//The parameter's value is extracted and transformed to lower case
			std::string valueString = line.substr(equalPos+1, endCharPos-equalPos-1);
			boost::algorithm::to_lower(valueString);

			std::istringstream iss;
			if(paramName=="attenuator factor")
			{
				if(valueString=="auto")
				{
					fixedParam.attenFactor = -10;
				}
				else
				{
					//The value is numerical
					iss.str(valueString);
					iss >> fixedParam.attenFactor;
					if(fixedParam.attenFactor<0 || fixedParam.attenFactor>30)
					{
						std::string str = "The given value to configure variable " + paramName + " is invalid.";
						CustomException exc(str);
						throw(exc);
					}
				}
			}
			else if(paramName=="display unit")
			{
				if(valueString=="dbm")
				{
					fixedParam.displayUnit=0;
				}
				else if(valueString=="dbuv")
				{
					fixedParam.displayUnit=1;
				}
				else if(valueString=="v/m")
				{
					fixedParam.displayUnit=2;
				}
				else if(valueString=="a/m")
				{
					fixedParam.displayUnit=3;
				}
				else
				{
					std::string str = "The given value to configure variable " + paramName + " is invalid.";
					CustomException exc(str);
					throw(exc);
				}
			}
			else if(paramName=="demodulator mode")
			{
				if(valueString=="off")
				{
					fixedParam.demodMode=0;
				}
				else if(valueString=="am")
				{
					fixedParam.demodMode=1;
				}
				else if(valueString=="fm")
				{
					fixedParam.demodMode=3;
				}
				else
				{
					std::string str = "The given value to configure variable " + paramName + " is invalid.";
					CustomException exc(str);
					throw(exc);
				}
			}
			else if(paramName=="antenna type")
			{
				if(valueString=="hl7025")
				{
					fixedParam.antennaType=0;
				}
				else if(valueString=="hl7040")
				{
					fixedParam.antennaType=1;
				}
				else if(valueString=="hl7060")
				{
					fixedParam.antennaType=2;
				}
				else if(valueString=="hl6080")
				{
					fixedParam.antennaType=3;
				}
				else if(valueString=="h60100")
				{
					fixedParam.antennaType=4;
				}
				else
				{
					std::string str = "The given value to configure variable " + paramName + " is invalid.";
					CustomException exc(str);
					throw(exc);
				}
			}
			else if(paramName=="cable type")
			{
				iss.str(valueString);
				iss >> fixedParam.cableType;
				if(fixedParam.cableType!=-1 && fixedParam.cableType!=0)
				{
					std::string str = "The given value to configure variable " + paramName + " is invalid.";
					CustomException exc(str);
					throw(exc);
				}
			}
			else if(paramName=="receiver configuration")
			{
				iss.str(valueString);
				iss >> fixedParam.recvConf;
				if(fixedParam.recvConf!=0 && fixedParam.recvConf!=1)
				{
					std::string str = "The given value to configure variable " + paramName + " is invalid.";
					CustomException exc(str);
					throw(exc);
				}
			}
			else if(paramName=="internal preamplifier")
			{
				if(valueString=="off")
				{
					fixedParam.internPreamp=false;
				}
				else if(valueString=="on")
				{
					fixedParam.internPreamp=true;
				}
				else
				{
					std::string str = "The given value to configure variable " + paramName + " is invalid.";
					CustomException exc(str);
					throw(exc);
				}
//				iss.str(valueString);
//				iss >> fixedParam.internPreamp;
//				if(fixedParam.internPreamp!=0 && fixedParam.internPreamp!=1)
//				{
//					std::string str = "The given value to configure variable " + paramName + " is invalid.";
//					CustomException exc(str);
//					throw(exc);
//				}
			}
			else if(paramName=="sweep delay accuracy")
			{
				if(valueString=="off")
				{
					fixedParam.sweepDelayAcc=false;
				}
				else if(valueString=="on")
				{
					fixedParam.sweepDelayAcc=true;
				}
				else
				{
					std::string str = "The given value to configure variable " + paramName + " is invalid.";
					CustomException exc(str);
					throw(exc);
				}
//				iss.str(valueString);
//				iss >> fixedParam.sweepDelayAcc;
//				if(fixedParam.sweepDelayAcc!=0 && fixedParam.recvConf!=1)
//				{
//					std::string str = "The given value to configure variable " + paramName + " is invalid.";
//					CustomException exc(str);
//					throw(exc);
//				}
			}
			else if(paramName=="peak level audio tone")
			{
				if(valueString=="off")
				{
					fixedParam.peakLevelAudioTone=false;
				}
				else if(valueString=="on")
				{
					fixedParam.peakLevelAudioTone=true;
				}
				else
				{
					std::string str = "The given value to configure variable " + paramName + " is invalid.";
					CustomException exc(str);
					throw(exc);
				}
//				iss.str(valueString);
//				iss >> fixedParam.peakLevelAudioTone;
//				if(fixedParam.peakLevelAudioTone!=0 && fixedParam.peakLevelAudioTone!=1)
//				{
//					std::string str = "The given value to configure variable " + paramName + " is invalid.";
//					CustomException exc(str);
//					throw(exc);
//				}
			}
			else if(paramName=="back bb detector")
			{
				if(valueString=="off")
				{
					fixedParam.backBBDetector=false;
				}
				else if(valueString=="on")
				{
					fixedParam.backBBDetector=true;
				}
				else
				{
					std::string str = "The given value to configure variable " + paramName + " is invalid.";
					CustomException exc(str);
					throw(exc);
				}
//				iss.str(valueString);
//				iss >> fixedParam.backBBDetector;
//				if(fixedParam.backBBDetector!=0 && fixedParam.backBBDetector!=1)
//				{
//					std::string str = "The given value to configure variable " + paramName + " is invalid.";
//					CustomException exc(str);
//					throw(exc);
//				}
			}
			else if(paramName=="speaker volume")
			{
				iss.str(valueString);
				iss >> fixedParam.speakerVol;
				if(fixedParam.speakerVol<0.0 || fixedParam.speakerVol>1.0)
				{
					std::string str = "The given value to configure variable " + paramName + " is invalid.";
					CustomException exc(str);
					throw(exc);
				}
			}
			else
			{
				std::string str = "The parameter " + paramName + " is not a valid parameter.";
				CustomException exc(str);
				throw(exc);
			}
		}while( endChar!=';' );

		ifs.close();
		return true;
	}

	return false;
}

bool SpectranConfigurator::LoadBandsParameters()
{
	std::string paramName, line;
	char endChar;
	size_t definitionEndPos, endCharPos, equalPos;
	std::string pathAndName;

	pathAndName = FILES_PATH + "freqbands.txt";
	if( lastWriteTimes[1] < boost::filesystem::last_write_time(pathAndName) )
	{
		lastWriteTimes[1] = boost::filesystem::last_write_time(pathAndName);
		//Opening the files with the frequency bands's parameters and loading these ones
		ifs.open(pathAndName);

		BandParameters bandParam = {false, 0.0, 0.0, 0.0, 0.0, 0, 0, 0};
		do
		{
			line.clear();
			getline(ifs, line);
			//The next instruction determines if there are comments and which position they start in.
			//If there are no comments, the position returned by the method find() will be std::string::npos.
			definitionEndPos = line.find("//");

			if( ( endCharPos = line.find(',') ) < definitionEndPos ||
					( endCharPos = line.find(';') ) < definitionEndPos )
			{
				//The definition has an end character (a comma or a semicolon) before the comment (if this exists)
				equalPos = line.find('=');
				paramName = line.substr(0, equalPos);
				endChar = line.at(endCharPos);
			}
			else
			{
				//The definition is incomplete
				std::string str = "The parameter's definition \"" + line + "\" does not have a comma or semicolon in the end.";
				CustomException exc(str);
				throw(exc);
			}

			//The parameter's name is transformed to lower case
			boost::algorithm::to_lower(paramName);

			//The parameter's value is extracted and transformed to lower case
			std::string valueString = line.substr(equalPos+1, endCharPos-equalPos-1);
			boost::algorithm::to_lower(valueString);

			std::istringstream iss;
			if(paramName=="band index")
			{}
			else if(paramName=="enabled")
			{
				if(valueString=="y")
				{
					bandParam.flagEnable=true;
				}
				else if(valueString=="n")
				{
					bandParam.flagEnable=false;
				}
				else
				{
					std::ostringstream oss;
					oss << "It is not clear if the band " << (bandsParam.size()+1) << " is enabled or not.";
					CustomException exc( oss.str() );
					throw(exc);
				}
			}
			else if(paramName=="fstart")
			{
				iss.str(valueString);
				iss >> bandParam.startFreq;
				if(bandParam.startFreq<1e6 || bandParam.startFreq>9.4e9)
				{
					std::string str = "The given value to configure variable " + paramName + " is out of range.";
					CustomException exc(str);
					throw(exc);
				}
			}
			else if(paramName=="fstop")
			{
				iss.str(valueString);
				iss >> bandParam.stopFreq;
				if(bandParam.stopFreq<1e6 || bandParam.stopFreq>9.4e9)
				{
					std::string str = "The given value to configure variable " + paramName + " is out of range.";
					CustomException exc(str);
					throw(exc);
				}
			}
			else if(paramName=="rbw")
			{
				if(valueString=="full")
				{
					bandParam.rbw=50e6;
				}
				else
				{
					//The value is numerical
					iss.str(valueString);
					iss >> bandParam.rbw;
					if(bandParam.rbw!=3e6 && bandParam.rbw!=1e6 && bandParam.rbw!=300e3 && bandParam.rbw!=100e3 &&
							bandParam.rbw!=30e3 && bandParam.rbw!=10e3 && bandParam.rbw!=3e3 && bandParam.rbw!=1e3 &&
							bandParam.rbw!=120e3 && bandParam.rbw!=9e3 && bandParam.rbw!=200.0 && bandParam.rbw!=5e6 &&
							bandParam.rbw!=200e3 && bandParam.rbw!=1.5e6)
					{
						std::string str = "The given value to configure variable " + paramName + " is invalid.";
						CustomException exc(str);
						throw(exc);
					}
				}
			}
			else if(paramName=="vbw")
			{
				if(valueString=="full")
				{
					bandParam.vbw=50e6;
				}
				else
				{
					//The value is numerical
					iss.str(valueString);
					iss >> bandParam.vbw;
					if(bandParam.vbw!=3e6 && bandParam.vbw!=1e6 && bandParam.vbw!=300e3 && bandParam.vbw!=100e3 &&
							bandParam.vbw!=30e3 && bandParam.vbw!=10e3 && bandParam.vbw!=3e3 && bandParam.vbw!=1e3 &&
							bandParam.vbw!=120e3 && bandParam.vbw!=9e3 && bandParam.vbw!=200.0 && bandParam.vbw!=5e6 &&
							bandParam.vbw!=200e3 && bandParam.vbw!=1.5e6)
					{
						std::string str = "The given value to configure variable " + paramName + " is invalid.";
						CustomException exc(str);
						throw(exc);
					}
				}
			}
			else if(paramName=="sweep time")
			{
				iss.str(valueString);
				iss >> bandParam.sweepTime;
				if(bandParam.sweepTime<10 || bandParam.sweepTime>600000)
				{
					std::string str = "The given value to configure variable " + paramName + " is invalid.";
					CustomException exc(str);
					throw(exc);
				}
			}
			else if(paramName=="sample points")
			{
				iss.str(valueString);
				iss >> bandParam.samplePoints;
				bandParam.flagDefaultSamplePoints = (bandParam.samplePoints==0);
			}
			else if(paramName=="detector")
			{
				if(valueString=="rms")
				{
					bandParam.detector=0;
				}
				else if(valueString=="min/max")
				{
					bandParam.detector=1;
				}
				else
				{
					std::string str = "The given value to configure variable " + paramName + " is invalid.";
					CustomException exc(str);
					throw(exc);
				}
			}
			else
			{
				std::string str = "The parameter " + paramName + " is not a valid parameter.";
				CustomException exc(str);
				throw(exc);
			}

			//Control if the definitions of one band finished
			if(endChar==';')
			{
				if(bandParam.flagDefaultSamplePoints)
					bandParam.samplePoints = 2 * (unsigned int)( (bandParam.stopFreq - bandParam.startFreq) / bandParam.rbw ) + 1;

				bandsParam.push_back(bandParam);
				bandParam = {false, 0.0, 0.0, 0.0, 0.0, 0, 0, 0};

				if( ifs.peek()=='\n' )
					ifs.get();
			}
		}while(ifs.eof()!=true);

		ifs.close();
		return true;
	}

	return false;
}

void SpectranConfigurator::SetAndCheckVariable(SpecVariable variable, float value)
{
	//Setting up the variable
	Command comm(Command::SETSTPVAR, variable, value);
	Reply reply(Reply::SETSTPVAR);
	try
	{
		interface.Write(comm);
		interface.Read(reply);
		if(reply.IsRight()!=true)
		{
			std::string str = "The reply to the command to set up the variable \"" + reply.GetVariableNameString() + "\" was wrong.";
			CustomException exc(str);
			throw(exc);
		}
	}
	catch(CustomException & exc)
	{
		exc.Append("\nThe setting of the variable \"" + reply.GetVariableNameString() + "\" failed.");
		throw;
	}


	//Checking the variable's current value
	comm.Clear();
	comm.SetAs(Command::GETSTPVAR, variable);
	try
	{
		interface.Write(comm);
		reply.Clear();
		reply.PrepareTo(Reply::GETSTPVAR, variable);
		interface.Read(reply);
		if( reply.IsRight()!=true )
		{
			std::string str = "The reply to the command to get the current value of the \"" + reply.GetVariableNameString() + "\" was wrong.";
			CustomException exc(str);
			throw(exc);
		}
		else if( reply.GetValue()!=value )
		{
			std::string str = "The reply to the command to get the current value of the \"" + reply.GetVariableNameString() + "\" stated a different value with respect to the one which was used to configure it.";
			CustomException exc(str);
			throw(exc);
		}
	}
	catch(CustomException & exc)
	{
		exc.Append("\nThe checking of the configured variable \"" + reply.GetVariableNameString() + "\" failed.");
		throw;
	}
}

//! This method is intended to configure the spectrum analyzer with the parameters which will stay fixed during the multiple sweeps.
/*! This method should be used the first time the spectrum analyzer will be configured and when a measurements cycle has
 * finished if the fixed parameters have changed. Obviously, the sending of measurements via USB must be disable before
 * calling this method.
 */
void SpectranConfigurator::InitialConfiguration()
{
	SetAndCheckVariable( SpecVariable::ATTENFAC, float(fixedParam.attenFactor) );

	SetAndCheckVariable( SpecVariable::DISPUNIT, float(fixedParam.displayUnit) );

	SetAndCheckVariable( SpecVariable::DEMODMODE, float(fixedParam.demodMode) );

	SetAndCheckVariable( SpecVariable::ANTTYPE, float(fixedParam.antennaType) );

	SetAndCheckVariable( SpecVariable::CABLETYPE, float(fixedParam.cableType) );

	SetAndCheckVariable( SpecVariable::RECVCONF, float(fixedParam.recvConf) );

	SetAndCheckVariable( SpecVariable::PREAMPEN, float(fixedParam.internPreamp) );

	SetAndCheckVariable( SpecVariable::SWPDLYACC, float(fixedParam.sweepDelayAcc) );

	SetAndCheckVariable( SpecVariable::LEVELTONE, float(fixedParam.peakLevelAudioTone) );

	SetAndCheckVariable( SpecVariable::BACKBBEN, float(fixedParam.backBBDetector) );

	SetAndCheckVariable( SpecVariable::SPKVOLUME, fixedParam.speakerVol );
}

//! The aim of this method is to configure the spectrum analyzer with parameters of the next frequency band.
/*! The parameters which will be configured are the variable parameters, i.e. ones which are different from one
 * frequency band to another. The first time this method is called, it will configured the spectrum analyzer with
 * the first band's parameters. Again, the sending of measurements via USB should be disabled before calling this
 * method. The method returns the index of the next frequency band.
 */
unsigned int SpectranConfigurator::ConfigureNextBand()
{
	do
	{
		if( ++bandIndex >= bandsParam.size() )
		{
			bandIndex=0;
		}
	}while(bandsParam[bandIndex].flagEnable==false); //Checking if the current band is enabled

	SetAndCheckVariable( SpecVariable::STARTFREQ, bandsParam[bandIndex].startFreq );

	SetAndCheckVariable( SpecVariable::STOPFREQ, bandsParam[bandIndex].stopFreq );

	SetAndCheckVariable( SpecVariable::RESBANDW, bandsParam[bandIndex].rbw );

	SetAndCheckVariable( SpecVariable::VIDBANDW, bandsParam[bandIndex].vbw );

	SetAndCheckVariable( SpecVariable::SWEEPTIME, float(bandsParam[bandIndex].sweepTime) );

	if(!bandsParam[bandIndex].flagDefaultSamplePoints)
		SetAndCheckVariable( SpecVariable::SWPFRQPTS, float(bandsParam[bandIndex].samplePoints) );

	SetAndCheckVariable( SpecVariable::DETMODE, float(bandsParam[bandIndex].detector) );

	return bandIndex;
}
