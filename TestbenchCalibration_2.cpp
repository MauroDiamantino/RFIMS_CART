/*
 * TestbenchCalibration.cpp
 *
 *  Created on: 19/04/2019
 *      Author: new-mauro
 */

#include "SweepProcessing.h"

const float TOTAL_GAIN = 30.0; //dB (1 RF switch, 2 SMA-SMA adapters, 1 coaxial 1m cable and 2 Mini-Circuits LNA)
const float TOTAL_NF = 1.5; //dB (1 RF switch, 2 SMA-SMA adapters, 1 coaxial 1m cable and 2 Mini-Circuits LNA)

const float RBW = 1e6;
const float BOLTZMANN_CONST = 1.38e-23;
const float REF_TEMPERATURE = 290.0;
const float ENR_DB = 15.0;

void WaitForKey()
{
	cin.clear();
	cin.ignore(std::cin.rdbuf()->in_avail());
	cin.get();
}

FreqValueSet FrontEnd(const FreqValueSet & inputPower)
{
	float totalNoiseFactor, totalNoiseTemp ;
	totalNoiseFactor = pow(10.0, TOTAL_NF/10.0);
	totalNoiseTemp = REF_TEMPERATURE * (totalNoiseFactor - 1.0);

	FreqValueSet outputPower;
	float gain = pow(10.0, TOTAL_GAIN/10.0);
	outputPower = gain * ( inputPower + BOLTZMANN_CONST * RBW * totalNoiseTemp );
	return outputPower;
}

int main()
{
	cout << "Testbench de las clases asociadas con la calibracion del front end y de los datos" << endl;

	BandParameters aux;
	std::vector<BandParameters> bandsParam;

	aux.detector=0;
	aux.flagDefaultSamplePoints=false;
	aux.flagEnable=true;
	aux.rbw=RBW;
	aux.startFreq=900e6;
	aux.stopFreq=950e6;
	aux.sweepTime=1000;
	aux.vbw=3e3;
	aux.samplePoints = 2 * ( aux.stopFreq - aux.startFreq ) / aux.rbw; //101
	bandsParam.push_back(aux);

	aux.detector=0;
	aux.flagDefaultSamplePoints=false;
	aux.flagEnable=true;
	aux.rbw=RBW;
	aux.startFreq=950e6;
	aux.stopFreq=1000e6;
	aux.sweepTime=1000;
	aux.vbw=3e3;
	aux.samplePoints = 2 * ( aux.stopFreq - aux.startFreq ) / aux.rbw; //101
	bandsParam.push_back(aux);

	CurveAdjuster adjuster(bandsParam);
	FrontEndCalibrator frontCalibrator(adjuster, bandsParam);

	frontCalibrator.LoadENR();

	frontCalibrator.StartCalibration();

	FreqValueSet inputPower;
	Gnuplot inputPloter, outputPloter;

	//Noise source turned off and at ambient temperature (290°K)
	float power;
	for(auto it=bandsParam.begin(); it!=bandsParam.end(); it++)
	{
		float deltaFreq = (it->stopFreq - it->startFreq) / (it->samplePoints - 1);
		power = BOLTZMANN_CONST * it->rbw * REF_TEMPERATURE;
		//All bands don't include the last frequency, except the last one
		for(float f = it->startFreq; f < it->stopFreq; f += deltaFreq)
		{
			inputPower.frequencies.push_back(f);
			inputPower.values.push_back(power);
		}
	}
	//The last band includes the stop frequency
	inputPower.frequencies.push_back( bandsParam.back().stopFreq );
	inputPower.values.push_back(power);

	FreqValueSet inputPowerdBm = 10.0*log10(inputPower) + 30.0;
	inputPloter.plot_xy(inputPowerdBm.frequencies, inputPowerdBm.values, "Potencia de entrada con NS apagado");

	FreqValueSet outFrontEndPower = FrontEnd(inputPower);

	FreqValueSet outFrontEndPowerdBm = 10.0*log10(outFrontEndPower) + 30.0;
	frontCalibrator.SetSweep(outFrontEndPowerdBm);
	outputPloter.plot_xy(outFrontEndPowerdBm.frequencies, outFrontEndPowerdBm.values, "Potencia de salida con NS apagado");

	inputPower.Clear();
	outFrontEndPower.Clear();

	frontCalibrator.TurnOnNS();

	//Noise source turned on and at ambient temperature (290°K)

	float tson = REF_TEMPERATURE * ( pow(10.0, ENR_DB/10.0) + 1.0 );

	for(auto it=bandsParam.begin(); it!=bandsParam.end(); it++)
	{
		float deltaFreq = (it->stopFreq - it->startFreq) / (it->samplePoints - 1);
		power = BOLTZMANN_CONST * it->rbw * tson;
		for(float f=it->startFreq; f<it->stopFreq; f+=deltaFreq)
		{
			inputPower.frequencies.push_back(f);
			inputPower.values.push_back(power);
		}
	}
	//The last band includes the stop frequency
	inputPower.frequencies.push_back( bandsParam.back().stopFreq );
	inputPower.values.push_back(power);

	inputPowerdBm.Clear();
	inputPowerdBm = 10.0*log10(inputPower) + 30.0;
	inputPloter.plot_xy(inputPowerdBm.frequencies, inputPowerdBm.values, "Potencia de entrada con NS encendido");

	outFrontEndPower = FrontEnd(inputPower);

	outFrontEndPowerdBm.Clear();
	outFrontEndPowerdBm = 10.0*log10(outFrontEndPower) + 30.0;
	frontCalibrator.SetSweep(outFrontEndPowerdBm);
	outputPloter.plot_xy(outFrontEndPowerdBm.frequencies, outFrontEndPowerdBm.values, "Potencia de salida con NS encendido");

	frontCalibrator.SetNSoffTemperature(REF_TEMPERATURE);

	FrontEndParameters frontParameters = frontCalibrator.CalculateParameters();

	frontCalibrator.EndCalibration();

	Gnuplot ploter1, ploter2;

	ploter1.plot_xy(frontParameters.frequency, frontParameters.noiseFigure, "Figura de ruido del front end(NF, dB)");
	ploter2.plot_xy(frontParameters.frequency, frontParameters.gain_dB, "Ganancia del front end (G, dB)");

	cout << "Presione una tecla para terminar..." << endl;
	WaitForKey();

	return 0;
}
