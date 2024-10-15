#ifndef PSK_MODULATOR_H
#define PSK_MODULATOR_H


#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <cstdint>
#include <numeric>
#include <stdexcept>
#include <vector>
#include <fftw3.h>
#include <map>
#include <tuple>

#include "costasloop.h"
#include "filters.h"
#include "Scrambler.h"

static constexpr double CARRIER_FREQ = 1800.0;
static constexpr size_t SYMBOL_RATE = 2400;
static constexpr double ROLLOFF_FACTOR = 0.35;
static constexpr double SCALE_FACTOR = 32767.0;

class PSKModulator {
public:
    PSKModulator(const double _sample_rate, const bool _is_frequency_hopping, const size_t _num_taps) 
        : sample_rate(validateSampleRate(_sample_rate)), gain(1.0/sqrt(2.0)), is_frequency_hopping(_is_frequency_hopping), samples_per_symbol(static_cast<size_t>(sample_rate / SYMBOL_RATE)), srrc_filter(48, _sample_rate, SYMBOL_RATE, ROLLOFF_FACTOR) {
            initializeSymbolMap();
            phase_detector = PhaseDetector(symbolMap);
    }

    std::vector<int16_t> modulate(const std::vector<uint8_t>& symbols) {
        std::vector<double> baseband_I(symbols.size() * samples_per_symbol);
        std::vector<double> baseband_Q(symbols.size() * samples_per_symbol);
        std::vector<std::complex<double>> baseband_components(symbols.size() * samples_per_symbol);
        size_t symbol_index = 0;

        for (const auto& symbol : symbols) {
            if (symbol >= symbolMap.size()) {
                throw std::out_of_range("Invalid symbol value for 8-PSK modulation. Symbol must be between 0 and 7.");
            }
            const std::complex<double> target_symbol = symbolMap[symbol];

            for (size_t i = 0; i < samples_per_symbol; ++i) {
                baseband_components[symbol_index * samples_per_symbol + i] = target_symbol;
            }

            symbol_index++;
        }

        // Filter the I/Q phase components
        std::vector<std::complex<double>> filtered_components = srrc_filter.applyFilter(baseband_components);

        // Combine the I and Q components
        std::vector<double> passband_signal;
        passband_signal.reserve(baseband_components.size());

        double carrier_phase = 0.0;
        double carrier_phase_increment = 2 * M_PI * CARRIER_FREQ / sample_rate;
        for (const auto& sample : baseband_components) {
            double carrier_cos = std::cos(carrier_phase);
            double carrier_sin = -std::sin(carrier_phase);
            double passband_value = sample.real() * carrier_cos + sample.imag() * carrier_sin;
            passband_signal.emplace_back(passband_value * 32767.0); // Scale to int16_t
            carrier_phase += carrier_phase_increment;
            if (carrier_phase >= 2 * M_PI)
                carrier_phase -= 2 * M_PI;
        }

        std::vector<int16_t> final_signal;
        final_signal.reserve(passband_signal.size());

        for (const auto& sample : passband_signal) {
            int16_t value = static_cast<int16_t>(sample);
            value = std::clamp(value, (int16_t)-32768, (int16_t)32767);
            final_signal.emplace_back(value);
        }

        return final_signal;
    }

    std::vector<uint8_t> demodulate(const std::vector<int16_t> passband_signal, size_t& baud_rate, size_t& interleave_setting, bool& is_voice) {
        // Carrier recovery. initialize the Costas loop.
        CostasLoop costas_loop(CARRIER_FREQ, sample_rate, symbolMap, 5.0, 0.05, 0.01);

        // Convert passband signal to doubles.
        std::vector<double> normalized_passband(passband_signal.size());
        for (size_t i = 0; i < passband_signal.size(); i++) {
            normalized_passband[i] = passband_signal[i] / 32767.0;
        }

        // Downmix passband to baseband
        std::vector<std::complex<double>> baseband_IQ = costas_loop.process(normalized_passband);
        std::vector<uint8_t> detected_symbols;

        // Phase detection and symbol formation
        size_t samples_per_symbol = sample_rate / SYMBOL_RATE;
        bool sync_found = false;
        size_t sync_segments_detected;

        size_t window_size = 32*15;
        
        for (size_t i = 0; i < baseband_IQ.size(); i += samples_per_symbol) {
            std::complex<double> symbol_avg(0.0, 0.0);
            for (size_t j = 0; j < samples_per_symbol; j++) {
                symbol_avg += baseband_IQ[i + j];
            }
            symbol_avg /= static_cast<double>(samples_per_symbol);

            uint8_t detected_symbol = phase_detector.getSymbol(symbol_avg);
            detected_symbols.push_back(detected_symbol);
        }

        if (processSyncSegments(detected_symbols, baud_rate, interleave_setting, is_voice)) {
            return processDataSymbols(detected_symbols);
        }

    }

private:
    const double sample_rate;        ///< The sample rate of the system.
    const double gain;               ///< The gain of the modulated signal.
    size_t samples_per_symbol; ///< Number of samples per symbol, calculated to match symbol duration with cycle.
    PhaseDetector phase_detector;
    SRRCFilter srrc_filter;
    std::vector<std::complex<double>> symbolMap;  ///< The mapping of tribit symbols to I/Q components.
    const bool is_frequency_hopping;                    ///< Whether to use frequency hopping methods. Not implemented (yet?)
    

