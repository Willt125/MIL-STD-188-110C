import numpy as np

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

        fec_buffer = [0, 0, 0, 0, 0, 0, 1] # Initialize the buffer
        x = 0 # initialize FEC output pointer

        if fqmode == 'fixed':
                match Bd:
                        case 600 or 1200 or 2400 or 75:
                                fec_out = [None]*(len(inArray)*2)
                                pass
                        case 300:
                                fec_out = [None]*(len(inArray)*4)
                                pass
                        case 150:
                                fec_out = [None]*(len(inArray)*8)
                                pass
                        case 4800:
                                return inArray # 4800-baud does not receive error correction

                for bit in inArray:
                        fec_buffer[0] = bit
                        bit1 = fec_buffer[0] ^ fec_buffer[2] ^ fec_buffer[3] ^ fec_buffer[5] ^ fec_buffer[6] # The error correction is just a simple convolution.
                        bit2 = fec_buffer[0] ^ fec_buffer[1] ^ fec_buffer[2] ^ fec_buffer[3] ^ fec_buffer[6] # There is probably a more efficient way to do this.
                                        
                        match Bd:
                                case 150:
                                        fec_out[x] = bit1
                                        fec_out[x+1] = bit2
                                        fec_out[x+2] = bit1
                                        fec_out[x+3] = bit2
                                        fec_out[x+4] = bit1
                                        fec_out[x+5] = bit2
                                        fec_out[x+6] = bit1
                                        fec_out[x+7] = bit2
                                        x += 8
                                        pass
                                case 300:
                                        fec_out[x] = bit1
                                        fec_out[x+1] = bit2
                                        fec_out[x+2] = bit1
                                        fec_out[x+3] = bit2
                                        x += 4
                                        pass
                                case 600 or 1200 or 2400 or 75:
                                        fec_out[x] = bit1
                                        fec_out[x+1] = bit2
                                        x += 2
                        fec_buffer[5] = fec_buffer[4]
                        fec_buffer[4] = fec_buffer[3]
                        fec_buffer[3] = fec_buffer[2]
                        fec_buffer[2] = fec_buffer[1]
                        fec_buffer[1] = fec_buffer[0]
                fec_out = np.array(fec_out, dtype=int)
                return fec_out

        elif fqmode == 'hopping':
                match Bd:
                        case 75:
                                fec_out = [None]*(len(inArray)*16)
                                pass
                        case 150:
                                fec_out = [None]*(len(inArray)*8)
                                pass
                        case 300:
                                fec_out = [None]*(len(inArray)*4)
                                pass
                        case 600 or 1200:
                                fec_out = [None]*(len(inArray)*2)
                                pass
                        case 2400:
                                fec_out = [None]*((len(inArray)/2)*3)
                                pass
                        case 4800:
                                return inArray

                for bit in inArray:
                        fec_buffer[0] = bit
                        bit1 = fec_buffer[0] ^ fec_buffer[2] ^ fec_buffer[3] ^ fec_buffer[5] ^ fec_buffer[6]
                        bit2 = fec_buffer[0] ^ fec_buffer[1] ^ fec_buffer[2] ^ fec_buffer[3] ^ fec_buffer[6]
                        match Bd:
                                case 75:
                                        fec_out[x] = bit1
                                        fec_out[x+1] = bit2
                                        fec_out[x+2] = bit1
                                        fec_out[x+3] = bit2
                                        fec_out[x+4] = bit1
                                        fec_out[x+5] = bit2
                                        fec_out[x+6] = bit1
                                        fec_out[x+7] = bit2
                                        fec_out[x+8] = bit1
                                        fec_out[x+9] = bit2
                                        fec_out[x+10] = bit1
                                        fec_out[x+11] = bit2
                                        fec_out[x+12] = bit1
                                        fec_out[x+13] = bit2
                                        fec_out[x+14] = bit1
                                        fec_out[x+15] = bit2
                                        x += 16
                                        pass
                                case 150:
                                        fec_out[x] = bit1
                                        fec_out[x+1] = bit2
                                        fec_out[x+2] = bit1
                                        fec_out[x+3] = bit2
                                        fec_out[x+4] = bit1
                                        fec_out[x+5] = bit2
                                        fec_out[x+6] = bit1
                                        fec_out[x+7] = bit2
                                        x += 8
                                        pass
                                case 300:
                                        fec_out[x] = bit1
                                        fec_out[x+1] = bit2
                                        fec_out[x+2] = bit1
                                        fec_out[x+3] = bit2
                                        x += 4
                                        pass
                                case 600 or 1200:
                                        fec_out[x] = bit1
                                        fec_out[x+1] = bit2
                                        x += 2
                                        pass
                                case 2400:
                                        fec_out[x] = bit1
                                        fec_out[x+1] = bit2
                                        fec_out[x+2] = bit1
                                        x += 3
                        fec_buffer[5] = fec_buffer[4]
                        fec_buffer[4] = fec_buffer[3]
                        fec_buffer[3] = fec_buffer[2]
                        fec_buffer[2] = fec_buffer[1]
                        fec_buffer[1] = fec_buffer[0]
                fec_out = np.array(fec_out, dtype=int)
                return fec_out