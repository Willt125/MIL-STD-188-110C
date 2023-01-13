import numpy as np

scramble_out = [4, 1, 0, 2, 2, 1, 5, 4, 3, 6, 6, 2, 2, 4, 4, 2, 6, 0, 5, 1, 3, 6, 6, 2, 5, 1, 0, 3, 3, 1, 7, 1, 0, 2, 7, 5, 0, 1, 4, 3, 5, 1, 1, 5, 6, 7, 3, 7, 5, 1, 0, 4, 1, 0, 2, 5, 4, 1, 5, 6, 7, 4, 2, 1, 0, 5, 5, 3, 0, 1, 1, 5, 6, 0, 6, 3, 4, 4, 1, 6, 1, 3, 6, 4, 3, 4, 5, 2, 7, 2, 2, 7, 4, 3, 0, 7, 2, 4, 1, 1, 1, 4, 7, 0, 4, 6, 7, 0, 0, 1, 3, 6, 6, 5, 0, 5, 1, 6, 2, 3, 7, 2, 4, 3, 0, 0, 0, 5, 4, 5, 5, 7, 5, 3, 3, 3, 1, 2, 5, 5, 7, 0, 0, 3, 2, 7, 2, 7, 4, 4, 2, 6, 7, 0, 5, 2, 3, 7, 0, 5]

def ScrambleBits(inArray):
	scramble_temp = scramble_out
	while (len(scramble_temp) < len(inArray)):
		scramble_temp.extend(scramble_out)
	scrambld = []
	for i in range(len(inArray)):
		scrambld.append(inArray[i] ^ scramble_temp[i])
	scrambld = np.array(scrambld, dtype=int)
	return scrambld