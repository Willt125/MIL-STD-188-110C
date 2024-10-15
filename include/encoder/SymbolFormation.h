#ifndef SYMBOLFORMATION_H
#define SYMBOLFORMATION_H

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

#include "Scrambler.h"

std::vector<uint8_t> baud75_exceptional_0 = {0, 0, 0, 0, 4, 4, 4, 4};
std::vector<uint8_t> baud75_exceptional_1 = {0, 4, 0, 4, 4, 0, 4, 0};
std::vector<uint8_t> baud75_exceptional_2 = {0, 0, 4, 4, 4, 4, 0, 0};
std::vector<uint8_t> baud75_exceptional_3 = {0, 4, 4, 0, 4, 0, 0, 4};
std::vector<uint8_t> baud75_normal_0 = {0, 0, 0, 0};
std::vector<uint8_t> baud75_normal_1 = {0, 4, 0, 4};
std::vector<uint8_t> baud75_normal_2 = {0, 0, 4, 4};
std::vector<uint8_t> baud75_normal_3 = {0, 4, 4, 0};

class SymbolFormation {
    public:
    SymbolFormation(size_t baud_rate, size_t interleave_setting, bool is_voice, bool is_frequency_hopping) : interleave_setting(interleave_setting), baud_rate(baud_rate), is_voice(is_voice), is_frequency_hopping(is_frequency_hopping) {
        // Determine the block sizes
        unknown_data_block_size = (baud_rate >= 2400) ? 32 : 20;
        known_data_block_size = (baud_rate >= 2400) ? 16 : 20;

        if (baud_rate == 2400) {
            interleaver_block_size = (interleave_setting == 2) ? (40 * 576) : (40 * 72);
        } else if (baud_rate == 1200) {
            interleaver_block_size = (interleave_setting == 2) ? (40 * 288) : (40 * 36);
        } else if ((baud_rate >= 150) || (baud_rate == 75 && is_frequency_hopping)) {
            interleaver_block_size = (interleave_setting == 2) ? (40 * 144) : (40 * 18);
        } else {
            interleaver_block_size = (interleave_setting == 2) ? (20 * 36) : (10 * 9);
        }

        total_frames = interleaver_block_size / (unknown_data_block_size + known_data_block_size);
    }

    std::vector<uint8_t> formSymbols(std::vector<uint8_t>& symbol_data) {
        // Generate and scramble the sync preamble
        std::vector<uint8_t> sync_preamble = generateSyncPreamble();
        sync_preamble = scrambler.scrambleSyncPreamble(sync_preamble);
        
        size_t set_count = 0;
        size_t symbol_count = 0;
        size_t current_frame = 0;
        std::vector<uint8_t> data_stream;

        size_t current_index = 0;
        while (current_index < symbol_data.size()) {
            // Determine the size of the current unknown data block
            size_t block_size = std::min(unknown_data_block_size, symbol_data.size() - current_index);
            std::vector<uint8_t> unknown_data_block(symbol_data.begin() + current_index, symbol_data.begin() + current_index + block_size);
            current_index += block_size;

            // Map the unknown data based on baud rate
            if (baud_rate == 75) {
                size_t set_size = (interleave_setting == 2) ? 360 : 32;
                for (size_t i = 0; i < unknown_data_block.size(); i += set_size) {
                    bool is_exceptional_set = (set_count % ((interleave_setting == 1) ? 45 : 360)) == 0;
                    std::vector<uint8_t> mapped_set = map75bpsSet(unknown_data_block, i, set_size, is_exceptional_set);
                    data_stream.insert(data_stream.end(), mapped_set.begin(), mapped_set.end());
                    set_count++;
                }
            } else {
                // For baud rates greater than 75 bps
                std::vector<uint8_t> mapped_unknown_data = mapUnknownData(unknown_data_block);
                symbol_count += mapped_unknown_data.size();
                data_stream.insert(data_stream.end(), mapped_unknown_data.begin(), mapped_unknown_data.end());
            }

            // Insert probe data if we are at an interleaver block boundary
            if (baud_rate > 75) {
                std::vector<uint8_t> probe_data = generateProbeData(current_frame, total_frames);
                data_stream.insert(data_stream.end(), probe_data.begin(), probe_data.end());
                current_frame = (current_frame + 1) % total_frames;
            }
        }

        // Scramble the entire data stream
        data_stream = scrambler.scrambleData(data_stream);

        // Combine sync preamble and scrambled data stream
        std::vector<uint8_t> symbol_stream;
        symbol_stream.insert(symbol_stream.end(), sync_preamble.begin(), sync_preamble.end());
        symbol_stream.insert(symbol_stream.end(), data_stream.begin(), data_stream.end());

        return symbol_stream;
    }
    
