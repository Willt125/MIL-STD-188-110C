import numpy as np

D1D2_conv = {
    (150, "Z", _): [0, 4, 0, 4, 4, 0, 4, 0, 0, 0, 0, 0, 4, 4, 4, 4],
    (150, "S", _): [0, 4, 0, 4, 4, 0, 4, 0, 0, 0, 0, 0, 4, 4, 4, 4],
    (150, "L", _): [0, 4, 4, 0, 4, 0, 0, 4, 0, 0, 0, 0, 4, 4, 4, 4],
    (300, "Z", _): [0, 0, 0, 0, 4, 4, 4, 4, 0, 4, 4, 0, 4, 0, 0, 4],
    (300, "S", _): [0, 0, 0, 0, 4, 4, 4, 4, 0, 4, 4, 0, 4, 0, 0, 4],
    (300, "L", _): [0, 0, 4, 4, 4, 4, 0, 0, 0, 4, 4, 0, 4, 0, 0, 4],
    (600, "Z", _): [0, 0, 0, 0, 4, 4, 4, 4, 0, 0, 4, 4, 4, 4, 0, 0],
    (600, "S", _): [0, 0, 0, 0, 4, 4, 4, 4, 0, 0, 4, 4, 4, 4, 0, 0],
    (600, "L", _): [0, 0, 4, 4, 4, 4, 0, 0, 0, 0, 4, 4, 4, 4, 0, 0],
    (1200, "Z", _): [0, 0, 0, 0, 4, 4, 4, 4, 0, 4, 0, 4, 4, 0, 4, 0],
    (1200, "S", _): [0, 0, 0, 0, 4, 4, 4, 4, 0, 4, 0, 4, 4, 0, 4, 0],
    (1200, "L", _): [0, 0, 4, 4, 4, 4, 0, 0, 0, 4, 0, 4, 4, 0, 4, 0],
    (2400, "Z", "Data"): [0, 0, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4],
    (2400, "S", "Data"): [0, 0, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4],
    (2400, "L", "Data"): [0, 0, 0, 0, 4, 4, 4, 4, 0, 0, 0, 0, 4, 4, 4, 4],
    (2400, "Z", "Voice"): [0, 4, 4, 0, 4, 0, 0, 4, 0, 4, 4, 0, 4, 0, 0, 4],
    (2400, "S", "Voice"): [0, 4, 4, 0, 4, 0, 0, 4, 0, 4, 4, 0, 4, 0, 0, 4],
    (2400, "L", "Voice"): [None],
    (4800, "Z", _): [0, 4, 4, 0, 4, 0, 0, 4, 0, 0, 4, 4, 4, 4, 0, 0],
    (4800, "S", _): [0, 4, 4, 0, 4, 0, 0, 4, 0, 0, 4, 4, 4, 4, 0, 0],
    (4800, "L", _): [None]
}

75bps_fixed_constellation_map_normal = {
    0: [0, 0, 0, 0] * 8,
    1: [0, 4, 0, 4] * 8,
    2: [0, 0, 4, 4] * 8,
    3: [0, 4, 4, 0] * 8
}

75bps_fixed_constellation_map_exceptional = {
    0: [0, 0, 0, 0, 4, 4, 4, 4] * 4,
    1: [0, 4, 0, 4, 4, 0, 4, 0] * 4,
    2: [0, 0, 4, 4, 4, 4, 0, 0] * 4,
    3: [0, 4, 4, 0, 4, 0, 0, 4] * 4
}

150_600bps_constellation_map = {
    0: 0,
    1: 4
}

1200bps_constellation_map = {
    0: 0,
    1: 2,
    2: 4,
    3: 6
}

def channelMapChunk(indata: np.array[int], Bd: int, interleave_len: str, freq_mode: str) -> np.array[int]:
  
    if freq_mode == 'fixed' and Bd == 75:
        len_mod = 360 if interleave_len == "L" else 45
        map_out = np.array([], dtype=int)
        for i, symbol in enumerate(indata):
            if (i % len_mod == 0) and (i != 0):
                map_out.concatenate((map_out, 75bps_fixed_constellation_map_exceptional[symbol]))
            else
                map_out.concatenate((map_out, 75bps_fixed_constellation_map_normal[symbol]))
        return map_out
    


    if Bd in (75, 150, 300, 600)
        map_out = np.array([], dtype=int)
        for symbol in indata:
            map_out.concatenate((map_out, 150_600bps_constellation_map[symbol]))
        return map_out

    if Bd == 1200:
        map_out = np.array([], dtype=int)
        for symbol in indata:
            map_out.concatenate((map_out, 1200bps_constellation_map[symbol]))
        return map_out
    
    if Bd in (2400, 4800):
        return indata

def ChannelMap(indata: np.array[int], Bd: int, interleave_len: str, freq_mode: str, data_type: str) -> np.array[int]:
    """Map the input symbols to their final forms, adding known data probes (commented below)
    and repeat symbols where necessary. We're almost done, woohoo!
    This is where a lot of the heavy lifting is done, and using numpy arrays gets REALLY slow.
    Unfortunately I think the only way to get faster is to move to C/C++, which I'm already doing."""

    if Bd == 75 and freq_mode == "fixed":  # 75bps fixed-frequency mode does not get data probes, it gets its own encoding. Just pass it on
        return ChannelMapChunk(indata, Bd, interleave_len, freq_mode)
    
    if Bd == 2400 and data_type == "voice" and interleave_len == "L":
        raise RuntimeError("2400 bps voice with long interleave is not supported")
    
    probe_len = 16 if Bd in (2400, 4800) else 20
    len_mod = 32 if Bd in (2400, 4800) else 20
    
    if Bd in (150, 300, 600, 1200):
        out_arr = np.array([], dtype=int)
        for i in range(0, len(indata), len_mod):
            out_arr = np.concatenate((out_arr, ChannelMapChunk(indata[i:i+len_mod], Bd, interleave_len, freq_mode)))
            probe = [0] * probe_len
            probe[0:] = D1D2_conv[Bd, interleave_len, freq_mode]
            out_arr = np.concatenate((out_arr, probe))
        return out_arr
    
    if Bd == 2400:
        out_arr = np.array([], dtype=int)
        for i in range(0, len(indata), len_mod):
            out_arr = np.concatenate((out_arr, ChannelMapChunk(indata[i:i+len_mod], Bd, interleave_len, freq_mode)))
            probe = [0] * probe_len
            probe[0:] = D1D2_conv[Bd, interleave_len, freq_mode]
            out_arr = np.concatenate((out_arr, probe))
        return out_arr
    
    if Bd == 4800:
        out_arr = np.array([], dtype=int)
        for i in range(0, len(indata), len_mod):
            out_arr = np.concatenate((out_arr, ChannelMapChunk(indata[i:i+len_mod], Bd, interleave_len, freq_mode)))
            probe = [0] * probe_len
            probe[0:] = D1D2_conv[Bd, "S" if interleave_len == "L" else interleave_len, freq_mode]
            out_arr = np.concatenate((out_arr, probe))
        return out_arr