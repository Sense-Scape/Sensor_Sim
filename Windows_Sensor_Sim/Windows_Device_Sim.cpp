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

	// TCP Tx
	std::string strTCPTxIP;
	std::string strTCPTxPort;
	std::vector<uint8_t> vu8SourceIdentifier = { 0,0 };

	try
	{	
		// Reading and parsing JSON config
		std::ifstream file("./Config.json");
		std::string jsonString((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		nlohmann::json jsonConfig = nlohmann::json::parse(jsonString);

		// Updating config variables 
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
	auto pSimulatorModule = std::make_shared<SimulatorModule>(44100, 512, 2, 10000, vu8SourceIdentifier,10);
	auto pChunkToBytesModule = std::make_shared<ChunkToBytesModule>(100, 512);
	auto pTCPRXModule = std::make_shared<WinTCPTxModule>(strTCPTxIP, strTCPTxPort, 100, 512);

	// ------------
	// Connection
	// ------------
	pSimulatorModule->SetNextModule(pChunkToBytesModule);
	pChunkToBytesModule->SetNextModule(pTCPRXModule);
	pTCPRXModule->SetNextModule(nullptr);

	// ------------
	// Start-Up
	// ------------
	pTCPRXModule->StartProcessing();
	pChunkToBytesModule->StartProcessing();
	pSimulatorModule->StartProcessing();
	
	while (1)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}
