import numpy as np

def InterleaveChunk(inchunk, numrows, numcols, rowInc,colDec):
	# Interleave the input chunk. You shouldn't be calling this function manually.
	# MIL-STD-188-110A specifies a two-matrix approach, with different loading
	# and fetching patterns to interleave the bitstream, adding burst error protection.
	# The inputs are handled by the wrapper.
	
	if len(inchunk) != (numrows * numcols):
		raise RuntimeError("The input chunk doesn't fit in the interleave matrix.")
	i = 0
	rownum = 0
	colnum = 0
	temp = np.zeros(inchunk.shape)
	out = np.zeros(inchunk.shape)

	while i < numrows * numcols:
		count = 0
		while count < numrows:
			temp[colnum + numcols * rownum] = inchunk[i]
			rownum = (rownum + rowInc) % numrows
			count += 1
			i += 1
		rownum = 0
		colnum += 1

	i = 0
	rownum = 0
	colnum = 0
	del count

	while i < numrows * numcols:
		lastcolnum = colnum
		while rownum < numrows:
			out[i] = temp[colnum + numcols * rownum]
			rownum += 1
			colnum = (colnum - colDec) % numcols
			i += 1
		rownum = 0
		colnum = lastcolnum + 1
		lastcolnum = colnum
	return out

def InterleaveData(indata, Bd=75, interleave_len="S", frqmode="fixed"):
        # Chunkwise interleave the input data.
	# indata is the input bitstream. No bounds correction is applied;
	# the input stream should already be a multiple of the chunk size.
	# Bd selects the baud, and
	# interleave_len is pretty self-explanatory.
	# The conbination of these and frqmode (not supported) selects the
	# interleave length.
	
        if interleave_len == "Z":
                return indata
        elif Bd == 75 and interleave_len == "S" and frqmode == "fixed":
                numrows = 10
                numcols = 9
                rowInc = 7
                colDec = 7
        elif Bd == 75 and interleave_len == "L" and frqmode == "fixed":
                numrows = 20
                numcols = 36
                rowInc = 7
                colDec = 7
        elif Bd == 75 and interleave_len == "S" and frqmode == "hopping":
                numrows = 40
                numcols = 18
                rowInc = 9
                colDec = 17
        elif Bd == 75 and interleave_len == "L" and frqmode == "hopping":
                numrows = 40
                numcols = 144
                rowInc = 9
                colDec = 17
        elif (Bd == 150 or Bd == 300 or Bd == 600) and interleave_len == "S":
                numrows = 40
                numcols = 18
                rowInc = 9
                colDec = 17
        elif (Bd == 150 or Bd == 300 or Bd == 600) and interleave_len == "L":
                numrows = 40
                numcols = 144
                rowInc = 9
                colDec = 17
        elif Bd == 1200 and interleave_len == "S":
                numrows = 40
                numcols = 36
                rowInc = 9
                colDec = 17
        elif Bd == 1200 and interleave_len == "L":
                numrows = 40
                numcols = 288
                rowInc = 9
                colDec = 17
        elif Bd == 2400 and interleave_len == "S":
                numrows = 40
                numcols = 72
                rowInc = 9
                colDec = 17
        elif Bd == 2400 and interleave_len == "L":
                numrows = 40
                numcols = 576
                rowInc = 9
                colDec = 17
        elif Bd == 4800:
                return indata
        else:
                raise RuntimeError("Invalid input.")
	
        z = numrows * numcols

        if Bd == 2400 and frqmode == 'hopping':
                tmpout = np.zeros(indata.shape)
                out = np.array([])
        else:
                out = np.zeros(indata.shape)
	
        if (len(indata) % z) != 0:
                raise RuntimeError("The input data does not evenly fit in the interleaver. Something is VERY wrong.")
	
        for i in range(0,len(indata),z):
                temp = indata[i:i+z]
                temp = InterleaveChunk(temp,numrows,numcols,rowInc,colDec)
                if Bd == 2400 and frqmode == 'hopping':
                        tmpout[i:i+z] = temp
                else:
                        out[i:i+z] = temp

        if Bd == 2400 and frqmode == 'hopping':
                for i in range(len(tmpout)):
                        if i % 4 != 3:
                                out.append(tmpout[i])
        out = np.array(out, dtype=int)
        return out