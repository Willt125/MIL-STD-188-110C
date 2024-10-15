#ifndef INTERLEAVER_H
#define INTERLEAVER_H

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "bitstream.h"

/**
 * @class Interleaver
 * @brief A class to interleave input bitstreams based on the specified baud rate and interleave settings.
 * 
 * The Interleaver processes input data in chunks, rearranging the bit order as required 
 * by the MIL-STD-188-110 standard for different baud rates and interleave settings.
 */
class Interleaver {
public:
    /**
     * @brief Constructs an Interleaver with specific baud rate, interleave settings, and operation mode.
     * @param baud_rate The baud rate of the transmission.
     * @param interleave_setting The interleave setting (0, short, or long).
     * @param is_frequency_hopping Boolean flag indicating if frequency-hopping mode is enabled.
     */
    Interleaver(size_t baud_rate, size_t interleave_setting, bool is_frequency_hopping)
        : baud_rate(baud_rate), interleave_setting(interleave_setting), is_frequency_hopping(is_frequency_hopping) {
        setMatrixDimensions();
        matrix.resize(rows * columns, 0);
    }

    /**
     * @brief Interleaves the entire input BitStream and returns the interleaved result.
     * @param input_data The input BitStream to be interleaved.
     * @return A new BitStream containing the interleaved data.
     */
    std::vector<uint8_t> interleaveStream(const BitStream& input_data) {
        BitStream data = input_data;
        BitStream interleaved_data;
        size_t chunk_size = rows * columns;
        size_t input_index = 0;

        while (input_index < data.getMaxBitIndex()) {
            size_t end_index = std::min(input_index + chunk_size, data.getMaxBitIndex());
            BitStream chunk = data.getSubStream(input_index, end_index);
            if (chunk.getMaxBitIndex() > rows * columns) {
                throw std::invalid_argument("Input data exceeds interleaver matrix size in loadChunk()");
            }
            BitStream interleaved_chunk = interleaveChunk(chunk);
            interleaved_data += interleaved_chunk;
            input_index = end_index;
        }

        // Apply puncturing for 2400 bps in frequency-hopping mode (Rate 2/3)
        if (baud_rate == 2400 && is_frequency_hopping) {
            return applyPuncturing(interleaved_data);
        }

        std::vector<uint8_t> final_interleaved_data = groupSymbols(interleaved_data);
        return final_interleaved_data;
    }

    /**
     * @brief Retrieves the number of bits required to fully flush the interleaver matrix.
     * @return The number of bits needed for a complete flush.
     */
    size_t getFlushBits() const {
        return rows * columns;
    }

private:
    size_t baud_rate;
    size_t interleave_setting;
    bool is_frequency_hopping;
    size_t rows;
    size_t columns;
    std::vector<uint8_t> matrix; ///< 1D vector representing the interleaver matrix.

    static constexpr size_t ROW_INCREMENT_DEFAULT = 9;
    static constexpr size_t COLUMN_DECREMENT_DEFAULT = 17;

    /**
     * @brief Groups the bits into symbols based on the baud rate (e.g., 2 bits per symbol at 1200 bps).
     * @param input_data The input BitStream to be grouped into symbols.
     * @return A vector of grouped symbols.
     */
    std::vector<uint8_t> groupSymbols(BitStream& input_data) {
        std::vector<uint8_t> grouped_data;
        size_t max_index = input_data.getMaxBitIndex();
        size_t bits_per_symbol = (baud_rate == 2400) ? 3 : (baud_rate == 1200 || (baud_rate == 75 && !is_frequency_hopping)) ? 2 : 1;
        size_t current_index = 0;

        while ((current_index + bits_per_symbol) < max_index) {
            uint8_t symbol = 0;

            for (int i = 0; i < bits_per_symbol; i++) {
                symbol = (symbol << 1) | input_data.getBitVal(current_index + i);
            }

            grouped_data.push_back(symbol);
            current_index += bits_per_symbol;
        }

        return grouped_data;
    }

