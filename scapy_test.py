import binascii, re
'''
from scapy.all import *
INFILE = 'meindump_neu'
OUTFILE = 'stripped.pcap'
paks = rdpcap(INFILE)
myLLDPpacket = paks[0].payload

#for pak in paks:
#    pak[TCP].remove_payload()
#wrpcap(OUTFILE, paks)
'''

'''
filename = 'meindump_neu'
with open(filename, 'rb') as f:
    content = f.read()
hexLLDPpaket = binascii.hexlify(content)
'''

def getLLDPcontent(hexLLDPpaket):
  match = re.search(r"(fe|fd)..cc4d55d9", hexLLDPpaket)
  hexLLDPpayload = hexLLDPpaket[match.start(1):]
  print(hexLLDPpayload)
  
  TLVsizeAndType = binascii.unhexlify(hexLLDPpayload[:4])
  TLVsizeAndType = int.from_bytes(TLVsizeAndType, byteorder='big', signed=False)
  TLVsize = TLVsizeAndType & 0b111111111
  
  hexLLDPcontent = hexLLDPpayload[4+8:4+TLVsize*2]  # 4 Size + Type, 8 OUI + Subtype
  return hexLLDPcontent



#main()
#def main():
hexLLDPcontent = getLLDPcontent('d4c3b2a10200040000000000000000009f86020001000000b4a4d25841100f00ab010000ab0100000180c200000e00044b0170aa88cc02070400044b0170ac04070300044b0170aa060200780a096c6f63616c686f73740c674665646f72612032352028576f726b73746174696f6e2045646974696f6e29204c696e757820342e392e31342d3230302e666332352e7838365f363420233120534d50204d6f6e204d61722031332031393a32363a3430205554432032303137207838365f36340e04009c0014100c0501c0a8b28502000000030010181102fe800000000000000f94918e4bd31ccc0200000003000807656e7030733138fe0900120f030100000000fe0900120f01036c010010febccc4d55d9007b084d4e4d2d564c414e2e20495076342e20495076362e20656d61696c2e20444e532d74797065732e20526f757465722e20446173206973742065696e20426569737069656c20667565722065696e656e2062656e75747a6572646566696e69657274656e20537472696e672e204f7267616e697a6174696f6e616c206964656e7469666965723a204c4d55203132372e2020564c414e20494420616e6420564c414e204e616d653a203132332e204d4e4d2d564c414e0000')
print(hexLLDPcontent)

VLANid = int.from_bytes(binascii.unhexlify(hexLLDPcontent[:4]), byteorder='big', signed=False)
print("VLANid: " + str(VLANid))

VLANnameLength = int.from_bytes(binascii.unhexlify(hexLLDPcontent[4:6]), byteorder='big', signed=False)
print("VLANnameLength: " + str(VLANnameLength))

stringLLDPcontent = binascii.unhexlify(hexLLDPcontent[6:]).decode('ascii')
print(stringLLDPcontent)





# d4c3b2a10200040000000000000000009f86020001000000b4a4d25841100f00ab010000ab0100000180c200000e00044b0170aa88cc02070400044b0170ac04070300044b0170aa060200780a096c6f63616c686f73740c674665646f72612032352028576f726b73746174696f6e2045646974696f6e29204c696e757820342e392e31342d3230302e666332352e7838365f363420233120534d50204d6f6e204d61722031332031393a32363a3430205554432032303137207838365f36340e04009c0014100c0501c0a8b28502000000030010181102fe800000000000000f94918e4bd31ccc0200000003000807656e7030733138fe0900120f030100000000fe0900120f01036c010010fec2cc4d55d9febccc4d55d9007b084d4e4d2d564c414e2e20495076342e20495076362e20656d61696c2e20444e532d74797065732e20526f757465722e20446173206973742065696e20426569737069656c20667565722065696e656e2062656e75747a6572646566696e69657274656e20537472696e672e204f7267616e697a6174696f6e616c206964656e7469666965723a204c4d55203132372e2020564c414e20494420616e6420564c414e204e616d653a203132332e204d4e4d2d564c414e0000
