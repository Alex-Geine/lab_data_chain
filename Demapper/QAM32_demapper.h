#include <array>
#include <cmath>
#include <deque>
#include <iterator>
#include <numeric>
#include <vector>
#include <iostream>
#include <sstream>
#include "filter.hpp"

// #include "../cpp_utils/testing/utils.hpp"
#include "bit_packing.hpp"


class quadrature_receiver
{
    public:

    double f,fd;
    int i = 0;
    double phase = 0;

    iir_filter lpf_1;
    iir_filter lpf_2;

    std::vector<iir_filter> lpf_1_v;
    std::vector<iir_filter> lpf_2_v;


    public:

    quadrature_receiver(double f, double sample_rate, double symbol_speed) :
    lpf_1(design_butterworth_low_pass_filter(symbol_speed * 1, sample_rate)),
    lpf_2(design_butterworth_low_pass_filter(symbol_speed * 1, sample_rate)),
    lpf_1_v(4, design_butterworth_low_pass_filter(symbol_speed * 1, sample_rate)),
    lpf_2_v(4, design_butterworth_low_pass_filter(symbol_speed * 1, sample_rate)),
    f(f),
    fd(sample_rate)
    {}

    auto process(double v)
    {
        auto v_inphase = 2*v * cos(2 * M_PI * f * i / fd + phase);
        auto v_quadr = -2*v * sin(2 * M_PI * f * i / fd + phase);

        //v_inphase = lpf_1(v_inphase);
        //v_quadr = lpf_2(v_quadr);

        for (auto j = 0; j < lpf_1_v.size(); ++j)
        {
            v_inphase = lpf_1_v[j](v_inphase);
        }
        for (auto j = 0; j < lpf_2_v.size(); ++j)
        {
            v_quadr = lpf_2_v[j](v_quadr);
        }

        ++i;
        return std::array{v_inphase,v_quadr};
    }

    template <typename InputIt>
    auto process(InputIt begin, InputIt end)
    {
        auto s = std::distance(begin,end);
        std::vector<double> inphase(s);
        std::vector<double> quadrature(s);

        for (auto i = 0; i < s; ++i) {
            auto v = process(*(begin + i));
            inphase[i] = v[0];
            quadrature[i] = v[1];
        }

        return std::array{inphase,quadrature};
    }

    void correct_phase(double d_phase)
    {phase += d_phase;}
};

int binary_to_gray(int binary)
{
    return binary ^ (binary >> 1);
}

struct QAM32_demapper
{
    quadrature_receiver r;  
    double symbol_speed;
    double singular_amplitude = 1.0;
    
    QAM32_demapper(double f, double sample_rate,double symbol_speed) : r(f,sample_rate,symbol_speed) ,symbol_speed(symbol_speed) {}

    int process(double I, double Q)
    {
        I /= singular_amplitude;
        Q /= singular_amplitude;

        int i = std::floor(I / 2.0);
        int q = std::floor(Q / 2.0);

        static char map[6][6] =
        {
            {-1,0,1,17,16,-1},
            {4,12,8,24,28,20},
            {5,13,9,25,29,21},
            {7,15,11,27,31,23},
            {6,14,10,26,30,22},
            {-1,2,3,19,18,-1}
        };

        if ((i < -3) || (i > 2)) return -1;
        if ((q < -3) || (q > 2)) return -1;

        else return map[5 - (q + 3)][i + 3];
    }

    template <typename InputIt>
    auto process_preambule(InputIt begin, int n)
    {
        // auto s = std::distance(begin,end);

        double bit_segment_width = r.fd / double(symbol_speed);

        auto end = begin + 3 * n * bit_segment_width;

        auto IQ = r.process(begin,end);

        // int n = s / bit_segment_width;

        // std::vector<int> message(3*n);
        // for (auto i = 0; i < 3 * n; ++i)
        // {
        //     int index = offset + i * bit_segment_width + bit_segment_width / 2;
        //     auto I = IQ[0][index];
        //     auto Q = IQ[1][index];

        //     // message[i] = process(I,Q);
        // }

        double I = 0;
        double Q = 0;

        for (auto i = 0; i < n; ++i)
        {
            // int index = offset + i * bit_segment_width + bit_segment_width / 2;
            // message[i] = process(I,Q);

            I += (IQ[0][(3*i + 0.5) * bit_segment_width] + IQ[0][(3 * i + 1.5) * bit_segment_width] - IQ[0][(3 * i + 2.5) * bit_segment_width]) ;
            Q += (IQ[1][(3 * i + 0.5) * bit_segment_width] + IQ[1][(3 * i + 1.5) * bit_segment_width] - IQ[1][(3 * i + 2.5) * bit_segment_width]) ;
        }

        I /= double(3*n);
        Q /= double(3*n);


        //std::cout << hypot(I,Q) / sqrt(2) << std::endl;
        //std::cout << atan2(Q,I) - M_PI/4.0 << std::endl;

        // std::cout << "amplitude evaluated: " << hypot(I, Q) / sqrt(2) << std::endl;
        std::cout << "phase evaluated: " << atan2(Q, I) - M_PI / 4.0 << std::endl;
        // singular_amplitude = hypot(I,Q) / sqrt(2);
        r.phase = atan2(Q,I) - M_PI/4.0;

        return end;
    }


    template <typename InputIt>
    auto process(InputIt begin, InputIt end)
    {
        auto s = std::distance(begin,end);
        
        double bit_segment_width = r.fd / double(symbol_speed);
        
        auto IQ = r.process(begin,end);
        
        int n = s / bit_segment_width;
        int offset = 0;

        std::vector<int> message(n);

        for (auto i = 0; i < n; ++i)
        {
            int index = offset + i * bit_segment_width + bit_segment_width / 2;
            auto I = IQ[0][index];
            auto Q = IQ[1][index];

            message[i] = process(I,Q);
        }
        
        return message;
    }
};


void run(std::string& input_data, std::string& output_data)
{
    std::stringstream str(input_data);

    std::vector<double> v;

    double a{};
    while (str >> a)
    {
        v.push_back(a);
    }

    double sample_rate = 1000.0;
    double symbol_speed = 1;
    double f = 100.0;

    QAM32_demapper d(f,sample_rate,symbol_speed);

    double mod = sample_rate /double(symbol_speed);

    d.process_preambule(std::begin(v), 20);
    auto message = d.process(std::begin(v) + mod *3 * 20,std::end(v));
    std::vector<int> bits;
    bits = pack_bits_N_to_M<std::vector<int>>(5,1,message);

    output_data.resize(bits.size());
    for (uint32_t i = 0; i < bits.size(); ++i)
        output_data[i] = bits[i] ? 1 : 0;
}
