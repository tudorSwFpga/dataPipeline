#include <poll.h>
#include <string>




template<class T>
DataManager<T>* DataManager<T>::getInstance(const std::string& name){
	std::lock_guard<std::mutex> lock(m_Mutex);
	if(m_Pinstance == nullptr){
		m_Pinstance = new DataManager<T>(name);
	}
	return m_Pinstance;
}


template<class T>
bool DataManager<T>::setConf(const uint8_t& inQueues,const uint8_t& outQueues,const MODE& mode){
	m_maxNbInQueues = inQueues;
	m_maxNbOutQueues = outQueues;
	m_mode = mode;
	return true;
}

template<class T>
void DataManager<T>::run(){
	m_isRunning = true;
	if (m_mode == BROADCAST) {
		PLOG_DEBUG << " Manage data in broadcast mode";
		DataManager::manageBroadcast();
	} else {
		DataManager::manageMap();
	}
}

template<class T>
void DataManager<T>::stop(){
	PLOG_DEBUG << " Stopping...";
	m_isRunning = false;
	m_condVarIn.notify_all();

}


/*
check each input queue status and copy on every output queue
*/
template<class T>
void DataManager<T>::manageBroadcast(){
	PLOG_DEBUG << " Waiting...";
	std::unique_lock<std::mutex> lk(m_inQueuesMutex);
	m_condVarIn.wait(lk, [this] {
		return m_isRunning == false;
	} );
	T data;
	for (auto &it:m_inQueues){
		if (!it.empty()){	
			data = it.front();
			it.pop();
			{	//local context to reduce lock time acquisition
				PLOG_DEBUG << " Acquire out lock";
				std::unique_lock<std::mutex> lockout(m_outQueuesMutex);
				for (auto &it:m_outQueues) {
					it.push(data);
					PLOG_DEBUG << " Pushing new data " << data << " to output q;Notyifing consumers!";
				}
				m_condVarOut.notify_all();
			}	
		}
	}
}

template<class T>
void DataManager<T>::manageMap(){
}


template<class T>
void DataManager<T>::push(T&& data,const std::string& pushId){
	//acquire mutex and push data into the input queue 
	std::unique_lock<std::mutex> guard(m_inQueuesMutex);
	if  (m_inQueuesIds.count(pushId) == 0) {
		PLOG_ERROR << "No input queue with ID: " << pushId;
	} else {
		m_inQueues[m_inQueuesIds[pushId]].push(data);
		PLOG_INFO << " Pushed new data " << data << " in queue " <<  std::to_string(m_inQueuesIds[pushId]);
		//notify thread waiting waiting for input data
		m_condVarIn.notify_one();
	}
}


template<class T>
bool DataManager<T>::pop(const std::string& popId, T* data){
	//acquire mutex and push data into the input queue 
	if  (m_outQueuesIds.count(popId) == 0) {
		PLOG_ERROR << "No output queue with ID: " << popId;
		return false;
	} else {
		std::unique_lock<std::mutex> lk(m_outQueuesMutex);
		PLOG_DEBUG << "Waiting until corresponding output queue " << m_outQueuesIds[popId] <<  " is filled...";
		//auto t = m_outQueues[m_outQueuesIds[popId]].empty();
		//while (m_outQueues[m_outQueuesIds[popId]].empty()){
				m_condVarOut.wait(lk);
		//}
		//pop oldest data and then erase it
		data[0] = m_outQueues[m_outQueuesIds[popId]].front();
		m_outQueues[m_outQueuesIds[popId]].pop();
		PLOG_INFO << "Returned data from output queue " << popId << " " << data[0];
		return true;	
	}
}

template<class T>
bool  DataManager<T>::setFeeder(const std::string& appId){
	if (m_inQueuesIds.count(appId) == 0){
		//initially all the map is empty 
		if (m_inQueuesIds.size() < m_maxNbInQueues){
			m_inQueuesIds.emplace(std::make_pair(appId,m_inQueuesIds.size()));
			m_inQueues.push_back(std::queue<T>());
			PLOG_DEBUG << "New Feeder: " << appId;

		//but once it becomes full, we can only find place if a feeder has left
		} else {
			auto f = m_inQueuesIds.find("empty");
			if (f != m_inQueuesIds.end()){
				m_inQueuesIds.emplace(std::make_pair(appId,f->second));
				m_inQueuesIds.erase(f);
				PLOG_DEBUG << "New Feeder: " << appId;
				return true;
			} else {
				PLOG_ERROR << " No empty place to set new Feeder " << appId;	
				return false;
			}
		}
	} else {
		PLOG_ERROR << " Feeder already existing:  " << appId;;	
		return false;
	}
}


template<class T>
bool  DataManager<T>::setConsumer(const std::string& appId){
	if (m_outQueuesIds.count(appId) == 0){
		//initially all the map is empty
		if (m_outQueuesIds.size() < m_maxNbOutQueues){
			m_outQueuesIds.emplace(std::make_pair(appId,m_outQueuesIds.size()));
			m_outQueues.push_back(std::queue<T>());
			PLOG_INFO << "New Consumer " << appId;	

		//but once it becomes full, we can only find place if a feeder has left
		} else {
			auto f = m_outQueuesIds.find("empty");
			if (f != m_outQueuesIds.end()){
				m_outQueuesIds.emplace(std::make_pair(appId,f->second));
				m_outQueuesIds.erase(f);
				PLOG_INFO << "New Consumer: " << appId;
				return true;
			} else {
				PLOG_ERROR << " No empty place to set new Consumer " << appId;	
				return false;
			}
		}
	} else {
		PLOG_ERROR << " Consumer already existing:  " << appId;;	
		return false;
	}
}



template class  DataManager<std::string>;
template <class T>
DataManager<T>* DataManager<T>::m_Pinstance{nullptr};
template <class T>
std::mutex DataManager<T>::m_Mutex;

//template class  DataManager<int>;

