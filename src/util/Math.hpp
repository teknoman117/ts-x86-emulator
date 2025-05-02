#ifndef MATH_HPP
#define MATH_HPP

#include <cstddef>
#include <climits>

namespace {
    // Rounds up to the next power of 2
    size_t roundUpToPowerOfTwo(size_t n) {
        // If n is already a power of 2, return it
        // This check can be removed since the problem assumes n is not a power of 2
        
        // Calculate number of bits in size_t
        const int numBits = sizeof(size_t) * CHAR_BIT;
        
        // Find the position of the highest bit set
        int position = 0;
        for (int i = numBits - 1; i >= 0; i--) {
            if (n & (static_cast<size_t>(1) << i)) {
                position = i;
                break;
            }
        }
        
        // If there are any bits set below the highest bit,
        // then we need to round up to the next power of 2
        for (int i = 0; i < position; i++) {
            if (n & (static_cast<size_t>(1) << i)) {
                // Return 2^(position+1)
                return static_cast<size_t>(1) << (position + 1);
            }
        }
        
        // n is already a power of 2 (shouldn't happen given the problem assumption)
        return n;
    }
}

#endif /* MATH_HPP */