    static inline double validateSampleRate(const double rate) {
        if (rate <= 2 * CARRIER_FREQ) {
            throw std::out_of_range("Sample rate must be above the Nyquist frequency (PSKModulator.h)");
        }
        return rate;
    }

    inline void initializeSymbolMap() {
        symbolMap = {
            {gain * std::cos(2.0*M_PI*(0.0/8.0)), gain * std::sin(2.0*M_PI*(0.0/8.0))}, // 0 (000) corresponds to I = 1.0, Q = 0.0
            {gain * std::cos(2.0*M_PI*(1.0/8.0)), gain * std::sin(2.0*M_PI*(1.0/8.0))}, // 1 (001) corresponds to I = cos(45), Q = sin(45)
            {gain * std::cos(2.0*M_PI*(2.0/8.0)), gain * std::sin(2.0*M_PI*(2.0/8.0))}, // 2 (010) corresponds to I = 0.0, Q = 1.0
            {gain * std::cos(2.0*M_PI*(3.0/8.0)), gain * std::sin(2.0*M_PI*(3.0/8.0))}, // 3 (011) corresponds to I = cos(135), Q = sin(135)
            {gain * std::cos(2.0*M_PI*(4.0/8.0)), gain * std::sin(2.0*M_PI*(4.0/8.0))}, // 4 (100) corresponds to I = -1.0, Q = 0.0
            {gain * std::cos(2.0*M_PI*(5.0/8.0)), gain * std::sin(2.0*M_PI*(5.0/8.0))}, // 5 (101) corresponds to I = cos(225), Q = sin(225)
            {gain * std::cos(2.0*M_PI*(6.0/8.0)), gain * std::sin(2.0*M_PI*(6.0/8.0))}, // 6 (110) corresponds to I = 0.0, Q = -1.0
            {gain * std::cos(2.0*M_PI*(7.0/8.0)), gain * std::sin(2.0*M_PI*(7.0/8.0))}  // 7 (111) corresponds to I = cos(315), Q = sin(315)
        };
    }

    uint8_t extractBestTribit(const std::vector<uint8_t>& stream, const size_t start, const size_t window_size) const {
        if (start + window_size > stream.size()) {
            throw std::out_of_range("Window size exceeds symbol stream size.");
        }

        Scrambler scrambler;
        std::vector<uint8_t> symbol(stream.begin() + start, stream.begin() + start + window_size);
        std::vector<uint8_t> descrambled_symbol = scrambler.scrambleSyncPreamble(symbol);

        const size_t split_len = window_size / 4;
        std::array<uint8_t, 8> tribit_counts = {0};  // Counts for each channel symbol (000 to 111)

        // Loop through each split segment (4 segments)
        for (size_t i = 0; i < 4; ++i) {
            // Extract the range for this split
            size_t segment_start = start + i * split_len;
            size_t segment_end = segment_start + split_len;

            // Compare this segment to the predefined patterns from the table and map to a channel symbol
            uint8_t tribit_value = mapSegmentToChannelSymbol(descrambled_symbol, segment_start, segment_end);

            // Increment the corresponding channel symbol count
            tribit_counts[tribit_value]++;
        }

        // Find the channel symbol with the highest count (majority vote)
        uint8_t best_symbol = std::distance(tribit_counts.begin(), std::max_element(tribit_counts.begin(), tribit_counts.end()));

        return best_symbol;
    }

