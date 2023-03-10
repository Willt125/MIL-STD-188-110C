import numpy as np

# The repeat factors define how many times to repeat the output of the FEC buffer into the output bitstream.
# Defined in MIL-STD-188-110, page 26 & 28 (32 & 24 in the PDF)
fixed_repeat_factor = {
    75: 2,
    150: 8,
    300: 4,
    600: 2,
    1200: 2,
    2400: 2
}

hopping_repeat_factor = {
    75: 16,
    150: 8,
    300: 4,
    600: 2,
    1200: 2,
    2400: 3/2
}


def FECEncodeBits(inArray: np.ndarray, Bd: int = 75, fqmode: str = "Fixed") -> np.ndarray:
    """Returns a (much longer) FEC-encoded bit array from an input bit array
       `inArray` is the input bitstream
       `Bd` is the selected baud
       `fqmode` is whether we're operating serial or frequency hopping.
       Frequency hopping is not supported at this time.
       MIL-STD-188-110 specifies a scheme that returns two bits from every one
       input bit. The even bits are encoded using x^6+x^4+x^3+x+1, and
       the odd bits are encoded using x^6+x^5+x^4+x^3+1.
       These bits are repeat coded depending on the selected mode."""

    # Initialize the FEC buffer. The seventh position is a static '1'
    fec_buffer = [0, 0, 0, 0, 0, 0, 1]
    x = 0  # initialize FEC output pointer

    if fqmode == 'Fixed':
        if Bd != 4800:
            # Prepare the output bitstream.
            fec_out = [None]*(len(inArray)*fixed_repeat_factor[Bd])
        else:
            return inArray  # 4800-baud does not receive error correction

        for bit in inArray:
            # load the lowest bit position with the input bit
            fec_buffer[0] = bit
            # The error correction is just a simple convolution.
            bit1 = fec_buffer[0] ^ fec_buffer[2] ^ fec_buffer[3] ^ fec_buffer[5] ^ fec_buffer[6]
            # There is probably a more efficient way to do this.
            bit2 = fec_buffer[0] ^ fec_buffer[1] ^ fec_buffer[2] ^ fec_buffer[3] ^ fec_buffer[6]

            # load the output array with the current batch.
            fec_out[x:] = [bit1, bit2] * \
                (fixed_repeat_factor[Bd] // 2)
            x += fixed_repeat_factor[Bd]  # and shift the output pointer

            # now shift the rest of the fec_buffer to the right, leaving the static '1' in place
            fec_buffer[0:6] = [0] + fec_buffer[0:5]

        fec_out = np.array(fec_out, dtype=int).flatten()
        return fec_out

    elif fqmode == 'Hopping':
        if Bd != 4800:
            # prepare the output bitstream
            fec_out = [None]*(len(inArray)*hopping_repeat_factor[Bd])
        else:
            return inArray  # 4800-baud does not get FEC correction

        for bit in inArray:
            fec_buffer[0] = bit
            # The error correction is just a simple convolution.
            bit1 = fec_buffer[0] ^ fec_buffer[2] ^ fec_buffer[3] ^ fec_buffer[5] ^ fec_buffer[6]
            # There is probably a more efficient way to do this.
            bit2 = fec_buffer[0] ^ fec_buffer[1] ^ fec_buffer[2] ^ fec_buffer[3] ^ fec_buffer[6]

            if Bd != 2400:
                fec_out[x:] = [bit1, bit2] * \
                    (hopping_repeat_factor[Bd] // 2)
                x += hopping_repeat_factor[Bd]
            else:
                # 2400-baud hopping mode needs a special case due to bit puncturing
                fec_out[x:x+3] = [bit1, bit2, bit1]
                x += 3

            # now shift the rest of the fec_buffer to the right, leaving the static '1' in place
            fec_buffer[0:6] = [0] + fec_buffer[0:5]

        # ensure the output array is flat
        fec_out = np.array(fec_out, dtype=int).flatten()
        return fec_out
