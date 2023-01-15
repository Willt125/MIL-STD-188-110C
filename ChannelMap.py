import numpy as np

D1D2_conv = {
    (150, "Z", "Data"): [0, 4, 0, 4, 4, 0, 4, 0, 0, 0, 0, 0, 4, 4, 4, 4],
    (150, "S", "Data"): [0, 4, 0, 4, 4, 0, 4, 0, 0, 0, 0, 0, 4, 4, 4, 4],
    (150, "L", "Data"): [0, 4, 4, 0, 4, 0, 0, 4, 0, 0, 0, 0, 4, 4, 4, 4],
    (150, "Z", "Voice"): [0, 4, 0, 4, 4, 0, 4, 0, 0, 0, 0, 0, 4, 4, 4, 4],
    (150, "S", "Voice"): [0, 4, 0, 4, 4, 0, 4, 0, 0, 0, 0, 0, 4, 4, 4, 4],
    (150, "L", "Voice"): [0, 4, 4, 0, 4, 0, 0, 4, 0, 0, 0, 0, 4, 4, 4, 4],
    (300, "Z", "Data"): [0, 0, 0, 0, 4, 4, 4, 4, 0, 4, 4, 0, 4, 0, 0, 4],
    (300, "S", "Data"): [0, 0, 0, 0, 4, 4, 4, 4, 0, 4, 4, 0, 4, 0, 0, 4],
    (300, "L", "Data"): [0, 0, 4, 4, 4, 4, 0, 0, 0, 4, 4, 0, 4, 0, 0, 4],
    (300, "Z", "Voice"): [0, 0, 0, 0, 4, 4, 4, 4, 0, 4, 4, 0, 4, 0, 0, 4],
    (300, "S", "Voice"): [0, 0, 0, 0, 4, 4, 4, 4, 0, 4, 4, 0, 4, 0, 0, 4],
    (300, "L", "Voice"): [0, 0, 4, 4, 4, 4, 0, 0, 0, 4, 4, 0, 4, 0, 0, 4],
    (600, "Z", "Data"): [0, 0, 0, 0, 4, 4, 4, 4, 0, 0, 4, 4, 4, 4, 0, 0],
    (600, "S", "Data"): [0, 0, 0, 0, 4, 4, 4, 4, 0, 0, 4, 4, 4, 4, 0, 0],
    (600, "L", "Data"): [0, 0, 4, 4, 4, 4, 0, 0, 0, 0, 4, 4, 4, 4, 0, 0],
    (600, "Z", "Voice"): [0, 0, 0, 0, 4, 4, 4, 4, 0, 0, 4, 4, 4, 4, 0, 0],
    (600, "S", "Voice"): [0, 0, 0, 0, 4, 4, 4, 4, 0, 0, 4, 4, 4, 4, 0, 0],
    (600, "L", "Voice"): [0, 0, 4, 4, 4, 4, 0, 0, 0, 0, 4, 4, 4, 4, 0, 0],
    (1200, "Z", "Data"): [0, 0, 0, 0, 4, 4, 4, 4, 0, 4, 0, 4, 4, 0, 4, 0],
    (1200, "S", "Data"): [0, 0, 0, 0, 4, 4, 4, 4, 0, 4, 0, 4, 4, 0, 4, 0],
    (1200, "L", "Data"): [0, 0, 4, 4, 4, 4, 0, 0, 0, 4, 0, 4, 4, 0, 4, 0],
    (1200, "Z", "Voice"): [0, 0, 0, 0, 4, 4, 4, 4, 0, 4, 0, 4, 4, 0, 4, 0],
    (1200, "S", "Voice"): [0, 0, 0, 0, 4, 4, 4, 4, 0, 4, 0, 4, 4, 0, 4, 0],
    (1200, "L", "Voice"): [0, 0, 4, 4, 4, 4, 0, 0, 0, 4, 0, 4, 4, 0, 4, 0],
    (2400, "Z", "Data"): [0, 0, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4],
    (2400, "S", "Data"): [0, 0, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4],
    (2400, "L", "Data"): [0, 0, 0, 0, 4, 4, 4, 4, 0, 0, 0, 0, 4, 4, 4, 4],
    (2400, "Z", "Voice"): [0, 4, 4, 0, 4, 0, 0, 4, 0, 4, 4, 0, 4, 0, 0, 4],
    (2400, "S", "Voice"): [0, 4, 4, 0, 4, 0, 0, 4, 0, 4, 4, 0, 4, 0, 0, 4],
    (2400, "L", "Voice"): [None],
    (4800, "Z", "Data"): [0, 4, 4, 0, 4, 0, 0, 4, 0, 0, 4, 4, 4, 4, 0, 0],
    (4800, "S", "Data"): [0, 4, 4, 0, 4, 0, 0, 4, 0, 0, 4, 4, 4, 4, 0, 0],
    (4800, "L", "Data"): [None],
    (4800, "Z", "Voice"): [0, 4, 4, 0, 4, 0, 0, 4, 0, 0, 4, 4, 4, 4, 0, 0],
    (4800, "S", "Voice"): [0, 4, 4, 0, 4, 0, 0, 4, 0, 0, 4, 4, 4, 4, 0, 0],
    (4800, "L", "Voice"): [None],
}

