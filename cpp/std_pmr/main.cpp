#include <memory_resource>
#include <vector>
#include <iostream>

int main() {
    // Allocate a large buffer on the stack
    char buffer[1024 * 1024]; // 1MB buffer
    
    // Create a monotonic resource using our buffer
    std::pmr::monotonic_buffer_resource pool{
        std::data(buffer), std::size(buffer)};
    
    // Create PMR containers using this pool
    std::pmr::vector<int> vec(&pool);
    std::pmr::string str(&pool);
    
    // Allocations come from our buffer until exhausted
    for (int i = 0; i < 100000; ++i) {
        vec.push_back(i);
		std::cout << vec.size() << std::endl;
        str.append("a");
    }
    
    // No need to deallocate - buffer is cleared when pool goes out of scope
}
