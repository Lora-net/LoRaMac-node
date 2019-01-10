# TRxx FUOTA interoperability tests

## Test 1 - Basic multicast + fragmentation setup followed by file broadcast

This is a minimalistic test case where only 5 commands in total are used. This test assumes that the following commands are implemented

* AppTimeReq & Ans
* McGroupSetupReq & Ans
* McClassCsessionReq & Ans
* FragSessionSetupReq & Ans
* DataFragment

### Test overview

1. End-device boots, joins network
2. End-device periodically (in average 30sec) sends AppTimeReq. Those uplinks provide opportunity to send downlink commands.
3. Server sends AppTimeAns with a time correction
4. Server waits for the next AppTimeReq uplink to check that end-device time is now correct (+/- 2sec) and token has been incremented. If not, a command was probably lost, retry at step 3.

5. Server creates a multicast group with its associated key material
6. A fragmentation session is created. 995 bytes will be sent in 20 fragments of 50 bytes
7. A ClassC session is created starting 1 minute after current time
8. When ClassC session opens, 20 data fragments + 5 redundancy fragments (total 25) fragments are sent by the network 
9. At the end of the session, the end-device periodically requests authentication of the received file. The authentication request contains a CRC of the file, enabling server to check the completeness of the file.
10. Server checks that the CRC corresponds to the CRC of the file that was sent. If yes, the full test is successful.

### Current status

All required functionality is implemented. Please see below end-device execution log.


```
McKEKey       : 2C 57 8F 79 27 A9 49 D3 B5 11 AE 8F B6 91 45 C6

McKeyEncrypted: 01 5E 85 F4 B9 9D C0 B9 44 06 6C D0 74 98 33 0B
                             ||
McKey       = aes128_encrypt(McKeyEncryptionKey, McKeyEncrypted)
                             ||
                             \/
McKey         : 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 - **OK**
                             ||
McAppSKey = aes128_encrypt(McKey, [01 FF FF FF 01 00 00 00 00 00 00 00 00 00 00 00])
McNwkSKey = aes128_encrypt(McKey, [02 FF FF FF 01 00 00 00 00 00 00 00 00 00 00 00])
                             ||
                             \/
McAppSKey     : C3 F6 C3 9B 6B 64 96 C2 96 29 F7 E7 E9 B0 CD 29 - **OK**
McNwkSKey     : BB 75 C3 62 58 8F 5D 65 FC C6 1C 08 0B 76 DB A3 - **OK**

```

#### Execution

The end-device prints messages on the UART. In order to observe those messages one must setup a terminal using the below settings.
UART: 921600 8N1

Please find below an example of UART messages displayed while executing this test case.

