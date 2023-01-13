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

tribit_mgd_map = {
    0: 0,
    1: 1,
    2: 3,
    3: 2,
    4: 7,
    5: 6,
    6: 4,
    7: 5
}

dibit_mgd_map = {
    0: 0,
    1: 1,
    2: 3,
    3: 2
}

def MGD_Decode_Chunk(indata: np.array[int], Bd: int, frqmode: str) -> np.array[int]:
    # Convert the input bitgroups to their Gray form, so that
    # receive errors only result in one bit in err.
    
    if ((Bd == 75 and frqmode == 'fixed') or Bd == 1200):
        grouped = np.zeros([indata.shape[0]//2,],dtype=int)
        for i in range(0,len(indata),2):
            grouped[i//2] = int(str(indata[i]) + str(indata[i+1]), base=2)
        out = np.zeros(grouped.shape,dtype=int)
        for el in grouped:
            out[i] = dibit_mgd_map[el]
        return out
    
    elif (Bd == 150 or Bd == 300 or Bd == 600 or (Bd == 75 and frqmode == 'hopping')):
        return indata
    
    elif (Bd == 2400 or Bd == 4800):
        grouped = np.zeros([indata.shape[0]//3,],dtype=int)
        for i in range(0,len(indata),3):
            grouped[i//3] = int(str(indata[i]) + str(indata[i+1]) + str(indata[i+2]), base=2)
        out = np.zeros(grouped.shape,dtype=int)
        for el in grouped:
            out[i] = tribit_mgd_map[el]
        return out
    else:
        raise RuntimeError("Invalid input.")

def MGD_Decode(indata: np.array[int], Bd: int, interleave_len: str, frqmode: str, data_type: str) -> np.array[int]:
    # Convert the input bitgroups to their Gray form, so that
    # receive errors only result in one bit in err.
    # We also insert the known data probes at this time, if applicable.
    # The standard is not very clear on how this is supposed to work...

    if Bd == 75 and frqmode == "fixed":
        return MGD_Decode_Chunk(indata, Bd, frqmode) # 75bps fixed-frequency mode does not get data probes, just process the entire stream
    
    if Bd == 2400 and data_type == "voice" and interleave_len == "L":
        raise RuntimeError("2400 bps voice with long interleave is not supported")
    
    probe_len = 16 if Bd in (2400, 4800) else 20
    len_mod = 32 if Bd in (2400, 4800) else 20
    
    if Bd in (150, 300, 600, 1200):
        out_arr = np.array([], dtype=int)
        for i in range(0, len(indata), len_mod):
            out_arr = np.concatenate((out_arr, MGD_Decode_Chunk(indata[i:i+len_mod])))
            probe = [0] * probe_len
            probe[0:] = D1D2_conv[Bd, interleave_len, frqmode]
            out_arr = np.concatenate((out_arr, probe))
        return out_arr
    
    if Bd == 2400:
        out_arr = np.array([], dtype=int)
        for i in range(0, len(indata), len_mod):
            out_arr = np.concatenate((out_arr, MGD_Decode_Chunk(indata[i:i+len_mod])))
            probe = [0] * probe_len
            probe[0:] = D1D2_conv[Bd, interleave_len, frqmode]
            out_arr = np.concatenate((out_arr, probe))
        return out_arr
    
    if Bd == 4800:
        out_arr = np.array([], dtype=int)
        for i in range(0, len(indata), len_mod):
            out_arr = np.concatenate((out_arr, MGD_Decode_Chunk(indata[i:i+len_mod])))
            probe = [0] * probe_len
            probe[0:] = D1D2_conv[Bd, "S" if interleave_len == "L" else interleave_len, frqmode]
            out_arr = np.concatenate((out_arr, probe))
        return out_arr