    private:
    int baud_rate;
    int interleave_setting;
    bool is_voice;
    bool is_frequency_hopping;
    size_t interleaver_block_size;
    size_t unknown_data_block_size;
    size_t known_data_block_size;
    size_t total_frames;
    Scrambler scrambler = Scrambler();

    std::vector<uint8_t> mapChannelSymbolToTribitPattern(uint8_t symbol, bool repeat_twice = false) {
        std::vector<uint8_t> tribit_pattern;

        switch (symbol) {
            case 0b000: // 000
                tribit_pattern = {0, 0, 0, 0, 0, 0, 0, 0};
                break;
            case 0b001: // 001
                tribit_pattern = {0, 4, 0, 4, 0, 4, 0, 4};
                break;
            case 0b010: // 010
                tribit_pattern = {0, 0, 4, 4, 0, 0, 4, 4};
                break;
            case 0b011: // 011
                tribit_pattern = {0, 4, 4, 0, 0, 4, 4, 0};
                break;
            case 0b100: // 100
                tribit_pattern = {0, 0, 0, 0, 4, 4, 4, 4};
                break;
            case 0b101: // 101
                tribit_pattern = {0, 4, 0, 4, 4, 0, 4, 0};
                break;
            case 0b110: // 110
                tribit_pattern = {0, 0, 4, 4, 4, 4, 0, 0};
                break;
            case 0b111: // 111
                tribit_pattern = {0, 4, 4, 0, 4, 0, 0, 4};
                break;
            default:
                throw std::invalid_argument("Invalid channel symbol");
        }

        if (repeat_twice) {
            // Repeat the pattern twice instead of four times for known symbols
            tribit_pattern.insert(tribit_pattern.end(), tribit_pattern.begin(), tribit_pattern.end());
        } else {
            // Repeat the pattern four times as per Table XIII
            tribit_pattern.insert(tribit_pattern.end(), tribit_pattern.begin(), tribit_pattern.end());
            tribit_pattern.insert(tribit_pattern.end(), tribit_pattern.begin(), tribit_pattern.end());
            tribit_pattern.insert(tribit_pattern.end(), tribit_pattern.begin(), tribit_pattern.end());
        }

        return tribit_pattern;
    }

    std::vector<uint8_t> generateSyncPreamble() {
        std::vector<uint8_t> preamble;

        size_t num_segments = (interleave_setting == 2) ? 24 : 3;

        std::vector<uint8_t> segment_sequence = {0, 1, 3, 0, 1, 3, 1, 2, 0};

        uint8_t D1, D2;
        if (baud_rate == 4800) {
            D1 = 7; D2 = 6;
        } else if (baud_rate == 2400 && is_voice) {
            D1 = 7; D2 = 7;
        } else if (baud_rate == 2400) {
            D1 = (interleave_setting <= 1) ? 6 : 4;
            D2 = 4;
        } else if (baud_rate == 1200) {
            D1 = (interleave_setting <= 1) ? 6 : 4;
            D2 = 5;
        } else if (baud_rate == 600) {
            D1 = (interleave_setting <= 1) ? 6 : 4;
            D2 = 6;
        } else if (baud_rate == 300) {
            D1 = (interleave_setting <= 1) ? 6 : 4;
            D2 = 7;
        } else if (baud_rate == 150) {
            D1 = (interleave_setting <= 1) ? 7 : 5;
            D2 = 4;
        } else if (baud_rate == 75) {
            D1 = (interleave_setting <= 1) ? 7 : 5;
            D2 = 5;
        } else {
            throw std::invalid_argument("Invalid baud rate for Generate Sync Preamble");
        }

        segment_sequence.push_back(D1);
        segment_sequence.push_back(D2);
    
        uint8_t C1, C2, C3;
        if (interleave_setting == 2) {
            C1 = 5; C2 = 5; C3 = 7;
        } else {
            C1 = 1; C2 = 1; C3 = 2;
        }

        for (size_t i = 0; i < num_segments; i++) {
            std::vector<uint8_t> full_segment_sequence = segment_sequence;

            full_segment_sequence.push_back(C1);
            full_segment_sequence.push_back(C2);
            full_segment_sequence.push_back(C3);
            full_segment_sequence.push_back(0);

            for (uint8_t symbol : full_segment_sequence) {
                std::vector<uint8_t> mapped_tribit = mapChannelSymbolToTribitPattern(symbol);
                preamble.insert(preamble.end(), mapped_tribit.begin(), mapped_tribit.end());
            }

            if (C3 > 0) {
                C3--;
            } else if (C2 > 0) {
                C2--;
                C3 = 3;
            } else if (C1 > 0) {
                C1--;
                C2 = 3;
                C3 = 3;
            }
        }

        return preamble;
    }

