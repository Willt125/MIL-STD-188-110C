import numpy as np

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

def MGD_Decode(indata: np.array[int], Bd: int, frqmode: str) -> np.array[int]:
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