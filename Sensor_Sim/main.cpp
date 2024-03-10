/*Standard Includes*/
#include <memory>
#include <vector>
#include <queue>
#include <iostream>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>

/*Custom Includes*/
#include "BaseModule.h"
#include "SessionProcModule.h"
#include "RouterModule.h"
#include "WAVAccumulator.h"
#include "LinuxMultiClientTCPTxModule.h"
#include "SimulatorModule.h"
#include "ChunkToBytesModule.h"
#include "LinuxWAVReaderModule.h"

/* External Libraries */
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Appenders/RollingFileAppender.h>
#include "json.hpp"

int main()
{

	// ---------------------
	// Logging Configuration
	// ---------------------

	// Reading and parsing JSON config
	std::ifstream file("./Config.json");
	std::string jsonString((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	nlohmann::json jsonConfig = nlohmann::json::parse(jsonString);

	try // To configure logging
	{
		auto eLogLevel = plog::debug;

		// Get log level
		std::string strRequestLogLevel = jsonConfig["LoggingConfig"]["LoggingLevel"];
		std::transform(strRequestLogLevel.begin(), strRequestLogLevel.end(), strRequestLogLevel.begin(), [](unsigned char c)
					   { return std::toupper(c); });

		// Select log level
		if (strRequestLogLevel == "DEBUG")
			eLogLevel = plog::debug;
		else if (strRequestLogLevel == "INFO")
			eLogLevel = plog::info;
		else if (strRequestLogLevel == "WARNING")
			eLogLevel = plog::warning;
		else if (strRequestLogLevel == "ERROR")
			eLogLevel = plog::error;
		else
			eLogLevel = plog::debug;

		// And create loggers the after try append to it
		auto &logger = plog::init(eLogLevel);

		// Check if one should log to files
		std::string strLogToFile = jsonConfig["LoggingConfig"]["LogToTextFile"];
		std::transform(strLogToFile.begin(), strLogToFile.end(), strLogToFile.begin(), [](unsigned char c)
					   { return std::toupper(c); });

		if (strLogToFile == "TRUE")
		{
			// Get the time for the log file
			time_t lTime;
			time(&lTime);
			auto strTime = std::to_string((long long)lTime);
			std::string strLogFileName = "Sensor_Sim_" + strTime + ".log";

			// The create and add the appender
			static plog::RollingFileAppender<plog::CsvFormatter> fileAppender(strLogFileName.c_str(), 50'000'000, 2);
			logger.addAppender(&fileAppender);
		}

		// And to console
		std::string strLogToConsole = jsonConfig["LoggingConfig"]["LogToConsole"];
		std::transform(strLogToConsole.begin(), strLogToConsole.end(), strLogToConsole.begin(), [](unsigned char c)
					   { return std::toupper(c); });

		if (strLogToConsole == "TRUE")
		{
			static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
			logger.addAppender(&consoleAppender);
		}
	}
	catch (const std::exception &e)
	{
		PLOG_ERROR << e.what();
		throw;
	}

	// ----------------
	// Config Variables
	// ----------------

	// Simuator Module
	bool bSimulatorEnabled = false;
	double dSampleRate;
	double dSimulatedFrequency;
	unsigned uNumChannels;

	// Playback Module
	bool bPlaybackEnabled = false;
	std::string sPlaybackDirectory;

	std::vector<float> vfChannelPhases_deg = {};
	std::vector<uint8_t> vu8SourceIdentifier = {0};

	// TCP Tx
	std::string strTCPTxIP;
	std::string strTCPTxPort;

	try
	{
		// Updating config variables
		// Simulator Module Config
		std::istringstream issSimulatorEnabled = std::istringstream(jsonConfig["PipelineConfig"]["SimulatorModule"]["Enabled"].get<std::string>());
		issSimulatorEnabled >> std::boolalpha >> bSimulatorEnabled;
		std::cout << "1" << std::endl;
		dSampleRate = std::stod((std::string)jsonConfig["PipelineConfig"]["SimulatorModule"]["SampleRate_Hz"]);
		dSimulatedFrequency = std::stod((std::string)jsonConfig["PipelineConfig"]["SimulatorModule"]["SimulatedFrequency_Hz"]);
		uNumChannels = std::stoul((std::string)jsonConfig["PipelineConfig"]["SimulatorModule"]["NumberOfChannels"]);
		vfChannelPhases_deg = jsonConfig["PipelineConfig"]["SimulatorModule"]["ChannelPhases_deg"].get<std::vector<float>>();
		vu8SourceIdentifier = jsonConfig["PipelineConfig"]["SimulatorModule"]["SourceIdentifier"].get<std::vector<uint8_t>>();
		if (vfChannelPhases_deg.size() != uNumChannels)
		{
			std::string strFatal = std::string(__FUNCTION__) + "Channel phase count does to equal channel count";
			PLOG_FATAL << strFatal;
			throw;
		}

		// Playback Module
		std::istringstream issPlaybackEnabled = std::istringstream(jsonConfig["PipelineConfig"]["PlaybackModule"]["Enabled"].get<std::string>());
		issPlaybackEnabled >> std::boolalpha >> bPlaybackEnabled;

		sPlaybackDirectory = jsonConfig["PipelineConfig"]["PlaybackModule"]["Directory"].get<std::string>();

		// TCP Module Config
		strTCPTxIP = jsonConfig["PipelineConfig"]["TCPTxModule"]["IP"];
		strTCPTxPort = jsonConfig["PipelineConfig"]["TCPTxModule"]["Port"];
	}
	catch (const std::exception &e)
	{
		std::cout << e.what() << std::endl;
		throw;
	}

	// ------------
	// Construction
	// ------------
	std::shared_ptr<SimulatorModule> pSimulatorModule;
	if (bSimulatorEnabled)
	{
		pSimulatorModule = std::make_shared<SimulatorModule>(dSampleRate, 512, uNumChannels, dSimulatedFrequency, vu8SourceIdentifier, 10);
		pSimulatorModule->SetChannelPhases(vfChannelPhases_deg, "Degree");
	}

	std::shared_ptr<LinuxWAVReaderModule> pLinuxWAVReader;
	if (bPlaybackEnabled)
		auto pLinuxWAVReader = std::make_shared<LinuxWAVReaderModule>(sPlaybackDirectory, 512, 100);

	auto pChunkToBytesModule = std::make_shared<ChunkToBytesModule>(100, 512);
	auto pTCPTXModule = std::make_shared<LinuxMultiClientTCPTxModule>(strTCPTxIP, strTCPTxPort, 100, 512);

	// ------------
	// Connection
	// ------------
	if (bSimulatorEnabled)
		pSimulatorModule->SetNextModule(pChunkToBytesModule);
	if (bPlaybackEnabled)
		pLinuxWAVReader->SetNextModule(pChunkToBytesModule);

	pChunkToBytesModule->SetNextModule(pTCPTXModule);
	pTCPTXModule->SetNextModule(nullptr);

	// ------------
	// Start-Up
	// ------------
	if (bSimulatorEnabled)
		pSimulatorModule->StartProcessing();
	if (bPlaybackEnabled)
		pLinuxWAVReader->StartProcessing();

	pTCPTXModule->StartProcessing();
	pChunkToBytesModule->StartProcessing();

	while (1)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}