    // Function to map a segment of the stream back to a channel symbol based on the repeating patterns
    uint8_t mapSegmentToChannelSymbol(const std::vector<uint8_t>& segment, size_t start, size_t end) const {
        std::vector<uint8_t> extracted_pattern(segment.begin() + start, segment.begin() + end);

        // Compare the extracted pattern with known patterns from the table
        if (matchesPattern(extracted_pattern, {0, 0, 0, 0, 0, 0, 0, 0})) return 0b000;
        if (matchesPattern(extracted_pattern, {0, 4, 0, 4, 0, 4, 0, 4})) return 0b001;
        if (matchesPattern(extracted_pattern, {0, 0, 4, 4, 0, 0, 4, 4})) return 0b010;
        if (matchesPattern(extracted_pattern, {0, 4, 4, 0, 0, 4, 4, 0})) return 0b011;
        if (matchesPattern(extracted_pattern, {0, 0, 0, 0, 4, 4, 4, 4})) return 0b100;
        if (matchesPattern(extracted_pattern, {0, 4, 0, 4, 4, 0, 4, 0})) return 0b101;
        if (matchesPattern(extracted_pattern, {0, 0, 4, 4, 4, 4, 0, 0})) return 0b110;
        if (matchesPattern(extracted_pattern, {0, 4, 4, 0, 4, 0, 0, 4})) return 0b111;

        throw std::invalid_argument("Invalid segment pattern");
    }

    // Helper function to compare two patterns
    bool matchesPattern(const std::vector<uint8_t>& segment, const std::vector<uint8_t>& pattern) const {
        return std::equal(segment.begin(), segment.end(), pattern.begin());
    }

    bool configureModem(uint8_t D1, uint8_t D2, size_t& baud_rate, size_t& interleave_setting, bool& is_voice) {
        // Predefine all the valid combinations in a lookup map
        static const std::map<std::pair<uint8_t, uint8_t>, std::tuple<size_t, size_t, bool>> modemConfig = {
            {{7, 6}, {4800, 1, false}},  // 4800 bps
            {{7, 7}, {2400, 1, true}},   // 2400 bps, voice
            {{6, 4}, {2400, 1, false}},  // 2400 bps, data
            {{6, 5}, {1200, 1, false}},  // 1200 bps
            {{6, 6}, {600,  1, false}},  // 600 bps
            {{6, 7}, {300,  1, false}},  // 300 bps
            {{7, 4}, {150,  1, false}},  // 150 bps
            {{7, 5}, {75,   1, false}},  // 75 bps
            {{4, 4}, {2400, 2, false}},  // 2400 bps, long interleave
            {{4, 5}, {1200, 2, false}},  // 1200 bps, long interleave
            {{4, 6}, {600,  2, false}},  // 600 bps, long interleave
            {{4, 7}, {300,  2, false}},  // 300 bps, long interleave
            {{5, 4}, {150,  2, false}},  // 150 bps, long interleave
            {{5, 5}, {75,   2, false}},  // 75 bps, long interleave
        };

        // Use D1 and D2 to look up the correct configuration
        auto it = modemConfig.find({D1, D2});
        if (it != modemConfig.end()) {
            // Set the parameters if found
            std::tie(baud_rate, interleave_setting, is_voice) = it->second;
            return true;
        } else {
            return false;
        }
    }

    uint8_t calculateSegmentCount(const uint8_t C1, const uint8_t C2, const uint8_t C3) {
        uint8_t extracted_C1 = C1 & 0b11;
        uint8_t extracted_C2 = C2 & 0b11;
        uint8_t extracted_C3 = C3 & 0b11;
        
        uint8_t segment_count = (extracted_C1 << 4) | (extracted_C2 << 2) | extracted_C3;
        return segment_count;
    }