    /**
     * @brief Interleaves a chunk of the input BitStream and returns the result.
     * @param input_data The input BitStream chunk.
     * @return A BitStream representing the interleaved chunk.
     */
    BitStream interleaveChunk(const BitStream& input_data) {
        loadChunk(input_data);
        return fetchChunk();
    }

    /**
     * @brief Loads bits from the input BitStream into the interleaver matrix.
     * @param data The input BitStream to load.
     */
    void loadChunk(const BitStream& data) {
        size_t row = 0;
        size_t col = 0;
        size_t index = 0;

        size_t row_increment = (baud_rate == 75 && interleave_setting == 2) ? 7 : 9;

        // Load bits into the matrix
        std::fill(matrix.begin(), matrix.end(), 0);  // Clear previous data
        while (index < data.getMaxBitIndex() && col < columns) {
            size_t matrix_idx = row * columns + col;
            if (matrix_idx >= matrix.size()) {
                throw std::out_of_range("Matrix index out of bounds in loadChunk()");
            }

            matrix[matrix_idx] = data.getBitVal(index++);
            row = (row + row_increment) % rows;

            if (row == 0) {
                col++;
            }
        }
    }

    /**
     * @brief Fetches bits from the interleaver matrix in the interleaved order.
     * @return A BitStream containing the fetched interleaved data.
     */
    BitStream fetchChunk() {
        BitStream fetched_data;
        size_t row = 0;
        size_t col = 0;

        size_t column_decrement = (baud_rate == 75 && interleave_setting == 2) ? 7 : 17;

        // Fetch bits from the matrix
        while (fetched_data.getMaxBitIndex() < rows * columns) {
            size_t matrix_idx = row * columns + col;
            if (matrix_idx >= matrix.size()) {
                throw std::out_of_range("Matrix index out of bounds in fetchChunk()");
            }

            fetched_data.putBit(matrix[matrix_idx]);
            row++;

            if (row == rows) {
                row = 0;
                col = (col + 1) % columns;
            } else {
                col = (col + columns - column_decrement) % columns;
            }
        }

        return fetched_data;
    }



    /**
     * @brief Sets the matrix dimensions based on baud rate and interleave setting.
     */
    void setMatrixDimensions() {
        if (baud_rate == 4800) {
            rows = 0;
            columns = 0;
        } else if (baud_rate == 2400) {
            rows = 40;
            columns = (interleave_setting == 2) ? 576 : 72;
        } else if (baud_rate == 1200) {
            rows = 40;
            columns = (interleave_setting == 2) ? 288 : 36;
        } else if (baud_rate == 600) {
            rows = 40;
            columns = (interleave_setting == 2) ? 144 : 18;
        } else if (baud_rate == 300 || baud_rate == 150) {
            rows = 40;
            columns = (interleave_setting == 2) ? 144 : 18;
        } else if (baud_rate == 75) {
            if (is_frequency_hopping) {
                rows = 40;
                columns = 18;
            } else {
                rows = (interleave_setting == 2) ? 20 : 10;
                columns = (interleave_setting == 2) ? 36 : 9;
            }
        } else {
            throw std::invalid_argument("Invalid baud rate for setMatrixDimensions");
        }
    }

    /**
     * @brief Applies puncturing to the interleaved data (used for 2400 bps in frequency-hopping mode).
     * @param interleaved_data The interleaved data to be punctured.
     * @return A BitStream containing punctured data.
     */
    BitStream applyPuncturing(const BitStream& interleaved_data) {
        BitStream punctured_data;
        for (size_t i = 0; i < interleaved_data.getMaxBitIndex(); i++) {
            if ((i % 4) != 1) { // Skip every fourth bit (the second value of T2)
                punctured_data.putBit(interleaved_data.getBitVal(i));
            }
        }
        return punctured_data;
    }
};

#endif
