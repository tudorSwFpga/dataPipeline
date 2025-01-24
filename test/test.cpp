#include <memory>
#include <thread>
#include "spdlog/spdlog.h"
#include <stdexcept>
#include "../include/dataFramework.hpp"
#include <fstream>
#include <stdlib.h>

// wrapepr class to allow access to the data received by the TCP client
class testTcpClient : public TcpClient {
public:
    using TcpClient::TcpClient;
    std::vector<std::string> data;
    bool done;
};

// class allowing to spawn TCP clients and check the data received
class testTool {
public:
    testTool(const std::string &topology, const int &time_sec) :
        m_topology(topology), m_timeToRun(time_sec), m_framework(topology){};

    ~testTool(){};

    std::array<std::vector<std::string>, 8> m_txData;
    std::array<std::vector<std::string>, 8> m_rxData;

    void run() {
        m_framework.run();
        std::thread clientThreads[2];
        for (auto &i : {0, 1}) {
            clientThreads[i] =
                std::thread(&testTool::spawnTCPClient, this, "client" + std::to_string(i), i, 50000 + i, 10);
        }
        for (auto &i : {0, 1}) {
            clientThreads[i].join();
        }
    };

    bool check() {
        // retrieve consumer names so that we can check the data received in the files
        std::vector<std::string> consumerNames;
        m_framework.getConsumerNames(consumerNames);
        for (auto &i : consumerNames) {
            const int consumer_id = std::stoi(i.substr(i.size() - 1));
            std::ifstream file(i + ".txt");
            if (!file.is_open()) {
                spdlog::error("File {} not found", i + ".txt");
                return false;
            }
            std::string line;
            while (std::getline(file, line)) {
                m_rxData[consumer_id].push_back(line);
            }
            file.close();

            for (auto &tx : m_txData) {
                for (auto &j : tx) {
                    bool found = false;
                    for (auto k = m_rxData[consumer_id].begin(); k != m_rxData[consumer_id].end(); ++k) {
                        if (k->find(j) != std::string::npos) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        spdlog::error("{} data not received by {}", j, i);
                        return false;
                    }
                }
            }
        }
        spdlog::info("testTool::check() successful check");
        return true;
    }

private:
    const std::string m_topology;
    const int m_timeToRun;
    std::mutex m_mutex;
    DataFramework m_framework;

    std::string gen_random(const int len) {
        static const char alphanum[] = "0123456789"
                                       "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                       "abcdefghijklmnopqrstuvwxyz";
        std::string tmp_s;
        tmp_s.reserve(len);

        for (int i = 0; i < len; ++i) {
            tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
        }

        return tmp_s;
    }

    void spawnTCPClient(const std::string &name, const int &id, const uint16_t &port, const int &time_sec) {
        std::shared_ptr<testTcpClient> tcpClient = std::make_shared<testTcpClient>(name);
        tcpClient->done                          = false;
        tcpClient->connect(port);
        // launch TCP client thread
        spdlog::debug("Client thread running");
        auto t           = std::chrono::system_clock::now();
        const auto t_end = t + std::chrono::seconds(time_sec);
        int msg_cnt      = 0;
        while (std::chrono::system_clock::now() < t_end) {
            // const auto data = gen_random(64);
            const std::string data = "Client_" + std::to_string(id) + "_message_" + std::to_string(msg_cnt++);
            spdlog::debug("tcpClient sending " + data);
            tcpClient->send(data);
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_txData[id].push_back(data);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        tcpClient->done = true;
        tcpClient->disconnect();
    }
};

int main() {
    spdlog::set_level(spdlog::level::debug);
    // parse configuration
    testTool test("topology.json", 5);
    test.run();
    test.check();
    return 0;
}
