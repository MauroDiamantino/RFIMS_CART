#include "SweepProcessing.h"

DataLogger::DataLogger()
{
	sweepIndex=10000; //To make sure this variable will be set to zero the first time the method SaveData() is called
	antPosition=0;
	
	if( !boost::filesystem::exists(MEASUREMENTS_PATH) )
	{
		try
		{
			boost::filesystem::create_directory(MEASUREMENTS_PATH);
		}
		catch(boost::filesystem::filesystem_error& exc)
		{
			CustomException customExc( "The creation of directory " + MEASUREMENTS_PATH.string() + " failed: " + exc.what() );
			throw(customExc);
		}
	}
						
	ofs.exceptions( std::ofstream::failbit | std::ofstream::badbit );
}

DataLogger::~DataLogger()
{
	ofs.exceptions( std::ofstream::goodbit );
	ofs.close();
}


void DataLogger::SetSweep(const FreqValueSet & swp)
{
	sweep=swp;
	if(++sweepIndex >= 2*NUM_OF_POSITIONS) //2 polarizations multiplied by NUM_OF_POSITIONS azimuthal positions
		sweepIndex=0;
}

bool DataLogger::SaveData()
{
	//The data are saved only if a sweep has been loaded
	if( !sweep.Empty() )
	{
		if(sweepIndex==0)
		{
			//First sweep
			firstSweepDate=sweep.timeData.date();
			
			//Creating the sweeps file
			boost::filesystem::path filesPath(MEASUREMENTS_PATH);
			filesPath /= ( "sweeps_" + firstSweepDate + ".csv" );
			try
			{
				ofs.open( filesPath.string() );
			}
			catch(std::ofstream::failure& exc)
			{
				CustomException customExc("The file " + filesPath.string() + " could not be created.");
				throw( customExc );
			}
			
			//Writing header with frequency values
			ofs << "Timestamp,Azimuthal Angle,Polarization,";
			for(auto& f : sweep.frequencies)
				ofs << std::setprecision(6) << (f/1e6) << ',';
			ofs << "\r\n";
		}
		else
		{
			//Opening the sweeps file
			boost::filesystem::path filesPath(MEASUREMENTS_PATH);
			filesPath /= ( "sweeps_" + firstSweepDate + ".csv" );
			try
			{
				ofs.open( filesPath.string(), std::ofstream::out | std::ofstream::app );
			}
			catch(std::ofstream::failure& exc)
			{
				CustomException customExc("The file " + filesPath.string() + " could not be opened.");
				throw( customExc );
			}
		}
		
		//Writing sweep's power values
		std::string aux = sweep.timeData.timestamp();
		ofs << sweep.timeData.timestamp() << ',' << std::setprecision(4) << antPosition << ',' << antPolarization << ',';
		for(auto& p : sweep.values)
			ofs << std::setprecision(5) << p << ',';
		ofs << "\r\n";
		
		ofs.close();

		return true;
	}

	return false;
}
