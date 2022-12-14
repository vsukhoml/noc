#include <math.h>
#include <stdint.h>

// Code adapted from paper "Efficient Floating - Point Square Root and
// Reciprocal Square Root Algorithms"
// https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=9660872

// These are based on the fast inverse square root and the second convergence
// order Householder methods. This method makes it possible to obtain highly
// accurate results after two iterations for the float and after three
// iterations for the double codes.
// The optimal parameters of the iterative process are calculated by balancing
// the values of the positive and negative errors. In comparison with known
// algorithms, the proposed algorithms reduce the error during the first
// iteration. For single accuracy, 23.67 correct bits are obtained during the
// second iteration and for double accuracy 52.00 correct bits are obtained
// during the third iteration. The same results are obtained during the square
// root calculation. For single accuracy, 23.43 correct bitsare obtained during
// the second iteration and for double accuracy 52.00 correct bits are obtained
// during the third iteration. An algorithm has been developed that performs one
// multiplication operation less without losing accuracy. This increases the
// speed of computing the reciprocal of the square root by 12 %.
float sqrtf(float x) {
    union {
        float f;
        uint32_t i;
    } cast;
    const uint32_t mk = 0x5f1110a0;
    const float k1 = 2.2825186f;
    const float k2 = 2.2533049f;
    cast.f = x;
    cast.i = mk - (cast.i >> 1);
    float y = cast.f;
    float c = x * y * y;
    y = y * (k1 - c * (k2 - c));
    c = x * y;
    y = 1.0f - y * c;
    y = 0.5f * c * y + c;
    return y;
}

double sqrt(double x) {
    union {
        double f;
        uint64_t i;
    } cast;
    const uint64_t mk = 0x5fe222142565d5c2;
    double k1, k2, k3;
    k1 = 2.28251863069107890;
    k2 = 2.25330495841181303;
    k3 = 1.500000000304;
    cast.f = x;
    cast.i = mk - (cast.i >> 1);
    double y = cast.f;
    double c = x * y * y;
    y = y * (k1 - c * (k2 - c));
    double xh = 0.5 * x;
    y = y * (k3 - xh * y * y);
    c = xh * y;
    y = 0.5 - y * c;
    y = 2.0 * (c * y + c);
    return y;
}