```

###### ===== Demo Application v1.0.RC1 ===== ######

DevEui      : 0A-50-32-59-5F-50-6F-59
AppEui      : 00-00-00-00-00-00-00-00
AppKey      : 2B 7E 15 16 28 AE D2 A6 AB F7 15 88 09 CF 4F 3C

###### ===========   JOINED     ============ ######

ABP

DevAddr     : 00D13A6B
NwkSKey     : 2B 7E 15 16 28 AE D2 A6 AB F7 15 88 09 CF 4F 3C
AppSKey     : 2B 7E 15 16 28 AE D2 A6 AB F7 15 88 09 CF 4F 3C


###### =========== MCPS-Request ============ ######
######           MCPS_UNCONFIRMED            ######
###### ===================================== ######
STATUS      : OK

###### =========== MCPS-Confirm ============ ######
STATUS      : OK

###### =====   UPLINK FRAME        1   ===== ######

CLASS       : A

TX PORT     : 202
TX DATA     : UNCONFIRMED
01 06 FD 61 49 00

DATA RATE   : DR_3
U/L FREQ    : 868500000
TX POWER    : 0
CHANNEL MASK: 0007


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      888  ===== ######
RX WINDOW   : 1
RX PORT     : 202
RX DATA     :
01 CC 01 00 00 00

DATA RATE   : DR_3
RX RSSI     : -79
RX SNR      : 11


###### =========== MCPS-Request ============ ######
######           MCPS_UNCONFIRMED            ######
###### ===================================== ######
STATUS      : OK

###### =========== MCPS-Confirm ============ ######
STATUS      : OK

###### =====   UPLINK FRAME        2   ===== ######

CLASS       : A

TX PORT     : 202
TX DATA     : UNCONFIRMED
01 FC FE 61 49 01

DATA RATE   : DR_3
U/L FREQ    : 868300000
TX POWER    : 0
CHANNEL MASK: 0007


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      889  ===== ######
RX WINDOW   : 1
RX PORT     : 202
RX DATA     :
01 00 00 00 00 01

DATA RATE   : DR_3
RX RSSI     : -79
RX SNR      : 11


###### =========== MCPS-Request ============ ######
######           MCPS_UNCONFIRMED            ######
###### ===================================== ######
STATUS      : OK

###### =========== MCPS-Confirm ============ ######
STATUS      : OK

###### =====   UPLINK FRAME        3   ===== ######

CLASS       : A

TX PORT     : 1
TX DATA     : UNCONFIRMED
0E

DATA RATE   : DR_3
U/L FREQ    : 868300000
TX POWER    : 0
CHANNEL MASK: 0007


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      890  ===== ######
RX WINDOW   : 1
RX PORT     : 200
RX DATA     :
02 00 FF FF FF 01 01 5E 85 F4 B9 9D C0 B9 44 06
6C D0 74 98 33 0B 00 00 00 00 FF 00 00 00

DATA RATE   : DR_3
RX RSSI     : -80
RX SNR      : 11


###### =========== MCPS-Request ============ ######
######           MCPS_UNCONFIRMED            ######
###### ===================================== ######
STATUS      : OK
ID          : 0
McAddr      : 01FFFFFF
McKey       : 01-5E-85-F4-B9-9D-C0-B9-44-06-6C-D0-74-98-33-0B
McFCountMin : 0
McFCountMax : 255
SessionTime : 0
SessionTimeT: 0
Rx Freq     : 0
Rx DR       : DR_0

###### =========== MCPS-Confirm ============ ######
STATUS      : OK

###### =====   UPLINK FRAME        4   ===== ######

CLASS       : A

TX PORT     : 200
TX DATA     : UNCONFIRMED
02 00

DATA RATE   : DR_5
U/L FREQ    : 868500000
TX POWER    : 3
CHANNEL MASK: 0007


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      891  ===== ######
RX WINDOW   : 1
RX PORT     : 0

DATA RATE   : DR_5
RX RSSI     : -79
RX SNR      : 13


###### =========== MCPS-Request ============ ######
######           MCPS_UNCONFIRMED            ######
###### ===================================== ######
STATUS      : OK

###### =========== MCPS-Confirm ============ ######
STATUS      : OK

###### =====   UPLINK FRAME        5   ===== ######

CLASS       : A

TX PORT     : 1
TX DATA     : UNCONFIRMED
09

DATA RATE   : DR_5
U/L FREQ    : 868300000
TX POWER    : 4
CHANNEL MASK: 0007


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      892  ===== ######
RX WINDOW   : 1
RX PORT     : 201
RX DATA     :
02 00 15 00 30 00 0D 00 00 00 00

DATA RATE   : DR_5
RX RSSI     : -79
RX SNR      : 13


###### =========== MCPS-Request ============ ######
######           MCPS_UNCONFIRMED            ######
###### ===================================== ######
STATUS      : OK

###### =========== MCPS-Confirm ============ ######
STATUS      : OK

###### =====   UPLINK FRAME        6   ===== ######

CLASS       : A

TX PORT     : 201
TX DATA     : UNCONFIRMED
02 00

DATA RATE   : DR_5
U/L FREQ    : 868300000
TX POWER    : 6
CHANNEL MASK: 0007


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      893  ===== ######
RX WINDOW   : 1
RX PORT     : 0

DATA RATE   : DR_5
RX RSSI     : -79
RX SNR      : 13


###### =========== MCPS-Request ============ ######
######           MCPS_UNCONFIRMED            ######
###### ===================================== ######
STATUS      : OK

###### =========== MCPS-Confirm ============ ######
STATUS      : OK

###### =====   UPLINK FRAME        7   ===== ######

CLASS       : A

TX PORT     : 1
TX DATA     : UNCONFIRMED
C5

DATA RATE   : DR_5
U/L FREQ    : 868300000
TX POWER    : 7
CHANNEL MASK: 0007


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      894  ===== ######
RX WINDOW   : 1
RX PORT     : 200
RX DATA     :
04 00 8C FF 61 49 07 D2 AD 84 00

DATA RATE   : DR_5
RX RSSI     : -78
RX SNR      : 13

Time2SessionStart: 25000 ms

###### =========== MCPS-Request ============ ######
######           MCPS_UNCONFIRMED            ######
###### ===================================== ######
STATUS      : OK
ID          : 0
McAddr      : 01FFFFFF
McKey       : 01-5E-85-F4-B9-9D-C0-B9-44-06-6C-D0-74-98-33-0B
McFCountMin : 0
McFCountMax : 255
SessionTime : 1547123980
SessionTimeT: 7
Rx Freq     : 869525000
Rx DR       : DR_0

###### =========== MCPS-Confirm ============ ######
STATUS      : OK

###### =====   UPLINK FRAME        8   ===== ######

CLASS       : A

TX PORT     : 200
TX DATA     : UNCONFIRMED
02 00 19 00 00

DATA RATE   : DR_5
U/L FREQ    : 868300000
TX POWER    : 7
CHANNEL MASK: 0007



###### ===== Switch to Class C done.  ===== ######


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      110  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 01 00 6A B8 00 4D 25 18 2F 58 65 89 6B AF 34
E0 07 AB 6A 8E 24 33 CC F7 50 B1 DF E4 16 0A 2B
E0 19 6B F4 88 B0 50 AF D5 05 BF FC BF 48 C9 1A
72 E8 4B

DATA RATE   : DR_0
RX RSSI     : -80
RX SNR      : 6


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    :     1 /    21 Fragments
                 48 /  1008 Bytes
LOST        :             0 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      111  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 02 00 49 21 05 AD 36 44 7D 0E 92 25 96 B2 1A
6A B1 6A 0D 89 A9 83 F1 96 E6 23 24 CE 65 2A ED
59 BF B9 E1 9F BF 59 45 E4 6D F6 A9 9F 1D F2 73
93 68 3C

DATA RATE   : DR_0
RX RSSI     : -80
RX SNR      : 7


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    :     2 /    21 Fragments
                 96 /  1008 Bytes
LOST        :             0 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      112  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 03 00 E6 92 01 9D 53 86 E2 5B E8 9F 04 ED B0
FE 2C 23 EE B2 11 C1 C0 EB B5 20 05 07 07 3F DB
89 8D D7 20 47 95 F7 8F 05 CC 3B CE 63 DC BF 8E
23 0F 1F

DATA RATE   : DR_0
RX RSSI     : -79
RX SNR      : 6


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    :     3 /    21 Fragments
                144 /  1008 Bytes
LOST        :             0 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      113  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 04 00 0B 1B 3A B6 8F 03 12 F7 91 34 40 BE 32
94 F7 D8 3D 7E 9E D3 28 05 12 7C 9B 91 51 FC 94
61 8C BE AB 44 11 5E A1 36 C0 11 42 CD 31 A3 86
EC 43 11

DATA RATE   : DR_0
RX RSSI     : -80
RX SNR      : 7


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ======================================######
RECEIVED    :     4 /    21 Fragments
                192 /  1008 Bytes
LOST        :             0 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      114  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 05 00 BB C5 E7 EE 04 3C 9D F2 F2 8E E9 A4 63
7C 9A 8C EC EA 65 F6 2C 20 22 81 05 F2 D3 04 2D
55 21 CE 58 F0 94 E0 D7 E7 75 8B CC 49 7D 99 04
97 6F CE

DATA RATE   : DR_0
RX RSSI     : -80
RX SNR      : 7


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    :     5 /    21 Fragments
                240 /  1008 Bytes
LOST        :             0 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      115  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 06 00 50 E4 93 2F C9 9C 0E 6B AD EA 00 F9 60
F8 9A D3 93 A0 49 96 BF DB C1 B2 DC 52 AB 73 61
69 66 51 9F 6E F8 AD 33 6D 58 CB E0 E6 A9 45 40
DA 87 CD

DATA RATE   : DR_0
RX RSSI     : -79
RX SNR      : 7


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    :     6 /    21 Fragments
                288 /  1008 Bytes
LOST        :             0 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      116  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 07 00 92 BB 84 C5 91 77 57 11 60 14 FB 2E CF
DF B0 91 29 77 58 39 97 50 EA E8 42 1C 31 7F BA
35 3F D9 6A 9D 3C 1A 84 7A 27 9F 8B A7 25 C0 39
84 C8 06

DATA RATE   : DR_0
RX RSSI     : -81
RX SNR      : 7


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    :     7 /    21 Fragments
                336 /  1008 Bytes
LOST        :             0 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      117  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 08 00 53 DF D7 89 DD F2 D3 DA 19 A6 B3 9C CC
09 C4 BB 42 42 A1 58 CB 72 C8 FD 4D 24 E6 8A F9
A2 FD 8B 86 23 5B 07 29 BE 08 5D DC B1 B0 30 71
94 FC 34

DATA RATE   : DR_0
RX RSSI     : -80
RX SNR      : 7


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    :     8 /    21 Fragments
                384 /  1008 Bytes
LOST        :             0 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      118  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 09 00 3F 43 BF 75 0F 82 36 CC 4C 07 97 D7 61
BF 82 8A F5 CD 08 B5 77 F2 38 44 15 6D 1C A2 CD
B2 C3 57 D8 6D D2 A0 25 14 05 11 75 1D 07 C0 65
BE 73 73

DATA RATE   : DR_0
RX RSSI     : -81
RX SNR      : 7


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    :     9 /    21 Fragments
                432 /  1008 Bytes
LOST        :             0 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      119  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 0A 00 7A 79 CD 67 E7 09 C5 20 9E 03 89 01 F3
E7 CB E9 25 28 30 9F E7 FC B5 BB E8 66 40 2C 1E
CF 25 43 D1 4F FB 44 88 50 E8 5D 6F 83 EF 08 B7
E3 07 85

DATA RATE   : DR_0
RX RSSI     : -80
RX SNR      : 5


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    :    10 /    21 Fragments
                480 /  1008 Bytes
LOST        :             0 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      120  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 0B 00 53 DB 8E B0 73 A0 4A 02 93 4F 84 EA 6D
3F 5F EE EF D7 EB 3A 16 3A 50 2D 9B 69 D0 2F B3
3D 92 59 0F 3A A9 7F 84 2D 92 FE D0 98 F9 E6 98
08 18 11

DATA RATE   : DR_0
RX RSSI     : -81
RX SNR      : 7


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    :    11 /    21 Fragments
                528 /  1008 Bytes
LOST        :             0 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      121  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 0C 00 73 60 F9 2B F8 C4 D2 A1 AB 7A 03 5A 7D
BA 78 75 23 03 C1 52 FB 38 56 86 C0 76 20 50 81
AC C4 21 06 84 CF 03 AB AF 73 E9 A4 01 7C DB D4
A6 AC 94

DATA RATE   : DR_0
RX RSSI     : -80
RX SNR      : 7


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    :    12 /    21 Fragments
                576 /  1008 Bytes
LOST        :             0 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      122  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 0D 00 46 8F AB 5A DA 32 BF 4A C5 6D CE 5A 36
C4 4F BB BE 38 37 33 24 60 07 1C AC CC 15 3B 35
EA B5 8D 4E D5 6F EB B4 7A 20 F9 29 34 6E 67 25
BA 30 A4

DATA RATE   : DR_0
RX RSSI     : -80
RX SNR      : 7


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    :    13 /    21 Fragments
                624 /  1008 Bytes
LOST        :             0 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      123  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 0E 00 C0 36 99 BF A3 98 4B BB F1 6D C7 0E D5
31 65 4D 14 E7 5E 87 7E 22 35 13 82 43 5B 1C C9
1B FB 2D 92 0B C9 30 87 BD 26 8D 37 C2 B8 2D DC
05 DB 8F

DATA RATE   : DR_0
RX RSSI     : -81
RX SNR      : 7


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    :    14 /    21 Fragments
                672 /  1008 Bytes
LOST        :             0 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      124  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 0F 00 67 C1 B7 FC 47 01 EE DB BA 84 B4 C7 60
C4 BF 9C 66 B2 01 C6 E5 3D 1F 38 4D E1 8B 49 23
4A 9D 53 75 71 D3 6D 58 AC 38 77 50 A0 E0 72 C8
75 A7 22

DATA RATE   : DR_0
RX RSSI     : -80
RX SNR      : 6


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    :    15 /    21 Fragments
                720 /  1008 Bytes
LOST        :             0 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      125  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 10 00 6E E8 9A C4 81 7F D7 11 92 F0 84 32 D8
40 B3 8A F2 9F D6 02 FC 14 52 F1 02 D2 DC 70 41
CD 7A 22 ED E4 7D DA 6B AE 65 81 30 F6 4B 1A 25
04 B7 90

DATA RATE   : DR_0
RX RSSI     : -80
RX SNR      : 6


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    :    16 /    21 Fragments
                768 /  1008 Bytes
LOST        :             0 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      126  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 11 00 CB 81 CA B1 C6 68 A5 2E 52 2C 68 3E 68
F9 52 FB A2 60 DB 9E 40 CA 6E 5B 54 B2 45 CE 4B
8B 7C DA E3 2F 95 E5 72 EB 47 9B AE 3A 04 6A EF
57 C7 2D

DATA RATE   : DR_0
RX RSSI     : -80
RX SNR      : 7


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    :    17 /    21 Fragments
                816 /  1008 Bytes
LOST        :             0 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      127  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 12 00 57 25 B7 B2 B0 41 B1 3A 6C 5F 5B 0F A1
B4 9C A5 2B 26 83 DF 2F 76 6D 7F 29 57 43 D7 CC
6D 9B 25 82 4C DB AB A2 20 78 FC F2 A4 27 A3 90
78 6D 99

DATA RATE   : DR_0
RX RSSI     : -82
RX SNR      : 8


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    :    18 /    21 Fragments
                864 /  1008 Bytes
LOST        :             0 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      128  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 13 00 D9 C0 94 EC 11 FD 0E 33 6C 1B 9F 0C 49
10 B3 AA 61 30 BE 57 CB 7C 86 07 A4 59 3A 6F 62
78 FA 5D C5 8D E3 5B 3F E8 0B F2 8E 60 FE 0F 84
08 92 2E

DATA RATE   : DR_0
RX RSSI     : -81
RX SNR      : 7


###### =========== FRAG_DECODER ============ ######
######               PROGRES                 ######
###### ===================================== ######
RECEIVED    :    19 /    21 Fragments
                912 /  1008 Bytes
LOST        :             0 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      129  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 14 00 A1 FA DF 73 B5 C6 7E 87 26 5E 24 B9 7A
72 E2 87 68 45 12 6B 07 4A 80 F6 1C AC 7F C6 25
15 66 CB 31 C4 4A 37 04 66 61 A8 12 27 04 1D A6
67 52 8E

DATA RATE   : DR_0
RX RSSI     : -80
RX SNR      : 7


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    :    20 /    21 Fragments
                960 /  1008 Bytes
LOST        :             0 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      130  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 15 00 FD D5 B2 EA 0A 12 79 59 EF 7D 8A E4 72
E0 41 46 54 8C 38 AB 24 18 DE 3C 62 92 86 13 DF
F3 CF 48 87 57 8D 00 00 00 00 00 00 00 00 00 00
00 00 00

DATA RATE   : DR_0
RX RSSI     : -80
RX SNR      : 5


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    :    21 /    21 Fragments
               1008 /  1008 Bytes
LOST        :             0 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      131  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 16 00 DA D5 81 DE 13 80 ED 0F BA 6B 15 72 FE
44 A3 5C 9F 2A CF 70 1F FE BA 9B AF FC 85 A6 FB
C9 15 25 17 27 ED 26 0B DC 1A 19 11 59 8F 1E 6C
0D 5E 93

DATA RATE   : DR_0
RX RSSI     : -80
RX SNR      : 7


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    :    22 /    21 Fragments
               1056 /  1008 Bytes
LOST        :             0 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      132  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 17 00 1C 65 43 9C 7B 4E 7A 30 F4 9F 08 71 48
ED 2B A8 AD C4 F0 97 EB BE AB 4A 17 3E C7 84 0B
C1 73 13 92 5C 00 98 64 97 BA 3D 34 F9 A3 4C 52
07 BE 5F

DATA RATE   : DR_0
RX RSSI     : -80
RX SNR      : 7


###### =========== FRAG_DECODER ============ ######
######               FINISHED                ######
###### ===================================== ######
STATUS      : -2
CRC         : ECB2A918


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      133  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 18 00 07 07 85 1C E1 E8 B3 31 EE B6 E9 55 F7
D2 38 83 08 FB 56 67 05 94 46 A6 8C 24 D9 D9 7B
8B 86 B3 F4 66 43 A9 1F 79 76 77 72 F4 0F 86 42
26 A1 B0

DATA RATE   : DR_0
RX RSSI     : -80
RX SNR      : 7


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      134  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 19 00 15 55 28 FF 7C 89 93 C1 28 E0 AF B9 DD
35 9D 06 5A 79 20 20 82 BA 4E 18 4B 0C 8E 6D 64
51 37 6C F9 B6 A2 93 54 C8 A1 82 86 EE C8 06 AD
BC 68 AC

DATA RATE   : DR_0
RX RSSI     : -80
RX SNR      : 7


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      135  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 1A 00 75 A3 73 ED 4E 3C 27 1F EE 01 16 70 C3
58 D0 19 BF 97 19 ED B3 19 A8 9F CF A8 3B CE A0
80 E2 E1 C1 64 6E C4 DF DD B6 41 C0 C1 16 18 AF
10 5C D7

DATA RATE   : DR_0
RX RSSI     : -80
RX SNR      : 7



###### ===== Switch to Class A done.  ===== ######


###### =========== MCPS-Request ============ ######
######           MCPS_UNCONFIRMED            ######
###### ===================================== ######
STATUS      : OK

###### =========== MCPS-Confirm ============ ######
STATUS      : OK

###### =====   UPLINK FRAME        9   ===== ######

CLASS       : A

TX PORT     : 201
TX DATA     : UNCONFIRMED
05 18 A9 B2 EC

DATA RATE   : DR_5
U/L FREQ    : 868500000
TX POWER    : 7
CHANNEL MASK: 0007


###### =========== MCPS-Request ============ ######
######           MCPS_UNCONFIRMED            ######
###### ===================================== ######
STATUS      : OK

###### =========== MCPS-Confirm ============ ######
STATUS      : OK

###### =====   UPLINK FRAME       10   ===== ######

CLASS       : A

TX PORT     : 201
TX DATA     : UNCONFIRMED
05 18 A9 B2 EC

DATA RATE   : DR_5
U/L FREQ    : 868100000
TX POWER    : 7
CHANNEL MASK: 0007

```