#ifndef BIT_PACKING_HPP
#define BIT_PACKING_HPP

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>

template <typename T>
auto ceil_div(T a, T b)
{
    return a / b + (a % b != 0? 1 : 0);
}

inline int lowbit_mask(int n)
{
    if (n >= 32) return UINT32_MAX;
    return (1u << n) -1;
}

template <typename InputIt, typename OutputIt>
void pack_bits_N_to_M(size_t N, size_t M , InputIt begin, InputIt end, OutputIt output)
{
    assert(N <= 24 && M <= 24);

    uint32_t bit_buffer = 0;
    int bit_count = 0;

    auto count = std::distance(begin,end);

    for (auto it = begin; it != end; ++it)
    {
        bit_buffer |= ((*it & lowbit_mask(N)) << bit_count);
        bit_count += N;
        
        while (bit_count >= M) {
            *(output++) = bit_buffer & lowbit_mask(M);
            bit_buffer >>= M;
            bit_count -= M;
        }
    }

    if (bit_count > 0)
    {
        *(output++) = bit_buffer & lowbit_mask(M);
    }
}



template <typename ContainerIn, typename ContainerOut = ContainerIn>
ContainerOut pack_bits_N_to_M(size_t N, size_t M, ContainerIn c)
{
    size_t size = ceil_div(N * std::size(c), M);
    ContainerOut o(size);

    pack_bits_N_to_M(N,M,std::begin(c),std::end(c),std::begin(o));
    return o;
}

#endif // BIT_PACKING_HPP


