#!/usr/local/bin/seabang --debug
// Build as debug so the asserts will fire if I screw up.

// The follow doc was used for the 32 bit taps for a 2n-1 repetition time.
// https://docs.xilinx.com/v/u/en-US/xapp052
#include <iostream>
#include <assert.h>
#include <unistd.h>

// Using 32,22,2,1
// Rather anoyingly the document uses 1 based bit index. 
// So really using 31, 21, 1, 0
uint32_t static LFSR = 0;
static uint32_t GetLFSR()
{
    assert(LFSR != 0); // NO! Not allowed!

    // Make a new least significant bit from the taps.
    const uint32_t LSB = ((LFSR>>31) ^ (LFSR>>21) ^ (LFSR>>1) ^ (LFSR>>0)) & 1;

    // Shift current pattern right one bit.
    LFSR <<= 1;

    // Set the least significant but. Because of the shift above, LSB in the LFSR should not be zero.
    // So if the new bit is zero, will stay that way. Means we do not have to clear the bit first.
    LFSR |= LSB;

    return LFSR; // LFSR can never be zero, this function will never return zero.
}

int main(int argc, char *argv[])
{
// Say hello to the world!
    std::cout << "Linear Feedback Shift Register example for a 32bit register\n";
    std::cout << "The following sequence will take 4971 days to complete\n";

    const uint32_t START_VALUE = 1;// You could set this to rand() but it will fail it rand() returns zero.
    LFSR = START_VALUE;
    uint64_t itterations = 0;
    do
    {
        assert( itterations < 0xffffffff );
        GetLFSR();

        std::clog << LFSR%10; // As we're using modulo, we will get a zeros, which is good.

        itterations++;
        usleep(1000*100);
    }while(LFSR != START_VALUE);

    std::cout << "itterations == " << itterations << "\n";
    assert( itterations == 0xffffffff );
    return EXIT_SUCCESS;
}