    std::vector<uint8_t> generateProbeData(size_t current_frame, size_t total_frames) {
        std::vector<uint8_t> probe_data;

        // Set the known symbol patterns for D1 and D2 based on Table XI
        uint8_t D1, D2;
        if (baud_rate == 4800) {
            D1 = 7; D2 = 6;
        } else if (baud_rate == 2400 && is_voice) {
            D1 = 7; D2 = 7;
        } else if (baud_rate == 2400) {
            D1 = (interleave_setting <= 1) ? 6 : 4;
            D2 = 4;
        } else if (baud_rate == 1200) {
            D1 = (interleave_setting <= 1) ? 6 : 4;
            D2 = 5;
        } else if (baud_rate == 600) {
            D1 = (interleave_setting <= 1) ? 6 : 4;
            D2 = 6;
        } else if (baud_rate == 300) {
            D1 = (interleave_setting <= 1) ? 6 : 4;
            D2 = 7;
        } else if (baud_rate == 150) {
            D1 = (interleave_setting <= 1) ? 7 : 5;
            D2 = 4;
        } else if (baud_rate == 75) {
            D1 = (interleave_setting <= 1) ? 7 : 5;
            D2 = 5;
        } else {
            throw std::invalid_argument("Invalid baud rate for generateProbeData");
        }

        // If the current frame is not the last two frames, set probe data to zeros
        if (current_frame < total_frames - 2) {
            probe_data.resize(known_data_block_size, 0x00);
        }
        // If the current frame is the second-to-last frame, set probe data to D1 pattern
        else if (current_frame == total_frames - 2) {
            std::vector<uint8_t> d1_pattern = mapChannelSymbolToTribitPattern(D1, true);
            probe_data.insert(probe_data.end(), d1_pattern.begin(), d1_pattern.end());

            // Fill the remaining symbols with zeros if necessary
            if (probe_data.size() < known_data_block_size) {
                probe_data.resize(known_data_block_size, 0x00);
            }
        }
        // If the current frame is the last frame, set probe data to D2 pattern
        else if (current_frame == total_frames - 1) {
            std::vector<uint8_t> d2_pattern = mapChannelSymbolToTribitPattern(D2, true);
            probe_data.insert(probe_data.end(), d2_pattern.begin(), d2_pattern.end());

            // Fill the remaining symbols with zeros if necessary
            if (probe_data.size() < known_data_block_size) {
                probe_data.resize(known_data_block_size, 0x00);
            }
        }

        return probe_data;
    }

