#include <vector>
#include <stdexcept>
#include <cstdint>
#include <iostream>
#include "SignalGenerator.h"
#include <string>
#include <chrono>
#include <thread>
#include <iostream>

#include <zmq.hpp>

#include <math.h>
using namespace std;

vector<bool> G1;
vector<bool> G2;
int K;
vector<vector<vector<bool>>> tableState;
vector<vector<int>> metrix;
vector<vector<bool>> paths;
vector<vector<bool>> curPaths;
vector<bool> resultCode;
vector<bool> finishState;
int minMetrix = 0;
int indexMin = 0;

vector<bool> decToBin(int dec, int sizeBits)
{
    vector<bool> binaryNum(sizeBits);
    int i = sizeBits - 1;
    while (dec > 0) {
        if (i < 0)
        {
            cout << "i < 0!" << endl;
            break;
        }
        binaryNum[i] = dec % 2;
        dec = dec / 2;
        i--;
    }
    return binaryNum;
}

int binToDec(const vector<bool>& bin)
{
    int res = 0;
    int size = bin.size() - 1;
    for (int f = size; f >= 0; f--)
    {
        res += pow(2, size - f) * (bin[f] == 1 ? 1 : 0);
    }
    return res;
}

void createRow(vector<bool> curState, bool input, vector<bool>& output, vector<bool>& nextState)
{
    curState.insert(curState.begin(), input);
    output.resize(2);
    vector<int> pos1;
    vector<int> pos2;
    for (int i = 0; i < G1.size(); i++)
    {
        if (G1[i])
            pos1.push_back(i);
        if (G2[i])
            pos2.push_back(i);
    }
    output[0] = curState[pos1[0]];
    for (int i = 1; i < pos1.size(); i++)
    {
        output[0] = output[0] ^ curState[pos1[i]];
    }
    output[1] = curState[pos2[0]];
    for (int i = 1; i < pos2.size(); i++)
    {
        output[1] = output[1] ^ curState[pos2[i]];
    }
    nextState = curState;
    nextState.pop_back();
}

void createTable()
{
    int index = 0;
    vector<bool> out;
    vector<bool> nextState;
    vector<bool> curState;
    tableState.resize(2 * pow(2, K - 1));
    for (int i = 0; i < pow(2, K - 1); i++)
    {
        index = 2 * i;
        tableState[index].resize(3);
        curState = decToBin(i, K - 1);
        createRow(curState, 0, out, nextState);
        tableState[index][0] = curState;
        tableState[index][1] = nextState;
        tableState[index][2] = out;

        index += 1;
        tableState[index].resize(3);
        createRow(curState, 1, out, nextState);
        tableState[index][0] = curState;
        tableState[index][1] = nextState;
        tableState[index][2] = out;
    }
}

int hamingDist(vector<bool> v1, bool v21, bool v22)
{
    int res = 0;
    if (v1[0] != v21)
        res++;
    if (v1[1] != v22)
        res++;
    return res;
}

void setMetrix()
{
    minMetrix = metrix[0][1];
    indexMin = 0;
    for (int i = 0; i < metrix.size(); i++)
    {
        if (minMetrix >= metrix[i][1])
        {
            minMetrix = metrix[i][1];
            indexMin = i;
        }
    }
    finishState = tableState[indexMin][1];

    int min = 0;
    curPaths.resize(pow(2, K - 1));
    vector<bool> nextCell;
    vector<bool> startCell;
    int num = 0;
    auto curMetrix = metrix;
    for (int k = 0; k < metrix.size() / 4; k++)
    {
        curPaths[2 * k].clear();
        if (metrix[4 * k][1] <= metrix[4 * k + 2][1])
        {
            min = metrix[4 * k][1];
            startCell = tableState[4 * k][0];
            nextCell = tableState[4 * k][1];
        }
        else
        {
            min = metrix[4 * k + 2][1];
            startCell = tableState[4 * k + 2][0];
            nextCell = tableState[4 * k + 2][1];
        }
        num = binToDec(nextCell);
        curMetrix[2 * num][1] = curMetrix[2 * num + 1][1] = min;
        curPaths[2 * k].insert(curPaths[2 * k].end(), startCell.begin(), startCell.end());
        curPaths[2 * k].insert(curPaths[2 * k].end(), nextCell.begin(), nextCell.end());


        curPaths[2 * k + 1].clear();
        if (metrix[4 * k + 1][1] <= metrix[4 * k + 3][1])
        {
            min = metrix[4 * k + 1][1];
            startCell = tableState[4 * k + 1][0];
            nextCell = tableState[4 * k + 1][1];
        }
        else
        {
            min = metrix[4 * k + 3][1];
            startCell = tableState[4 * k + 3][0];
            nextCell = tableState[4 * k + 3][1];
        }
        num = binToDec(nextCell);
        curMetrix[2 * num][1] = curMetrix[2 * num + 1][1] = min;
        curPaths[2 * k + 1].insert(curPaths[2 * k + 1].end(), startCell.begin(), startCell.end());
        curPaths[2 * k + 1].insert(curPaths[2 * k + 1].end(), nextCell.begin(), nextCell.end());
    }
    metrix = curMetrix;
}

