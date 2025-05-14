#include <vector>
#include <iostream>
#include <random>

class PayloadGenerator
{
    private:
    std::mt19937 generator;
    uint64_t mid_val;
    public:

    PayloadGenerator()
    {
        mid_val = (generator.max() - generator.min()) / 2;
    }

    void setSeed(uint64_t seed)
    {
        generator.seed(seed);
    }

    template <typename T>
    void getData(std::vector<T>& data)
    {
        for(auto& it:data)
            it = (generator() > mid_val) ? 1 : 0;
    }
};