    void append75bpsMapping(std::vector<uint8_t>& symbol_stream, uint8_t symbol, bool is_exceptional_set) {
        if (is_exceptional_set) {
            switch (symbol) {
                case 0:
                    // Exceptional set mapping for symbol 00: (0000 4444) repeated 4 times
                    for (int i = 0; i < 4; ++i) {
                        symbol_stream.insert(symbol_stream.end(), baud75_exceptional_0.begin(), baud75_exceptional_0.end());
                    }
                    break;
                case 1:
                    // Exceptional set mapping for symbol 01: (0404 4040) repeated 4 times
                    for (int i = 0; i < 4; ++i) {
                        symbol_stream.insert(symbol_stream.end(), baud75_exceptional_1.begin(), baud75_exceptional_1.end());
                    }
                    break;
                case 2:
                    // Exceptional set mapping for symbol 10: (0044 4400) repeated 4 times
                    for (int i = 0; i < 4; ++i) {
                        symbol_stream.insert(symbol_stream.end(), baud75_exceptional_2.begin(), baud75_exceptional_2.end());
                    }
                    break;
                case 3:
                    // Exceptional set mapping for symbol 11: (0440 4004) repeated 4 times
                    for (int i = 0; i < 4; ++i) {
                        symbol_stream.insert(symbol_stream.end(), baud75_exceptional_3.begin(), baud75_exceptional_3.end());
                    }
                    break;
                default:
                    throw std::invalid_argument("Invalid channel symbol for exceptional set mapping");
            }
        } else {
            switch (symbol) {
                case 0:
                    // Normal set mapping for symbol 00: (0000) repeated 8 times
                    for (int i = 0; i < 8; ++i) {
                        symbol_stream.insert(symbol_stream.end(), baud75_normal_0.begin(), baud75_normal_0.end());
                    }
                    break;
                case 1:
                    // Normal set mapping for symbol 01: (0404) repeated 8 times
                    for (int i = 0; i < 8; ++i) {
                        symbol_stream.insert(symbol_stream.end(), baud75_normal_1.begin(), baud75_normal_1.end());
                    }
                    break;
                case 2:
                    // Normal set mapping for symbol 10: (0044) repeated 8 times
                    for (int i = 0; i < 8; ++i) {
                        symbol_stream.insert(symbol_stream.end(), baud75_normal_2.begin(), baud75_normal_2.end());
                    }
                    break;
                case 3:
                    // Normal set mapping for symbol 11: (0440) repeated 8 times
                    for (int i = 0; i < 8; ++i) {
                        symbol_stream.insert(symbol_stream.end(), baud75_normal_3.begin(), baud75_normal_3.end());
                    }
                    break;
                default:
                    throw std::invalid_argument("Invalid channel symbol for normal set mapping");
            }
        }
    }

    std::vector<uint8_t> map75bpsSet(const std::vector<uint8_t>& data, size_t start_index, size_t set_size, bool is_exceptional_set) {
        std::vector<uint8_t> mapped_set;

        // Make sure we do not exceed the size of the data vector
        size_t end_index = std::min(start_index + set_size, data.size());

        for (size_t i = start_index; i < end_index; ++i) {
            append75bpsMapping(mapped_set, data[i], is_exceptional_set);
        }

        return mapped_set;
    }

    std::vector<uint8_t> mapUnknownData(const std::vector<uint8_t>& data) {
        std::vector<uint8_t> mapped_data;

        for (auto symbol : data) {
            if (baud_rate >= 2400) {
                // Pass tribit symbols as-is
                mapped_data.push_back(symbol);
            } else if (baud_rate == 1200) {
                // Map dibit symbols to tribit symbols 0, 2, 4, 6
                switch (symbol) {
                    case 0b00:
                        mapped_data.push_back(0);
                        break;
                    case 0b01:
                        mapped_data.push_back(2);
                        break;
                    case 0b10:
                        mapped_data.push_back(4);
                        break;
                    case 0b11:
                        mapped_data.push_back(6);
                        break;
                    default:
                        throw std::invalid_argument("Invalid dibit symbol for 1200 bps");
                }
            } else if (baud_rate >= 150 && baud_rate <= 600) {
                // Map binary symbols to tribit symbols 0 and 4
                if (symbol == 0) {
                    mapped_data.push_back(0);
                } else if (symbol == 1) {
                    mapped_data.push_back(4);
                } else {
                    throw std::invalid_argument("Invalid binary symbol for baud rates 150 to 600 bps");
                }
            } else {
                throw std::invalid_argument("Invalid baud rate for mapUnknownData");
            }
        }

        return mapped_data;
    }
};

#endif