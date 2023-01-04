import numpy as np

EOM = [0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,
       1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,
       0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,
       1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,
       0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,
       1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1]

def InputDataToBinaryStream(input_string, interleave_length: int = 90, EOMCycles: int = 1) -> np.ndarray[int]:
        """Converts an ASCII string to its bitwise representation and
        prepares it for the signal chain.
        `interleave_length` describes the length in bits of a single interleaver chunk,
        which the output array should be a multiple of.
        `EOMCycles` is how many times the signal should send the EOM message before ending.
        This module should not be called by the user."""
        
        # Here we split the input string into individual characters
        asciichars = bytearray([bin(ord(char))[2:].zfill(8) for i, char in enumerate(input_string)])
        
        # And then we split each character into individual bits
        input_bits = []
        for char in asciichars:
                for bit in char:
                        input_bits.append(int(bit))
        
        # And add `EOMCycles` number of the EOM list
        for i in range(EOMCycles):
                input_bits.extend(EOM)
        
        # And finish by adding enough 0 bits to clear the FEC and interleave matrix
        input_bits.extend(([0]*144))
        while ((len(input_bits)%interleave_length)!=0):
                input_bits.append(0)
        
        input_bits = np.array(input_bits, dtype=int)
        return input_bits