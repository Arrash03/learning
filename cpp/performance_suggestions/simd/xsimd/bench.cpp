#include <xsimd/xsimd.hpp>
#include <iostream>

int main() {
    using batch = xsimd::batch<float>;
    float a[8] = {1,2,3,4,5,6,7,8};
    float b[8] = {8,7,6,5,4,3,2,1};
    float c[8];

    batch va = batch::load_unaligned(a);
    batch vb = batch::load_unaligned(b);
    batch vc = va + vb;
    vc.store_unaligned(c);

    for (float f : c) std::cout << f << " ";
    std::cout << "\n";
}
