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
	boost::filesystem::path pathAndFilename(FILES_PATH);
	pathAndFilename /= "fixedparameters.txt";

	//Checking if the fixed parameters have been modified
	if( lastWriteTimes[0] < boost::filesystem::last_write_time(pathAndFilename) )
	{
		lastWriteTimes[0] = boost::filesystem::last_write_time(pathAndFilename);

		//Opening the files with the fixed parameters and loading these ones
		ifs.open( pathAndFilename.string() );
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
	boost::filesystem::path pathAndFilename(FILES_PATH);
	pathAndFilename /= "freqbands.txt";

	if( lastWriteTimes[1] < boost::filesystem::last_write_time(pathAndFilename) )
	{
		lastWriteTimes[1] = boost::filesystem::last_write_time(pathAndFilename);
		//Opening the files with the frequency bands's parameters and loading these ones
		ifs.open( pathAndFilename.string() );

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
				std::istringstream iss(valueString);
				iss >> bandParam.startFreq;
				if(bandParam.startFreq<1e6 || bandParam.startFreq>9.4e9)
				{
					std::ostringstream oss;
					oss << "The given value, " << bandParam.startFreq << ", to configure parameter Fstart is out of range.";
					CustomException exc( oss.str() );
					throw(exc);
				}
			}
			else if(paramName=="fstop")
			{
				std::istringstream iss(valueString);
				iss >> bandParam.stopFreq;
				if(bandParam.stopFreq<1e6 || bandParam.stopFreq>9.4e9)
				{
					std::ostringstream oss;
					oss << "The given value, " << bandParam.stopFreq << ", to configure parameter Fstop is out of range.";
					CustomException exc( oss.str() );
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
					std::istringstream iss(valueString);
					iss >> bandParam.rbw;
					if(bandParam.rbw!=3e6 && bandParam.rbw!=1e6 && bandParam.rbw!=300e3 && bandParam.rbw!=100e3 &&
							bandParam.rbw!=30e3 && bandParam.rbw!=10e3 && bandParam.rbw!=3e3 && bandParam.rbw!=1e3 &&
							bandParam.rbw!=120e3 && bandParam.rbw!=9e3 && bandParam.rbw!=200.0 && bandParam.rbw!=5e6 &&
							bandParam.rbw!=200e3 && bandParam.rbw!=1.5e6 && bandParam.rbw!=50e6)
					{
						std::ostringstream oss;
						oss << "The given value, " << bandParam.rbw << ", to configure parameter RBW is invalid.";
						CustomException exc( oss.str() );
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
					std::istringstream iss(valueString);
					iss >> bandParam.vbw;
					if(bandParam.vbw!=3e6 && bandParam.vbw!=1e6 && bandParam.vbw!=300e3 && bandParam.vbw!=100e3 &&
							bandParam.vbw!=30e3 && bandParam.vbw!=10e3 && bandParam.vbw!=3e3 && bandParam.vbw!=1e3 &&
							bandParam.vbw!=120e3 && bandParam.vbw!=9e3 && bandParam.vbw!=200.0 && bandParam.vbw!=5e6 &&
							bandParam.vbw!=200e3 && bandParam.vbw!=1.5e6 && bandParam.vbw!=50e6)
					{
						std::ostringstream oss;
						oss << "The given value, " << bandParam.vbw << ", to configure parameter VBW is invalid.";
						CustomException exc( oss.str() );
						throw(exc);
					}
				}
			}
			else if(paramName=="sweep time")
			{
				std::istringstream iss(valueString);
				iss >> bandParam.sweepTime;
				if(bandParam.sweepTime<10 || bandParam.sweepTime>600000)
				{
					std::ostringstream oss;
					oss << "The given value, " << bandParam.sweepTime << ", to configure parameter Sweep Time is invalid.";
					CustomException exc( oss.str() );
					throw(exc);
				}
			}
			else if(paramName=="sample points")
			{
				std::istringstream iss(valueString);
				iss >> bandParam.samplePoints;
				unsigned int defaultNumOfSamples = 2 * (unsigned int)(bandParam.stopFreq - bandParam.startFreq) / bandParam.rbw + 1;
				if(defaultNumOfSamples<51) defaultNumOfSamples=51;
				if(bandParam.samplePoints < defaultNumOfSamples)
				{
					bandParam.samplePoints = defaultNumOfSamples;
					bandParam.flagDefaultSamplePoints=true;
				}
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
					std::string str = "The given value, " + valueString + ", to configure parameter Detector is invalid.";
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
				float spansRatio = (bandParam.stopFreq - bandParam.startFreq) / 200e6;
				unsigned int numOfSubBands = ceil(spansRatio);

				if(numOfSubBands>1)
				{
					//If number of sub-bands is greater than 1 (span greater than 200MHz) the corresponding band is partitioned
					BandParameters subBandParam = bandParam;
					bool flagLastSubBand=false;

					for(unsigned int i=0; i<numOfSubBands; i++)
					{
						subBandParam.stopFreq = bandParam.startFreq + (i+1)*200e6;
						if( subBandParam.stopFreq  > bandParam.stopFreq )
						{
							//Last sub-band
							subBandParam.stopFreq = bandParam.stopFreq;
							subBandParam.sweepTime = bandParam.sweepTime * ( 1 - (numOfSubBands-1) / spansRatio );
							flagLastSubBand=true;
						}
						else
							subBandParam.sweepTime = bandParam.sweepTime / spansRatio;

						if(flagLastSubBand)
							subBandParam.samplePoints = bandParam.samplePoints * ( 1 - (numOfSubBands-1) / spansRatio );
						else
							subBandParam.samplePoints = bandParam.samplePoints / spansRatio;

						//Estimative calculation of the number of samples
						unsigned int defaultNumOfSamples = 2.0 * (subBandParam.stopFreq - subBandParam.startFreq) / subBandParam.rbw + 1;
						if(defaultNumOfSamples<51)	defaultNumOfSamples=51;
						if(subBandParam.samplePoints < defaultNumOfSamples)
						{
							subBandParam.samplePoints = defaultNumOfSamples;
							subBandParam.flagDefaultSamplePoints=true;
						}

						bandsParam.push_back(subBandParam);
						subBandParam.startFreq = subBandParam.stopFreq;
					}
				}
				else
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

void SpectranConfigurator::SetVariable(const SpecVariable variable, const float value)
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
}


void SpectranConfigurator::CheckEqual(const SpecVariable variable, const float value)
{
	//Checking the variable's current value
	Command comm(Command::GETSTPVAR, variable);
	Reply reply(Reply::GETSTPVAR, variable);
	try
	{
		interface.Write(comm);
		interface.Read(reply);
		if( reply.IsRight()!=true )
		{
			std::string str = "The reply to the command to get the current value of the \"" + reply.GetVariableNameString() + "\" was wrong.";
			CustomException exc(str);
			throw(exc);
		}
		else if( reply.GetValue()!=value )
		{
			std::ostringstream oss;
			oss << "The reply to the command to get the current value of the \"" + reply.GetVariableNameString() + "\" stated the value " << reply.GetValue() << " which is different to the one which was used to configure it, " << value << '.';
			CustomException exc( oss.str() );
			throw(exc);
		}
	}
	catch(CustomException & exc)
	{
		exc.Append("\nThe checking of the configured variable \"" + reply.GetVariableNameString() + "\" failed.");
		throw;
	}
}

void SpectranConfigurator::CheckApproxEqual(const SpecVariable variable, float & value)
{
	//Checking the variable's current value
	Command comm(Command::GETSTPVAR, variable);
	Reply reply(Reply::GETSTPVAR, variable);
	try
	{
		interface.Write(comm);
		interface.Read(reply);
		if( reply.IsRight()!=true )
		{
			std::string str = "The reply to the command to get the current value of the \"" + reply.GetVariableNameString() + "\" was wrong.";
			CustomException exc(str);
			throw(exc);
		}
		else if( reply.GetValue()<(0.9*value) || reply.GetValue()>(1.1*value) )
		{
			std::ostringstream oss;
			oss << "The reply to the command to get the current value of the \"" + reply.GetVariableNameString() + "\" stated the value " << reply.GetValue() << " which is different to the one which was used to configure it, " << value << '.';
			CustomException exc( oss.str() );
			throw(exc);
		}

		value = reply.GetValue();
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
	SetVariable( SpecVariable::ATTENFAC, float(fixedParam.attenFactor) );
	CheckEqual( SpecVariable::ATTENFAC, float(fixedParam.attenFactor) );

	SetVariable( SpecVariable::DISPUNIT, float(fixedParam.displayUnit) );
	CheckEqual( SpecVariable::DISPUNIT, float(fixedParam.displayUnit) );

	SetVariable( SpecVariable::DEMODMODE, float(fixedParam.demodMode) );
	CheckEqual( SpecVariable::DEMODMODE, float(fixedParam.demodMode) );

	SetVariable( SpecVariable::ANTTYPE, float(fixedParam.antennaType) );
	CheckEqual( SpecVariable::ANTTYPE, float(fixedParam.antennaType) );

	SetVariable( SpecVariable::CABLETYPE, float(fixedParam.cableType) );
	CheckEqual( SpecVariable::CABLETYPE, float(fixedParam.cableType) );

	SetVariable( SpecVariable::RECVCONF, float(fixedParam.recvConf) );
	CheckEqual( SpecVariable::RECVCONF, float(fixedParam.recvConf) );

	SetVariable( SpecVariable::PREAMPEN, float(fixedParam.internPreamp) );
	CheckEqual( SpecVariable::PREAMPEN, float(fixedParam.internPreamp) );

	SetVariable( SpecVariable::SWPDLYACC, float(fixedParam.sweepDelayAcc) );
	CheckEqual( SpecVariable::SWPDLYACC, float(fixedParam.sweepDelayAcc) );

	SetVariable( SpecVariable::LEVELTONE, float(fixedParam.peakLevelAudioTone) );
	CheckEqual( SpecVariable::LEVELTONE, float(fixedParam.peakLevelAudioTone) );

	SetVariable( SpecVariable::BACKBBEN, float(fixedParam.backBBDetector) );
	CheckEqual( SpecVariable::BACKBBEN, float(fixedParam.backBBDetector) );

	SetVariable( SpecVariable::SPKVOLUME, fixedParam.speakerVol );
	CheckApproxEqual( SpecVariable::SPKVOLUME, fixedParam.speakerVol );
}

//! The aim of this method is to configure the spectrum analyzer with parameters of the next frequency band.
/*! The parameters which will be configured are the variable parameters, i.e. ones which are different from one
 * frequency band to another. The first time this method is called, it will configured the spectrum analyzer with
 * the first band's parameters. Again, the sending of measurements via USB should be disabled before calling this
 * method. The method returns the index of the next frequency band.
 */
BandParameters SpectranConfigurator::ConfigureNextBand()
{
	do{
		if( ++bandIndex >= bandsParam.size() )
			bandIndex=0;
	}while(bandsParam[bandIndex].flagEnable==false); //Checking if the current band is enabled

	SetVariable( SpecVariable::STARTFREQ, bandsParam[bandIndex].startFreq );
	CheckApproxEqual( SpecVariable::STARTFREQ, bandsParam[bandIndex].startFreq );

	SetVariable( SpecVariable::STOPFREQ, bandsParam[bandIndex].stopFreq );
	CheckApproxEqual( SpecVariable::STOPFREQ, bandsParam[bandIndex].stopFreq );

	SetVariable( SpecVariable::RESBANDW, bandsParam[bandIndex].rbw );
	CheckEqual( SpecVariable::RESBANDW, bandsParam[bandIndex].rbw );

	SetVariable( SpecVariable::VIDBANDW, bandsParam[bandIndex].vbw );
	CheckEqual( SpecVariable::RESBANDW, bandsParam[bandIndex].rbw );

	SetVariable( SpecVariable::SWEEPTIME, float(bandsParam[bandIndex].sweepTime) );
	try
	{
		CheckEqual( SpecVariable::SWEEPTIME, float(bandsParam[bandIndex].sweepTime) );
	}
	catch(CustomException & exc)
	{
		std::string str( exc.what() );
		if( str.find("different") )
		{
			Command comm(Command::GETSTPVAR, SpecVariable::SWEEPTIME);
			Reply reply(Reply::GETSTPVAR, SpecVariable::SWEEPTIME);
			try
			{
				interface.Write(comm);
				interface.Read(reply);
				if( !reply.IsRight() )
				{
					CustomException exc("The reply to the command to get the current value of the \"sweep time\" was wrong.");
					throw(exc);
				}
				if( bandsParam[bandIndex].sweepTime > (unsigned int) reply.GetValue() )
				{
					std::ostringstream oss;
					oss << "The sweep time value which was taken by the spectrum analyzer, " << reply.GetValue() << "ms, is lesser than the original one, " << bandsParam[bandIndex].sweepTime << "ms.";
					CustomException exc( oss.str() );
					throw(exc);
				}
				bandsParam[bandIndex].sweepTime = (unsigned int) reply.GetValue();
			}
			catch(CustomException & exc)
			{
				std::string str = "The assignment of the actual sweep time value which was taken by the spectrum analyzer to the internal variable failed: ";
				str += exc.what();
				CustomException exc1(str);
				throw exc1;
			}
			cerr << "\nWarning: The band N " << bandIndex << " (Fstart=" << bandsParam[bandIndex].startFreq << ", Fstop=";
			cerr << bandsParam[bandIndex].stopFreq << ") will have the sweep time " << bandsParam[bandIndex].sweepTime;
			cerr << "ms which is bigger than original one";
		}
		else
		{
			throw;
		}
	}

	if(bandsParam[bandIndex].flagDefaultSamplePoints)
		SetVariable( SpecVariable::SWPFRQPTS, 1.0 ); //dummy setting to make sure that the spectrum analyzer has taken the default number of samples
	else
		SetVariable( SpecVariable::SWPFRQPTS, float(bandsParam[bandIndex].samplePoints) );

	SetVariable( SpecVariable::DETMODE, float(bandsParam[bandIndex].detector) );
	CheckEqual( SpecVariable::DETMODE, float(bandsParam[bandIndex].detector) );

	return bandsParam[bandIndex];
}
