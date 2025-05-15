#include "QAM32_demapper.h"
#include "../cpp_utils/testing/utils.hpp"
#include "../cpp_utils/math/fft.hpp"
#include <fstream>

int main()
{
    std::vector<double> signal(4096);
    // signal[0] = 1.;
    for (auto i = 0; i < 4096; ++i)
        signal[i] = (i % 128) > 64 ? 0:1;

    // filter f;
    // f.impulse = signal;
    // f.impulse = generate_sinc_filter(4096,20,1024.);


    iir_filter f1 = design_butterworth_low_pass_filter(400, 1024.);
    iir_filter f2 = design_butterworth_high_pass_filter(300, 1024.);

    signal = f1.process(signal);
    // signal = f2.process(signal);


    std::ofstream str("out.txt");
    utils::print(signal,"","\n",str);

    std::ofstream str2("out2.txt");
    utils::print(abs(fft(signal,-1)),"","\n",str2);


    std::vector<int> gray(32);
    for(auto i = 0; i < 32; ++i)
    {gray[i] = binary_to_gray(i);}

    utils::print(gray);
    
}