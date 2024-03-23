import hashlib
from binascii import unhexlify, hexlify

def bitcoinHash(headerHex):
    """
    https://en.bitcoin.it/wiki/Block_hashing_algorithm
    Python implementation of the bitcoin hashing algorithm to use a reference for testing.
    
    @param headerhex
        This is a hex string representing a block of bytes with the lowest addressed byte at the start of string.
    """
    headerBin = unhexlify(headerHex)
    hash = hashlib.sha256(hashlib.sha256(headerBin).digest()).digest()
    print(hexlify(hash).decode("utf-8"))
    print(hexlify(hash[::-1]).decode("utf-8"))

headerHex = ('01000000' +
    '81cd02ab7e569e8bcd9317e2fe99f2de44d49ab2b8851ba4a308000000000000' +
    'e320b6c2fffc8d750423db8b1eb942ae710e951ed797f7affc8892b0f1fc122b' +
    'c7f5d74d' +
    'f2b9441a' +
    '42a14695')
bitcoinHash(headerHex)

# Expected results:
# >>>'1dbd981fe6985776b644b173a4d0385ddc1aa2a829688d1e0000000000000000'
# >>>'00000000000000001e8d6829a8a21adc5d38d0a473b144b6765798e61f98bd1d'

headerHex = ('01000000' +
    '0000000000000000000000000000000000000000000000000000000000000000' +
    '3BA3EDFD7A7B12B27AC72C3E67768F617FC81BC3888A51323A9FB8AA4B1E5E4A' +
    '29AB5F49' +
    'FFFF001D' +
    '1DAC2B7C')
bitcoinHash(headerHex)