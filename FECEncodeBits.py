import numpy as np

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

def FECEncodeBits(inArray: np.array[int], Bd: int, fqmode: str ='fixed') -> np.array[int]:
	"""Returns a (much longer) FEC-encoded bit array from an input bit array
	   `inArray` is the input bitstream
	   `Bd` is the selected baud
	   `fqmode` is whether we're operating serial or frequency hopping.
	   Frequency hopping is not supported at this time.
	   MIL-STD-188-110 specifies a scheme that returns two bits from every one
	   input bit. The even bits are encoded using x^6+x^4+x^3+x+1, and
	   the odd bits are encoded using x^6+x^5+x^4+x^3+1.
	   These bits are repeat coded depending on the selected mode."""

        fec_buffer = [0, 0, 0, 0, 0, 0, 1] # Initialize the FEC buffer
        x = 0 # initialize FEC output pointer

        if fqmode == 'fixed':
                if Bd != 4800:
                        fec_out = [None]*(len(inArray)*fixed_repeat_factor[Bd])
                else:
                        return inArray # 4800-baud does not receive error correction

                for bit in inArray:
                        fec_buffer[0] = bit
                        bit1 = fec_buffer[0] ^ fec_buffer[2] ^ fec_buffer[3] ^ fec_buffer[5] ^ fec_buffer[6] # The error correction is just a simple convolution.
                        bit2 = fec_buffer[0] ^ fec_buffer[1] ^ fec_buffer[2] ^ fec_buffer[3] ^ fec_buffer[6] # There is probably a more efficient way to do this.
                                        
                        fec_out[x:x+fixed_repeat_factor[Bd]] = [bit1, bit2]*fixed_repeat_factor[Bd]
                        x += fixed_repeat_factor[Bd]
                        
                        fec_buffer[5] = fec_buffer[4]
                        fec_buffer[4] = fec_buffer[3]
                        fec_buffer[3] = fec_buffer[2]
                        fec_buffer[2] = fec_buffer[1]
                        fec_buffer[1] = fec_buffer[0]
                fec_out = np.array(fec_out, dtype=int).flatten()
                return fec_out

        elif fqmode == 'hopping':
                if Bd != 4800:
                        fec_out = [None]*(len(inArray)*hopping_repeat_factor[Bd])
                else:
                        return inArray

                for bit in inArray:
                        fec_buffer[0] = bit
                        bit1 = fec_buffer[0] ^ fec_buffer[2] ^ fec_buffer[3] ^ fec_buffer[5] ^ fec_buffer[6] # The error correction is just a simple convolution.
                        bit2 = fec_buffer[0] ^ fec_buffer[1] ^ fec_buffer[2] ^ fec_buffer[3] ^ fec_buffer[6] # There is probably a more efficient way to do this.

                        if Bd != 2400:
                                fec_out[x:x+hopping_repeat_factor[Bd]] = [bit1, bit2]*hopping_repeat_factor[Bd]
                                x += hopping_repeat_factor[Bd]
                        else:
                                fec_out[x:x+3] = [bit1, bit2, bit1]
                                x += 3
                        
                        fec_buffer[5] = fec_buffer[4]
                        fec_buffer[4] = fec_buffer[3]
                        fec_buffer[3] = fec_buffer[2]
                        fec_buffer[2] = fec_buffer[1]
                        fec_buffer[1] = fec_buffer[0]
                fec_out = np.array(fec_out, dtype=int).flatten()
                return fec_out