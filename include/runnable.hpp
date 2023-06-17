#include <iostream>
#ifndef RUNNABLE_H
#define RUNNABLE_H

class Runnable
{
public:
	Runnable() : m_isRunning(false){
	}
	~Runnable() {
	}

	void start();
	virtual void run();
	void stop();

protected:
	bool m_isRunning;

};

#endif