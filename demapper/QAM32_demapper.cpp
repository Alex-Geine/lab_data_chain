#include "QAM32_demapper.h"
#include <armadillo>
#include <fstream>
#include "../cpp_utils/testing/utils.hpp"
#include "../cpp_utils/testing/bit_packing.hpp"
#include <random>

using namespace arma;

int main()
{
    std::random_device rd;
    double sample_rate = 1000.0;
    double symbol_speed = 1;
    double f = 100.0;

    std::ifstream in("Output_Signal.txt");

    std::vector<double> v;

    while (!in.eof())
    {
        double a; in >> a;
        v.push_back(a);
    }

    // std::vector<double> v(sample_rate*32);
    double mod = sample_rate /double(symbol_speed);

    // int k = 0;
    // for (auto i = 0; i < v.size(); ++i)
    // {
    //     double x1 = -5, y1 =  3;
    //     double x2 =  5, y2 = -3;

    //     double a;
    //     double phase;
    //     if (fmod(i,2*mod) < (mod))
    //     {
    //         phase = atan2(y1,x1);
    //         a = std::sqrt(x1*x1 + y1*y1);
    //     }
    //     else
    //     {
    //         phase = atan2(y2,x2);
    //         a = std::sqrt(x2*x2 + y2*y2);
    //     }

    //     if (i / mod < 3) {a = sqrt(2); phase = -3*M_PI_4;}
    //     if (i / mod < 2) {a = sqrt(2); phase = M_PI_4;}


    //     v[i] = a*sin(2 * M_PI * f * i / sample_rate + phase);
    //     v[i] += std::normal_distribution<double>(0,0.1)(rd);
    // }


    QAM32_demapper d(f,sample_rate,symbol_speed);

    // auto message_begin = d.process_preambule(std::begin(v), 10);
    // auto message = d.process(message_begin,std::end(v));
    // auto message = d.process(std::begin(v),std::end(v));
    auto message = d.process(std::begin(v) + mod *3 * 10,std::end(v));

    utils::print(message);
    utils::print(pack_bits_N_to_M(5,1,message));
    utils::print(pack_bits_N_to_M(1,5,pack_bits_N_to_M(5,1,message)));



    quadrature_receiver r(f,sample_rate,symbol_speed);
    auto IQ = r.process(std::begin(v),std::end(v));


    std::ofstream str("constellation.txt");

    double symbol_length = sample_rate / double(symbol_speed);
    for (double i = symbol_length / 2.0; i < IQ[0].size(); i += symbol_length)
    {
        str << IQ[0][i] << ' ' << IQ[1][i] << std::endl;
    }

}
