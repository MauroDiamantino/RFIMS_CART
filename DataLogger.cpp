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
						
	ofs.exceptions ( std::ofstream::failbit | std::ofstream::badbit );
}

bool DataLogger::SaveData()
{
	//The data are saved only if a sweep has been loaded
	if( !sweep.Empty() )
	{
		//At least a sweep was loaded

		if(++sweepIndex >= 2*NUM_OF_POSITIONS) //2 polarizations multiplied by NUM_OF_POSITIONS azimuthal positions
		{
			//First sweep
			sweepIndex=0;
			
			firstSweepDate=sweep.timestamp.date;
			
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
				ofs << f << ',';
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
		ofs << sweep.timestamp.Whole() << ',' << antPosition << ',' << antPolarization;
		for(auto& p : sweep.values)
			ofs << p << ',';
		ofs << "\r\n";
		
		ofs.close();

		return true;
	}

	return false;
}
