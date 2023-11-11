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
#include "WinUDPRxModule.h"
#include "SessionProcModule.h"
#include "RouterModule.h"
#include "WAVAccumulator.h"
#include "WAVWriterModule.h"
#include "TimeToWAVModule.h"
#include "HPFModule.h"
#include "WinUDPTxModule.h"
#include "WinTCPRxModule.h"
#include "WinTCPTxModule.h"
#include "SimulatorModule.h"
#include "ChunkToBytesModule.h"

/*External Libraries*/
#include "json.hpp"

int main()
{

	// ----------------
	// Config Variables
	// ----------------

	// Simuator Module
	double dSampleRate;
	double dSimulatedFrequency;
	unsigned uNumChannels;
	std::vector<uint8_t> vu8SourceIdentifier = { 0,0 ,1,1,1,1};

	// TCP Tx
	std::string strTCPTxIP;
	std::string strTCPTxPort;


	try
	{	
		// Reading and parsing JSON config
		std::ifstream file("./Config.json");
		std::string jsonString((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		nlohmann::json jsonConfig = nlohmann::json::parse(jsonString);

		// Updating config variables 
		// Simulator Module Config
		dSampleRate = std::stod((std::string)jsonConfig["Config"]["SimulatorModule"]["SampleRate_Hz"]);
		dSimulatedFrequency = std::stod((std::string)jsonConfig["Config"]["SimulatorModule"]["SimulatedFrequency_Hz"]);
		uNumChannels = std::stoul((std::string)jsonConfig["Config"]["SimulatorModule"]["NumberOfChannels"]);

		// TCP Module Config
		strTCPTxIP = jsonConfig["Config"]["TCPTxModule"]["IP"];
		strTCPTxPort = jsonConfig["Config"]["TCPTxModule"]["Port"];
		
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		throw;
	}
	
	// ------------
	// Construction
	// ------------
	auto pSimulatorModule = std::make_shared<SimulatorModule>(dSampleRate, 512, uNumChannels, dSimulatedFrequency, vu8SourceIdentifier,10);
	auto pChunkToBytesModule = std::make_shared<ChunkToBytesModule>(100, 512);
	auto pTCPTXModule = std::make_shared<WinTCPTxModule>(strTCPTxIP, strTCPTxPort, 100, 512);
	
	std::vector<float> vfPhases_deg = { 0, 0 };
	pSimulatorModule->SetChannelPhases(vfPhases_deg);

	// ------------
	// Connection
	// ------------
	pSimulatorModule->SetNextModule(pChunkToBytesModule);
	pChunkToBytesModule->SetNextModule(pTCPTXModule);
	pTCPTXModule->SetNextModule(nullptr);

	// ------------
	// Start-Up
	// ------------
	pTCPTXModule->StartProcessing();
	pChunkToBytesModule->StartProcessing();
	pSimulatorModule->StartProcessing();
	
	while (1)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}
