#ifndef SCRAMBLER_H
#define SCRAMBLER_H

#include <array>
#include <cstdint>
#include <stdexcept>
#include <vector>

/**
 * @class Scrambler
 * @brief A class that performs scrambling operations for data sequences.
 */
class Scrambler {
public:
    /**
     * @brief Constructor initializes the scrambler with a predefined register value.
     */
    Scrambler() : data_sequence_register(0x0BAD), symbol_count(0), preamble_table_index(0) {}

    /**
     * @brief Scrambles a synchronization preamble using a fixed randomizer sequence.
     * @param preamble The synchronization preamble to scramble.
     * @return The scrambled synchronization preamble.
     */
    std::vector<uint8_t> scrambleSyncPreamble(const std::vector<uint8_t>& preamble) {
        static const std::array<uint8_t, 32> sync_randomizer_sequence = {
            7, 4, 3, 0, 5, 1, 5, 0, 2, 2, 1, 1,
            5, 7, 4, 3, 5, 0, 2, 6, 2, 1, 6, 2,
            0, 0, 5, 0, 5, 2, 6, 6
        };

        std::vector<uint8_t> scrambled_preamble;
        scrambled_preamble.reserve(preamble.size());  // Preallocate to improve efficiency

        for (size_t i = 0; i < preamble.size(); ++i) {
            uint8_t scrambled_value = (preamble[i] + sync_randomizer_sequence[preamble_table_index]) % 8;
            scrambled_preamble.push_back(scrambled_value);
            preamble_table_index = (preamble_table_index + 1) % sync_randomizer_sequence.size();
        }

        return scrambled_preamble;
    }

    /**
     * @brief Scrambles data using a pseudo-random sequence generated from an LFSR.
     * @param data The data to scramble.
     * @return The scrambled data.
     */
    std::vector<uint8_t> scrambleData(const std::vector<uint8_t>& data) {
        std::vector<uint8_t> scrambled_data;
        scrambled_data.reserve(data.size());  // Preallocate to improve efficiency

        for (size_t i = 0; i < data.size(); ++i) {
            uint8_t random_value = getNextRandomValue();
            uint8_t scrambled_value = (data[i] + random_value) % 8;
            scrambled_data.push_back(scrambled_value);
        }

        return scrambled_data;
    }

private:
    uint16_t data_sequence_register;
    size_t symbol_count;
    size_t preamble_table_index;

    /**
     * @brief Generates the next value from the data sequence randomizing generator.
     * @return A 3-bit random value (0-7) from the current state of the LFSR.
     */
    uint8_t getNextRandomValue() {
        // Reset the LFSR state every 160 symbols, as per the specification
        if (symbol_count >= 160) {
            data_sequence_register = 0x0BAD;  // Load initial value "BAD" (hex)
            symbol_count = 0;
        }
        symbol_count++;

        uint8_t output = data_sequence_register & 0x07;  // Extract the lower 3 bits

        // Perform 8 shifts to generate the next 3-bit random value
        for (int i = 0; i < 8; ++i) {
            // Extract the MSB (bit 11 in a 12-bit register)
            uint16_t msb = (data_sequence_register & 0x0800) ? 1 : 0;  // Bit 11 is 0x0800

            // Shift the register left by 1 bit
            data_sequence_register <<= 1;

            // Apply feedback if MSB was set
            if (msb) {
                // XOR with the tapped bits (positions 1, 4, and 6)
                // Tap mask is 0x052 (bits 1, 4, 6): taps at positions 1, 4, 6 affect the feedback
                data_sequence_register ^= 0x052;
            }

            // Ensure the LFSR stays within 12 bits
            data_sequence_register &= 0x0FFF;
        }

        return output;
    }

};

#endif
