# convert the binary file to numpy format to as the input of gs
import numpy as np
import os

def b2b(path, isLong):
    """
    binary to binary
    """
    vlistPath = os.path.join(path, 'csr_vlist.bin')
    # elistPath = os.path.join(path, 'csr_elist.bin')

    rowPtr = np.fromfile(vlistPath, dtype=np.int32) if isLong == False else np.fromfile(vlistPath, dtype=np.int64)
    vNum = len(rowPtr) - 1
    arr = np.arange(vNum, dtype=np.int32)
    repeatNum = np.diff(rowPtr)

    starts = np.repeat(arr, repeatNum)
    print(starts)
    outPath = os.path.join(path, 'starts.bin')
    starts.tofile(outPath)


def read(path):
    startPath = os.path.join(path, 'starts.bin')
    endPath = os.path.join(path, 'csr_elist.bin')
    vlistPath = os.path.join(path, 'csr_vlist.bin')
    
    size = os.path.getsize(vlistPath)
    print('size: ', size)
    eleNum = size // 4
    vNum = eleNum - 1

    print("vNum: ", vNum)
    nodes = np.arange(vNum, dtype=np.int32)
    starts = np.fromfile(startPath, dtype=np.int32)
    ends = np.fromfile(endPath, dtype=np.int32)

    ls_v = [nodes]
    ls_e = [starts, ends]

    # ls_v.append(nodes)
    # ls_e.append(starts, ends)

    print(ls_v)
    print(ls_e)
    
    

if __name__ == "__main__":
    # b2b('/data/coding/DailyCode/graphscope', False)
    # read('/data/coding/DailyCode/graphscope')