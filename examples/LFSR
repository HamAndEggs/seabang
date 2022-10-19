#!/usr/local/bin/seabang
// Build as debug so the asserts will fire if I screw up.

// The follow doc was used for the 32 bit taps for a 2n-1 repetition time.
// https://docs.xilinx.com/v/u/en-US/xapp052
#include <iostream>
#include <assert.h>
#include <unistd.h>
#include <cstring>

uint32_t static LFSR_A = 0;
uint32_t static LFSR_B = 0;

static void UpdateLFSR(uint32_t &LFSR)
{
// Using 32,22,2,1
// Rather anoyingly the document uses 1 based bit index. 
// So really using 31, 21, 1, 0
    assert(LFSR != 0); // NO! Not allowed!

    // Make a new least significant bit from the taps.
    const uint32_t LSB = ((LFSR>>31) ^ (LFSR>>21) ^ (LFSR>>1) ^ (LFSR>>0)) & 1;

    // Shift current pattern right one bit.
    LFSR <<= 1;

    // Set the least significant but. Because of the shift above, LSB in the LFSR should be zero.
    // So if the new bit is zero, will stay that way. Means we do not have to clear the bit first.
    LFSR |= LSB;
}

// Uses two LFSR's cycling at a different rate so that you can not discover their state from sampling the results.
static uint32_t GetLFSR()
{
    UpdateLFSR(LFSR_A);
    UpdateLFSR(LFSR_B);
    UpdateLFSR(LFSR_B);

    return LFSR_A ^ LFSR_B;
}


int main(int argc, char *argv[])
{
// Say hello to the world!

    std::cout << "Linear Feedback Shift Register example for a 32bit register\n";
    std::cout << "The following sequence will take 136 years to repeat\n";

    LFSR_A = 0x00000001;
    LFSR_B = 0xF00F000F;

    do
    {
        const uint32_t LFSR = GetLFSR();

        const uint32_t unit = LFSR%10; // As we're using modulo, we will get a zeros, which is good.
        std::clog << unit;
        sleep(1);
    }while( true );


    return EXIT_SUCCESS;
}

