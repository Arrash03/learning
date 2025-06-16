#include <iostream>
#include <immintrin.h>  // Header for AVX intrinsics
#include <vector>
#include <chrono>

// Function to add two arrays using SIMD (AVX2)
void vector_add_simd(const float* a, const float* b, float* c, size_t size) {
    // AVX2 processes 8 floats at a time (256-bit registers)
    constexpr size_t simd_width = 8;
    size_t i = 0;
    
    // Process elements in chunks of 8
    for (; i + simd_width <= size; i += simd_width) {
        // Load 8 floats from array a and b
        __m256 va = _mm256_loadu_ps(a + i);
        __m256 vb = _mm256_loadu_ps(b + i);
        
        // Add them together
        __m256 vc = _mm256_add_ps(va, vb);
        
        // Store the result
        _mm256_storeu_ps(c + i, vc);
    }
    
    // Process remaining elements (if any)
    for (; i < size; ++i) {
        c[i] = a[i] + b[i];
    }
}

// Regular scalar addition for comparison
void vector_add_scalar(const float* a, const float* b, float* c, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        c[i] = a[i] + b[i];
    }
}

int main() {
    const size_t size = 1024 * 1024;  // 1 million elements
    std::vector<float> a(size, 1.0f);  // Initialize with 1.0
    std::vector<float> b(size, 2.0f);  // Initialize with 2.0
    std::vector<float> c(size);
    
    // Time SIMD version
    auto start = std::chrono::high_resolution_clock::now();
    vector_add_simd(a.data(), b.data(), c.data(), size);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> simd_time = end - start;
    
    // Time scalar version
    start = std::chrono::high_resolution_clock::now();
    vector_add_scalar(a.data(), b.data(), c.data(), size);
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> scalar_time = end - start;
    
    // Verify results
    for (size_t i = 0; i < 10; ++i) {  // Check first 10 elements
        std::cout << "c[" << i << "] = " << c[i] << std::endl;
    }
    
    // Print timing results
    std::cout << "SIMD time: " << simd_time.count() << " seconds\n";
    std::cout << "Scalar time: " << scalar_time.count() << " seconds\n";
    std::cout << "Speedup: " << scalar_time.count() / simd_time.count() << "x\n";
    
    return 0;
}
