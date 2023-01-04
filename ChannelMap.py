import numpy as np

def channelMap(indata, Bd, inter_len, frqmode, data_type):
  # Map the input symbols to their final forms, adding known data probes (commented below)
  # and repeat symbols where necessary. We're almost done, woohoo!
  # This is where a lot of the heavy lifting is done, and using numpy arrays gets REALLY slow.
  # Unfortunately I think the only way to get faster is to move to C/C++, which I'm already doing.
  
    if frqmode == 'fixed':
        if Bd == 75:
            if inter_len == "Z" or inter_len == "S":
                lenMod = 45
            elif inter_len == "L":
                lenMod = 360
            map_out = []
            for i in range(len(indata)):
                if ((i % lenMod == 0) and (i != 0)):
                    if indata[i] == 0:
                        map_out += [0,0,0,0] * 8
                    elif indata[i] == 1:
                        map_out += [0,4,0,4] * 8
                    elif indata[i] == 2:
                        map_out += [0,0,4,4] * 8
                    elif indata[i] == 3:
                        map_out += [0,4,4,0] * 8
                    else:
                        raise RuntimeError("Invalid symbol in stream.")
                else:
                    if indata[i] == 0:
                        map_out += [0,0,0,0,4,4,4,4] * 4
                    elif indata[i] == 1:
                        map_out += [0,4,0,4,4,0,4,0] * 4
                    elif indata[i] == 2:
                        map_out += [0,0,4,4,4,4,0,0] * 4
                    elif indata[i] == 3:
                        map_out += [0,4,4,0,4,0,0,4] * 4
                    else:
                        raise RuntimeError("Invalid symbol in stream.")
            map_out = np.array(map_out)
            return map_out
        elif Bd == 150 or Bd == 300 or Bd == 600:
            unknownMod = 20
            map_out = []
            i = 0
            affecting = False
            while (i < len(indata)):
                if affecting == False:
                    for j in range(i,i + unknownMod):
                        if indata[j] == 0:
                            map_out += [0]
                        elif indata[j] == 1:
                            map_out += [4]
                        else:
                            raise RuntimeError("Invalid symbol in stream.")
                    i += unknownMod
                    affecting = True
                else:
                    if Bd == 150:
                        if inter_len == "Z" or inter_len == "S":
                            D1 = [0,4,4,0,4,0,0,4] * 2
                            D2 = [0,0,0,0,4,4,4,4] * 2
                            preblock = [0] * 1420
                        elif inter_len == "L":
                            D1 = [0,4,0,4,4,0,4,0] * 2
                            D2 = [0,0,0,0,4,4,4,4] * 2
                            preblock = [0] * 11500
                    elif Bd == 300:
                        if inter_len == "Z" or inter_len == "S":
                            D1 = [0,0,4,4,4,4,0,0] * 2
                            D2 = [0,4,4,0,4,0,0,4] * 2
                            preblock = [0] * 1420
                        elif inter_len == "L":
                            D1 = [0,0,0,0,4,4,4,4] * 2
                            D2 = [0,4,4,0,4,0,0,4] * 2
                            preblock = [0] * 11500
                    elif Bd == 600:
                        if inter_len == "Z" or inter_len == "S":
                            D1 = [0,0,4,4,4,4,0,0] * 2
                            D2 = [0,0,4,4,4,4,0,0] * 2
                            preblock = [0] * 1420
                        elif inter_len == "L":
                            D1 = [0,0,0,0,4,4,4,4] * 2
                            D2 = [0,0,4,4,4,4,0,0] * 2
                            preblock = [0] * 11500
                    map_out = map_out + D1 + D2 + [0, 0, 0, 0]
                    affecting = False
            map_out = np.array(map_out)
            return map_out
        elif Bd == 1200:
            unknownMod = 20
            knownMod = 20
            map_out = []
            i = 0
            affecting = False
            while (i < len(indata)):
                if affecting == False:
                    for j in range(i,i + unknownMod):
                        if indata[j] == 0:
                            map_out += [0]
                        elif indata[j] == 1:
                            map_out += [2]
                        elif indata[j] == 2:
                            map_out += [4]
                        elif indata[j] == 3:
                            map_out += [6]
                        else:
                            raise RuntimeError("Invalid symbol in stream.")
                    i += unknownMod
                    affecting = True
                else:
                    if inter_len == "Z" or inter_len == "S":
                        D1 = [0,0,4,4,4,4,0,0] * 2
                        D2 = [0,4,0,4,4,0,4,0] * 2
                        preblock = [0] * 1420
                    elif inter_len == "L":
                        D1 = [0,0,0,0,4,4,4,4] * 2
                        D2 = [0,4,0,4,4,0,4,0] * 2
                        preblock =  [0] * 11500
                    map_out = map_out + D1 + D2 + [0, 0, 0, 0]
                    affecting = False
            map_out = np.array(map_out)
            return map_out
        elif Bd == 2400 or Bd == 4800:
            unknownMod = 32
            map_out = []
            i = 0
            affecting = False
            while (i < len(indata)):
                if affecting == False:
                    for j in range(i,i + unknownMod):
                        try:
                            map_out += [indata[j]]
                        except:
                            pass
                    i += unknownMod
                    affecting = True
                else:
                    if Bd == 2400 and data_type == "Data":
                        if inter_len == "Z" or inter_len == "S":
                            D1 = [0,0,4,4,4,4,0,0] * 2
                            D2 = [0,0,0,0,4,4,4,4] * 2
                            preblock = [0] * 1424
                        elif inter_len == "L":
                            D1 = [0,0,0,0,4,4,4,4] * 2
                            D2 = [0,0,0,0,4,4,4,4] * 2
                            preblock = [0] * 11504
                    elif Bd == 2400 and data_type == "Voice":
                        if inter_len == "Z" or inter_len == "S":
                            D1 = [0,4,4,0,4,0,0,4] * 2
                            D2 = [0,4,4,0,4,0,0,4] * 2
                            preblock = [0] * 1424
                        elif inter_len == "L":
                            raise RuntimeError("Invalid input.")
                    elif Bd == 4800:
                        if inter_len == "Z" or inter_len == "S":
                            D1 = [0,4,4,0,4,0,0,4] * 2
                            D2 = [0,0,4,4,4,4,0,0] * 2
                            preblock = [0] * 1424
                        elif inter_len == "L":
                            raise RuntimeError("Invalid input.")
                    map_out = map_out + D1 + D2
                    affecting = False
            map_out = np.array(map_out)
            return map_out