#ifndef MGD_DECODER_H
#define MGD_DECODER_H

#include <array>
#include <cstdint>
#include <cmath>
#include <vector>

class MGDDecoder {
    public:
    MGDDecoder(size_t baud_rate, bool is_frequency_hopping) : baud_rate(baud_rate), is_frequency_hopping(is_frequency_hopping) {}

    /**
     * @brief Decodes the grouped symbols using Modified Gray Decoding (MGD).
     * @param symbols The input symbols to be decoded.
     * @return A vector of decoded symbols.
     */
    std::vector<uint8_t> mgdDecode(const std::vector<uint8_t>& symbols) {
        std::vector<uint8_t> decodedSymbols;

        for (uint8_t symbol : symbols) {
            if (baud_rate == 2400 || baud_rate == 4800) {
                decodedSymbols.push_back(mgd8psk(symbol));
            } else if (baud_rate == 1200 || (baud_rate == 75 && !is_frequency_hopping)) {
                decodedSymbols.push_back(mgd4psk(symbol));
            } else {
                decodedSymbols.push_back(symbol);
            }
        }

        return decodedSymbols;
    }

    private:
    size_t baud_rate;
    bool is_frequency_hopping;

    int mgd8psk(int input) {
        static std::array<int, 8> lookupTable = {0, 1, 3, 2, 7, 6, 4, 5};
        return lookupTable[input];
    }

    int mgd4psk(int input) {
        static std::array<int, 4> lookupTable = {0, 1, 3, 2};
        return lookupTable[input];
    }
};

#endif