    bool processSegment(const std::vector<uint8_t>& detected_symbols, size_t& start, size_t symbol_size, size_t& segment_count, uint8_t& D1, uint8_t& D2) {
        size_t sync_pattern_length = 9;

        if (start + symbol_size * sync_pattern_length > detected_symbols.size()) {
            start = detected_symbols.size();
            return false;
        }

        std::vector<uint8_t> window(detected_symbols.begin() + start, detected_symbols.begin() + start + sync_pattern_length * symbol_size);
        std::vector<uint8_t> extracted_window;
        for (size_t i = 0; i < sync_pattern_length; i++) {
            extracted_window.push_back(extractBestTribit(window, i * symbol_size, symbol_size));
        }

        if (!matchesPattern(extracted_window, {0, 1, 3, 0, 1, 3, 1, 2, 0})) {
            start += symbol_size;
            return false;
        }

        start += sync_pattern_length * symbol_size;
        size_t D1_index = start + symbol_size;
        size_t D2_index = D1_index + symbol_size;

        if (D2_index + symbol_size > detected_symbols.size()) {
            start = detected_symbols.size();
            return false;
        }

        D1 = extractBestTribit(detected_symbols, D1_index, symbol_size);
        D2 = extractBestTribit(detected_symbols, D2_index, symbol_size);

        // Process the count symbols (C1, C2, C3)
        size_t C1_index = D2_index + symbol_size;
        size_t C2_index = C1_index + symbol_size;
        size_t C3_index = C2_index + symbol_size;

        if (C3_index + symbol_size > detected_symbols.size()) {
            start = detected_symbols.size();
            return false;
        }

        uint8_t C1 = extractBestTribit(detected_symbols, C1_index, symbol_size);
        uint8_t C2 = extractBestTribit(detected_symbols, C2_index, symbol_size);
        uint8_t C3 = extractBestTribit(detected_symbols, C3_index, symbol_size);

        segment_count = calculateSegmentCount(C1, C2, C3);

        // Check for the constant zero pattern
        size_t constant_zero_index = C3_index + symbol_size;

        if (constant_zero_index + symbol_size > detected_symbols.size()) {
            start = detected_symbols.size();
            return false;
        }
        uint8_t constant_zero = extractBestTribit(detected_symbols, constant_zero_index, symbol_size);

        if (constant_zero != 0) {
            start = constant_zero_index + symbol_size;
            return false; // Failed zero check, resync
        }

        // Successfully processed the segment
        start = constant_zero_index + symbol_size; // Move start to next segment
        return true;
    }

    bool processSyncSegments(const std::vector<uint8_t>& detected_symbols, size_t& baud_rate, size_t& interleave_setting, bool& is_voice) {
        size_t symbol_size = 32;
        size_t start = 0;
        size_t segment_count = 0;
        std::map<std::pair<uint8_t, uint8_t>, int> vote_map;
        const int short_interleave_threshold = 2;
        const int long_interleave_threshold = 5;

        // Attempt to detect interleave setting dynamically
        bool interleave_detected = false;
        int current_threshold = short_interleave_threshold;  // Start by assuming short interleave

        while (start + symbol_size * 15 < detected_symbols.size()) {
            uint8_t D1 = 0, D2 = 0;
            if (processSegment(detected_symbols, start, symbol_size, segment_count, D1, D2)) {
                vote_map[{D1, D2}]++;

                // Check if we have enough votes to make a decision based on current interleave assumption
                if (vote_map.size() >= current_threshold) {
                    auto majority_vote = std::max_element(vote_map.begin(), vote_map.end(), [](const auto& a, const auto& b) { return a.second < b.second; });

                    if (configureModem(majority_vote->first.first, majority_vote->first.second, baud_rate, interleave_setting, is_voice)) {
                        interleave_detected = true;
                        break;  // Successfully configured modem, exit loop
                    } else {
                        // If configuration fails, retry with the other interleave type
                        if (current_threshold == short_interleave_threshold) {
                            current_threshold = long_interleave_threshold;  // Switch to long interleave
                            vote_map.clear();  // Clear the vote map and start fresh
                            start = 0;  // Restart segment processing
                        } else {
                            continue;  // Both short and long interleave attempts failed, signal is not usable
                        }
                    }
                }

                if (segment_count > 0) {
                    while (segment_count > 0 && start < detected_symbols.size()) {
                        uint8_t dummy_D1, dummy_D2;
                        if (!processSegment(detected_symbols, start, symbol_size, segment_count, dummy_D1, dummy_D2)) {
                            continue;
                        }
                    }
                }
            } else {
                start += symbol_size;  // Move to the next segment
            }
        }

        return interleave_detected;
    }

    std::vector<uint8_t> processDataSymbols(const std::vector<uint8_t>& detected_symbols) {
        return std::vector<uint8_t>();
    }
};

#endif