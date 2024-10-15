#ifndef BITSTREAM_H
#define BITSTREAM_H

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <vector>

/**
 * @class BitStream
 * @brief A class to represent a stream of bits with bit-level read and write access.
 *
 * The BitStream class provides functionality to manipulate a byte stream at the bit level.
 * It derives from std::vector<uint8_t> to utilize the benefits of a byte vector while providing
 * additional methods for bit manipulation.
 */
class BitStream : public std::vector<uint8_t> {
public:
    /**
     * @brief Default constructor.
     */
    BitStream() : std::vector<uint8_t>(), bit_index(0), max_bit_idx(0) {}

    /**
     * @brief Constructs a BitStream from an existing vector of bytes.
     * @param data The byte stream to be used for initializing the BitStream.
     */
    BitStream(const std::vector<uint8_t>& data) : std::vector<uint8_t>(data), bit_index(0), max_bit_idx(data.size() * 8) {}

    /**
     * @brief Constructs a BitStream from an existing vector of bytes with a specified bit size.
     * @param data The byte stream to be used for initializing the BitStream.
     * @param size_in_bits The number of bits to consider in the stream.
     */
    BitStream(const std::vector<uint8_t>& data, size_t size_in_bits) : std::vector<uint8_t>(data), bit_index(0), max_bit_idx(size_in_bits) {}

    /**
     * @brief Copy constructor from another BitStream.
     * @param data The BitStream to copy from.
     */
    BitStream(const BitStream& data) : std::vector<uint8_t>(data), bit_index(0), max_bit_idx(data.max_bit_idx) {}

    /**
     * @brief Constructs a BitStream from a substream of another BitStream.
     * @param other The original BitStream.
     * @param start_bit The starting bit index of the substream.
     * @param end_bit The ending bit index of the substream (exclusive).
     * @throws std::out_of_range if start or end indices are out of bounds.
     */
    BitStream(const BitStream& other, size_t start_bit, size_t end_bit) : bit_index(0) {
        if (start_bit >= other.max_bit_idx || end_bit > other.max_bit_idx || start_bit > end_bit) {
            throw std::out_of_range("BitStream substream indices are out of range.");
        }
        max_bit_idx = end_bit - start_bit;
        for (size_t i = start_bit; i < end_bit; i++) {
            putBit(other.getBitVal(i));
        }
    }

    /**
     * @brief Reads the next bit from the stream.
     * @return The next bit (0 or 1).
     * @throws std::out_of_range if no more bits are available in the stream.
     */
    int getNextBit() {
        if (bit_index >= max_bit_idx) {
            throw std::out_of_range("No more bits available in the stream.");
        }

        int bit = getBitVal(bit_index++);
        return bit;
    }

    /**
     * @brief Gets the value of a bit at a specific index.
     * @param idx The index of the bit to be retrieved.
     * @return The value of the bit (0 or 1).
     * @throws std::out_of_range if the bit index is out of range.
     */
    int getBitVal(const size_t idx) const {
        if (idx >= max_bit_idx) {
            throw std::out_of_range("Bit index out of range in getBitVal.");
        }

        size_t byte_idx = idx / 8;
        size_t bit_idx = idx % 8;

        uint8_t tmp = this->at(byte_idx);
        uint8_t mask = 0x80 >> bit_idx;
        uint8_t result = tmp & mask;

        return result ? 1 : 0;
    }

    /**
     * @brief Checks if there are more bits available in the stream.
     * @return True if there are more bits available, otherwise false.
     */
    bool hasNext() const {
        return bit_index < max_bit_idx;
    }

    /**
     * @brief Sets a specific bit value in the stream.
     * @param idx The index of the bit to set.
     * @param val The value to set the bit to (0 or 1).
     *
     * This function ensures that the stream has enough bytes to accommodate
     * the given bit index. If the bit index is out of bounds, the stream is
     * resized accordingly.
     */
    void setBitVal(const size_t idx, uint8_t val) {
        size_t byte_idx = idx / 8;
        size_t bit_idx = idx % 8;
        uint8_t mask = 0x80 >> bit_idx;

        if (byte_idx >= this->size()) {
            this->resize(byte_idx + 1, 0);
        }

        if (val == 0) {
            this->at(byte_idx) &= ~mask;
        } else {
            this->at(byte_idx) |= mask;
        }
    }

    /**
     * @brief Appends a bit to the end of the stream.
     * @param bit The value of the bit to append (0 or 1).
     *
     * This function keeps track of the current bit index and appends bits
     * sequentially. If necessary, the stream is resized to accommodate the new bit.
     */
    void putBit(uint8_t bit) {
        size_t byte_idx = max_bit_idx / 8;
        if (byte_idx >= this->size()) {
            this->push_back(0);
        }
        size_t bit_idx = max_bit_idx % 8;
        setBitVal(max_bit_idx, bit);
        max_bit_idx += 1;
    }

    /**
     * @brief Resets the bit index to the beginning of the stream.
     */
    void resetBitIndex() {
        bit_index = 0;
    }

    /**
     * @brief Returns the maximum bit index value (total number of bits in the stream).
     * @return The total number of bits in the stream.
     */
    size_t getMaxBitIndex() const {
        return max_bit_idx;
    }

    BitStream& operator=(const BitStream& other) {
        this->clear();
        this->resize(other.size());
        std::copy(other.begin(), other.end(), this->begin());
        this->bit_index = other.bit_index;
        this->max_bit_idx = other.max_bit_idx;
        return *this;
    }

    /**
     * @brief Adds the contents of another BitStream to the current BitStream.
     * @param other The BitStream to be added.
     * @return Reference to the current BitStream after adding.
     */
    BitStream& operator+=(const BitStream& other) {
        size_t other_max_bit_idx = other.getMaxBitIndex();
        for (size_t i = 0; i < other_max_bit_idx; i++) {
            this->putBit(other.getBitVal(i));
        }

        return *this;
    }

    /**
     * @brief Gets a substream from the current BitStream.
     * @param start_bit The starting bit index of the substream.
     * @param end_bit The ending bit index of the substream (exclusive).
     * @return A new BitStream containing the specified substream.
     * @throws std::out_of_range if start or end indices are out of bounds.
     */
    BitStream getSubStream(size_t start_bit, size_t end_bit) const {
        if (start_bit >= max_bit_idx || end_bit > max_bit_idx || start_bit > end_bit) {
            throw std::out_of_range("BitStream substream indices are out of range.");
        }
        BitStream substream;
        for (size_t i = start_bit; i < end_bit; i++) {
            substream.putBit(getBitVal(i));
        }
        return substream;
    }

    /**
     * @brief Returns the current bit index in the stream.
     * @return The current bit index.
     */
    size_t getCurrentBitIndex() const {
        return bit_index;
    }

private:
    size_t bit_index; ///< The current bit index in the stream.
    size_t max_bit_idx; ///< The total number of bits in the stream.
};

BitStream operator+(const BitStream& lhs, const BitStream& rhs) {
    BitStream result = lhs;
    result += rhs;
    return result;
}

#endif
