import numpy as np
PreambleRandomizerSeq = [7, 4, 3, 0, 5, 1, 5, 0, 2, 2, 1, 1,
                         5, 7, 4, 3, 5, 0, 2, 6, 2, 1, 6, 2, 0, 0, 5, 0, 5, 2, 6, 6]

bin_to_int = {
    "00": 4,
    "01": 6,
    "10": 5,
    "11": 7,
}

bps_to_3bit = {
    75: [7, 5],
    150: [7, 4],
    300: [6, 7],
    600: [6, 6],
    1200: [6, 5],
    2400: [6, 4],
    4800: [7, 6],
}


def encode_count(count):
    count_bin = bin(count)[2:].zfill(6)
    return [bin_to_int[count_bin[:2]],
            bin_to_int[count_bin[2:4]],
            bin_to_int[count_bin[4:]]]


def PreambleGenerate(Bd: int = 75, interleave_len: str = "S", data_type: str = "Data") -> np.ndarray:
    """Generate a preamble sequence for the encoding stream according to the MIL-STD-188-110 spec.
       This sequence is used to fill the interleaver matrix, so the `interleave_len` parameter decides
       how long the sequence is.
       In combination with the `bps` (Bits Per Second, or Baud) and `dtyp` parameter, they decide what
       pattern to load the preamble with, so that the receiving modem knows how to decode the stream.

       The data array first gets loaded with [0, 1, 3, 0, 1, 3, 1, 2, 0], regardless of settings.
       Then two 3-bit numbers are appended according to bps and data type, and the frame count is encoded
       and appended (as well as a trailing 0).
       Once all the preamble frames are generated, the entire preamble is exapnded and converted to symbols 0 or 4 out of 8PSK,
       and scrambled using `PreambleRandomizerSeq` and returned.
       This is an internal function. Do not call this directly."""

    if interleave_len == "Z" or interleave_len == "S":

        counts = range(2, -1, -1)

        preamble_frames = [[0, 1, 3, 0, 1, 3, 1, 2, 0] + (bps_to_3bit[Bd] if data_type == "Data" else [
                                                          7, 7]) + encode_count(count) + [0] for count in counts]

        data_array = np.array(preamble_frames, dtype=int).flatten()

    else:
        counts = range(23, -1, -1)

        preamble_frames = [[0, 1, 3, 0, 1, 3, 1, 2, 0] + (bps_to_3bit[Bd] if data_type == "Data" else [
                                                          7, 7]) + encode_count(count) + [0] for count in counts]

        data_array = np.array(preamble_frames, dtype=int).flatten()

    data_out = np.array([], dtype=int)
    for i in range(len(data_array)):
        match data_array[i]:
            case 0:
                data_out = np.append(data_out, [0, 0, 0, 0, 0, 0, 0, 0] * 4)
            case 1:
                data_out = np.append(data_out, [0, 4, 0, 4, 0, 4, 0, 4] * 4)
            case 2:
                data_out = np.append(data_out, [0, 0, 4, 4, 0, 0, 4, 4] * 4)
            case 3:
                data_out = np.append(data_out, [0, 4, 4, 0, 0, 4, 4, 0] * 4)
            case 4:
                data_out = np.append(data_out, [0, 0, 0, 0, 4, 4, 4, 4] * 4)
            case 5:
                data_out = np.append(data_out, [0, 4, 0, 4, 4, 0, 4, 0] * 4)
            case 6:
                data_out = np.append(data_out, [0, 0, 4, 4, 4, 4, 0, 0] * 4)
            case 7:
                data_out = np.append(data_out, [0, 4, 4, 0, 4, 0, 0, 4] * 4)

    data_out = np.array([(x + PreambleRandomizerSeq[i %
                        len(PreambleRandomizerSeq)]) % 8 for i, x in enumerate(data_out)])

    return data_out
