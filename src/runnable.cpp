#include "../include/runnable.hpp"

//TODO: convert this to bool or int to return something
void Runnable::start(){
	m_isRunning = true;
}

void Runnable::stop(){
	m_isRunning = false;
}

void Runnable::run(){
	std::cout << "Run" << std::endl;
}