void setPaths() //поправить, потому что записываем два раза состояние из которого выходим
{
    if (paths[0].empty())
    {
        paths = curPaths;
        return;
    }

    int size = paths[0].size();
    int sizeCurPaths = curPaths[0].size();
    int sizePaths = paths.size();
    bool result;

    for (int q = 0; q < sizePaths; q++)
    {
        for (int j = 0; j < curPaths.size(); j++)
        {
            result = true;
            for (int k = 1; k < K; k++)
            {
                if (paths[q][size - k] != curPaths[j][K - 1 - k])
                    result = false;
            }
            if (result)
            {
                paths.push_back(paths[q]);
                paths.back().insert(paths.back().end(), curPaths[j].begin() + K - 1, curPaths[j].end());
                curPaths.erase(curPaths.begin() + j);
                j--;
            }
        }
    }
    paths.erase(paths.begin(), paths.begin() + sizePaths);
}

void findCode()
{
    bool result = true;
    int size = paths[0].size();
    resultCode.clear();

    for (int q = 0; q < paths.size(); q++)
    {
        result = true;
        for (int k = 1; k < K; k++)
        {
            if (paths[q][size - k] != finishState[K - 1 - k])
                result = false;
        }
        if (result)
        {
            for (int i = 1; i < paths[q].size() / (K - 1); i++)
            {
                resultCode.push_back(paths[q][(K - 1) * i]);
            }
            return;
        }
    }
}

void decoder(vector<bool> inputData)
{
    metrix.resize(tableState.size());
    int size = inputData.size() / 2;
    paths.clear();
    paths.resize(tableState.size() / 2);
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < tableState.size(); j++)
        {
            metrix[j].resize(2);
            metrix[j][0] = hamingDist(tableState[j][2], inputData[2 * i], inputData[2 * i + 1]);
            metrix[j][1] += metrix[j][0];
        }
        setMetrix();
        setPaths();
    }
    findCode();
}

// Пример использования:
int main(int argc, char* argv[]) {
    if (argc != 3)
    {
        std::cout << "Error! Arguments are bad!" << std::endl;
        for (int i = 0; i < argc; ++i)
            std::cout << argv[i] << std::endl;
        return 1;
    }
    std::string client_port = std::string(argv[1]);
    std::string server_port = std::string(argv[2]);

    // CLIENT 5556
    zmq::context_t context_client{1};
    zmq::context_t context_server{1};

    // construct a REQ (request) socket and connect to interface
    zmq::socket_t socket_client{context_client, zmq::socket_type::req};
    socket_client.connect("tcp://localhost:" + client_port);

    // SERVER 5555
    // construct a REP (reply) socket and bind to interface
    zmq::socket_t socket_server{context_server, zmq::socket_type::rep};
    socket_server.bind("tcp://*:" + server_port);

    zmq::message_t request;
    zmq::message_t reply;
    std::string coder_data;
    std::vector<bool> input_bits;
    std::string output;
    K = 3;
    G1 = {1,0,1};
    G2 = {1,1,1};
    createTable();

    for (auto request_num = 0; request_num < 10; ++request_num) 
    {
        // receive a request from coder
        socket_server.recv(request, zmq::recv_flags::none);
        coder_data = request.to_string();
        // send reply to coder
        socket_server.send(zmq::buffer("OK!"), zmq::send_flags::none);
        input_bits.resize(coder_data.size());

        std::cout << "DECODER. Run:" << request_num <<  "\n";

        for (uint32_t i = 0; i < coder_data.size(); ++i)
            input_bits[i] = (coder_data[i] == (char)1) ? true : false;

        decoder(input_bits);
        output.resize(resultCode.size());
        for(uint32_t i = 0; i < resultCode.size(); ++i)
            output[i] = resultCode[i] ? (char)1 : (char)0;
        std::cout << "DECODER. Count corrected error: " << minMetrix << std::endl;
        metrix.clear();
        // send data to scoreboard
        socket_client.send(zmq::buffer(output), zmq::send_flags::none);

        // get rep from scoreboard and continue
        socket_client.recv(reply, zmq::recv_flags::none);
    }
    socket_client.close();
    socket_server.close();

    return 0;
}