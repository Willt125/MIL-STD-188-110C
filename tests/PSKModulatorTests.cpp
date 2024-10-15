#include <gtest/gtest.h>
#include "modulation/PSKModulator.h"

// Fixture for PSK Modulator tests
class PSKModulatorTest : public ::testing::Test {
protected:
    double sample_rate = 48000;
    size_t num_taps = 48;
    bool is_frequency_hopping = false;
    PSKModulator modulator{sample_rate, is_frequency_hopping, num_taps};

    std::vector<uint8_t> symbols = {0, 3, 5, 7};
};

TEST_F(PSKModulatorTest, ModulationOutputLength) {
    auto signal = modulator.modulate(symbols);

    size_t expected_length = symbols.size() * (sample_rate / SYMBOL_RATE);
    ASSERT_EQ(signal.size(), expected_length);

    for (auto& sample : signal) {
        EXPECT_GE(sample, -32768);
        EXPECT_LE(sample, 32767);
    }
}

TEST_F(PSKModulatorTest, DemodulationOutput) {
    auto passband_signal = modulator.modulate(symbols);

    // Debug: Print modulated passband signal
    std::cout << "Modulated Passband Signal: ";
    for (const auto& sample : passband_signal) {
        std::cout << sample << " ";
    }
    std::cout << std::endl;

    auto decoded_symbols = modulator.demodulate(passband_signal);

    // Debug: Print decoded symbols
    std::cout << "Decoded Symbols: ";
    for (const auto& symbol : decoded_symbols) {
        std::cout << (int)symbol << " ";
    }
    std::cout << std::endl;

    // Debug: Print expected symbols
    std::cout << "Expected Symbols: ";
    for (const auto& symbol : symbols) {
        std::cout << (int)symbol << " ";
    }
    std::cout << std::endl;

    ASSERT_EQ(symbols.size(), decoded_symbols.size());

    for (size_t i = 0; i < symbols.size(); i++) {
        EXPECT_EQ(symbols[i], decoded_symbols[i]) << " at index " << i;
    }
}


TEST_F(PSKModulatorTest, InvalidSymbolInput) {
    std::vector<uint8_t> invalid_symbols = {0, 8, 9};

    EXPECT_THROW(modulator.modulate(invalid_symbols), std::out_of_range);
}