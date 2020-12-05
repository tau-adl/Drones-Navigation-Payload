import constants as c

def TwoBytesToUint16 (a_byte_arr, a_shift = 0):
    return int.from_bytes( a_byte_arr[a_shift:a_shift+2],byteorder='little',signed=False)


def FourBytesToUint32 (a_byte_arr, a_shift):
    return int.from_bytes( a_byte_arr[a_shift:a_shift+4],byteorder='little',signed=False)

def TwoBytesToInt16 (a_byte_arr, a_shift = 0):
    return int.from_bytes( a_byte_arr[a_shift:a_shift+2],byteorder='little',signed=True)

def FourBytesToInt32 (a_byte_arr, a_shift):
    return int.from_bytes( a_byte_arr[a_shift:a_shift+4],byteorder='little',signed=True)

def ErrorHandler(a_string = ""):
    print("Fatal Error:")
    print(a_string)
    y = input()
    exit(1)

def WarningHandler(a_string = ""):
    print("Warning:")
    print(a_string)

def prnt(a_string ="", a_verbosity = 10000):
    if(a_verbosity >= c.GLOBAL_VERBOSITY):
        print(a_string)