bps75_fixed_constellation_map_normal = {
    0: [0, 0, 0, 0] * 8,
    1: [0, 4, 0, 4] * 8,
    2: [0, 0, 4, 4] * 8,
    3: [0, 4, 4, 0] * 8,
}

bps75_fixed_constellation_map_exceptional = {
    0: [0, 0, 0, 0, 4, 4, 4, 4] * 4,
    1: [0, 4, 0, 4, 4, 0, 4, 0] * 4,
    2: [0, 0, 4, 4, 4, 4, 0, 0] * 4,
    3: [0, 4, 4, 0, 4, 0, 0, 4] * 4
}

bps150_600_constellation_map = {
    0: 0,
    1: 4
}


bps1200_constellation_map = {
    0: 0,
    1: 2,
    2: 4,
    3: 6
}


def channelMapChunk(indata: np.ndarray, Bd: int = 75, interleave_length: str = "S", freq_mode: str = "Fixed") -> np.ndarray:
    """Map the input symbols to their final forms. We're almost done, woohoo!
       `indata` in the input array from the MGD
       `Bd` is the baud rate
       `interleave_length` is the interleaver block size, either Z, S, or L
       `freq_mode` is the current frequency mode, either fixed or hopping"""

    if freq_mode == 'Fixed' and Bd == 75:
        len_mod = 45 if interleave_length == "S" or interleave_length == "Z" else 360
        map_out = []
        for i, symbol in enumerate(indata):
            if (i % len_mod == 0) and (i != 0):
                map_out += bps75_fixed_constellation_map_exceptional[symbol]
            else:
                map_out += bps75_fixed_constellation_map_normal[symbol]
        return np.array(map_out, dtype=int)

    if Bd == 75:
        map_out = []
        for symbol in indata:
            map_out += bps150_600_constellation_map[symbol]
        return np.array(map_out, dtype=int)

    if (Bd == 150 | Bd == 300 | Bd == 600):
        map_out = []
        for symbol in indata:
            map_out += bps150_600_constellation_map[symbol]
        return np.array(map_out, dtype=int)

    if Bd == 1200:
        map_out = []
        for symbol in indata:
            map_out += bps1200_constellation_map[symbol]
        return np.array(map_out, dtype=int)

    return indata


def channelMap(indata: np.ndarray, Bd: int = 75, interleave_length: str = "S", freq_mode: str = "Fixed") -> np.ndarray:
    if Bd == 75 and freq_mode == "Fixed":
        return channelMapChunk(indata, Bd, interleave_length, freq_mode)

    probe_len = 16 if Bd in (2400, 4800) else 20
    len_mod = 32 if Bd in (2400, 4800) else 20

    if Bd in (150, 300, 600, 1200):
        map_out = np.array([], dtype=int)
        for i in range(0, len(indata), len_mod):
            map_out = np.concatenate((map_out, channelMapChunk(
                indata[i:i+len_mod], Bd, interleave_length, freq_mode)))
            probe = [0] * probe_len
            probe[0:] = D1D2_conv[Bd, interleave_length, freq_mode]
            map_out = np.concatenate((map_out, probe))
        return map_out

    if Bd == 2400:
        map_out = np.array([], dtype=int)
        for i in range(0, len(indata), len_mod):
            map_out = np.concatenate((map_out, channelMapChunk(
                indata[i:i+len_mod], Bd, interleave_length, freq_mode)))
            probe = [0] * probe_len
            probe[0:] = D1D2_conv[Bd, interleave_length, freq_mode]
            map_out = np.concatenate((map_out, probe))
        return map_out

    if Bd == 4800:
        map_out = np.array([], dtype=int)
        for i in range(0, len(indata), len_mod):
            map_out = np.concatenate((map_out, channelMapChunk(
                indata[i:i+len_mod], Bd, interleave_length, freq_mode)))
            probe = [0] * probe_len
            probe[0:] = D1D2_conv[Bd, "S" if interleave_length ==
                                  "L" else interleave_length, freq_mode]
            map_out = np.concatenate((map_out, probe))
        return map_out
