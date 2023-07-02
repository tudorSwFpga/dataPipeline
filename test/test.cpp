#include <memory>
#include <thread>
#include <plog/Log.h>
#include <plog/Initializers/RollingFileInitializer.h>
#include <stdexcept>
#include "../include/dataFramework.hpp"


int main(){
	plog::init(plog::debug, "/tmp/test.log", 1000000, 5);
	PLOG_INFO << "Starting System";
	//parse configuration
	const std::string conf = "topology.json";
	DataFramework example(conf);
	PLOG_INFO << "Starting System";
	example.start();
	std::cout << '\n' << "Press a key to stop...";
	do {
		example.run();	
		usleep(500000);    
 	} while (std::cin.get() != '\n');
 	example.stop();

	return 0;
}
