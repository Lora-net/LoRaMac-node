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
4. Server waits for the next AppTimeReq uplink to check that end-device’s time is now correct (+/- 2sec) and token has been incremented. If not, a command was probably lost, retry at step 3.

5. Server creates a multicast group with its associated key material
6. A fragmentation session is created. 995 bytes will be sent in 20 fragments of 50 bytes
7. A ClassC session is created starting 1 minute after current time
8. When ClassC session opens, 20 data fragments + 5 redundancy fragments (total 25) fragments are sent by the network 
9. At the end of the session, the end-device periodically requests authentication of the received file. The authentication request contains a CRC of the file, enabling server to check the completeness of the file.
10. Server checks that the CRC corresponds to the CRC of the file that was sent. If yes, the full test is successful.

### Current status

All required functionality is implemented except the final file reconstruction (FragDecoder.c).

Currently there is an issue concerning the used keys. The keys derivation process used in this project don't end up in to the same set of keys as the ones in the specification.

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

Note: As previously said the Fragmentation decoder is not yet functional.

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
01 1E 6D 3E 49 00

DATA RATE   : DR_5
U/L FREQ    : 868300000
TX POWER    : 0
CHANNEL MASK: 0007


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      373  ===== ######
RX WINDOW   : 1
RX PORT     : 202
RX DATA     :
01 5A 15 00 00 00

DATA RATE   : DR_5
RX RSSI     : -89
RX SNR      : 10


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
01 97 82 3E 49 01

DATA RATE   : DR_5
U/L FREQ    : 868300000
TX POWER    : 0
CHANNEL MASK: 0007


###### =========== MCPS-Request ============ ######
######           MCPS_UNCONFIRMED            ######
###### ===================================== ######
STATUS      : OK

###### =========== MCPS-Confirm ============ ######
STATUS      : OK

###### =====   UPLINK FRAME        3   ===== ######

CLASS       : A

TX PORT     : 202
TX DATA     : UNCONFIRMED
01 B8 82 3E 49 01

DATA RATE   : DR_5
U/L FREQ    : 868500000
TX POWER    : 0
CHANNEL MASK: 0007


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      375  ===== ######
RX WINDOW   : 1
RX PORT     : 202
RX DATA     :
01 00 00 00 00 01

DATA RATE   : DR_5
RX RSSI     : -85
RX SNR      : 12


###### =========== MCPS-Request ============ ######
######           MCPS_UNCONFIRMED            ######
###### ===================================== ######
STATUS      : OK

###### =========== MCPS-Confirm ============ ######
STATUS      : OK

###### =====   UPLINK FRAME        4   ===== ######

CLASS       : A

TX PORT     : 1
TX DATA     : UNCONFIRMED
99

DATA RATE   : DR_5
U/L FREQ    : 868500000
TX POWER    : 0
CHANNEL MASK: 0007


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      376  ===== ######
RX WINDOW   : 1
RX PORT     : 200
RX DATA     :
02 00 FE FF FF FF 01 5E 85 F4 B9 9D C0 B9 44 06
6C D0 74 98 33 0B 00 00 00 00 FF 00 00 00

DATA RATE   : DR_5
RX RSSI     : -86
RX SNR      : 12


###### =========== MCPS-Request ============ ######
######           MCPS_UNCONFIRMED            ######
###### ===================================== ######
STATUS      : OK
ID          : 0
McAddr      : FFFFFFFE
McKey       : 01-5E-85-F4-B9-9D-C0-B9-44-06-6C-D0-74-98-33-0B
McFCountMin : 0
McFCountMax : 255
SessionTime : 0
SessionTimeT: 0
Rx Freq     : 0
Rx DR       : DR_0

###### =========== MCPS-Confirm ============ ######
STATUS      : OK

###### =====   UPLINK FRAME        5   ===== ######

CLASS       : A

TX PORT     : 200
TX DATA     : UNCONFIRMED
02 00

DATA RATE   : DR_5
U/L FREQ    : 868300000
TX POWER    : 0
CHANNEL MASK: 0007


###### =========== MCPS-Request ============ ######
######           MCPS_UNCONFIRMED            ######
###### ===================================== ######
STATUS      : OK

###### =========== MCPS-Confirm ============ ######
STATUS      : OK

###### =====   UPLINK FRAME        6   ===== ######

CLASS       : A

TX PORT     : 1
TX DATA     : UNCONFIRMED
00

DATA RATE   : DR_5
U/L FREQ    : 868300000
TX POWER    : 0
CHANNEL MASK: 0007


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      377  ===== ######
RX WINDOW   : 1
RX PORT     : 201
RX DATA     :
02 00 19 00 28 00 05 00 00 00 00

DATA RATE   : DR_5
RX RSSI     : -87
RX SNR      : 11


###### =========== MCPS-Request ============ ######
######           MCPS_UNCONFIRMED            ######
###### ===================================== ######
STATUS      : OK

###### =========== MCPS-Confirm ============ ######
STATUS      : OK

###### =====   UPLINK FRAME        7   ===== ######

CLASS       : A

TX PORT     : 201
TX DATA     : UNCONFIRMED
02 00

DATA RATE   : DR_5
U/L FREQ    : 868300000
TX POWER    : 0
CHANNEL MASK: 0007


###### =========== MCPS-Request ============ ######
######           MCPS_UNCONFIRMED            ######
###### ===================================== ######
STATUS      : OK

###### =========== MCPS-Confirm ============ ######
STATUS      : OK

###### =====   UPLINK FRAME        8   ===== ######

CLASS       : A

TX PORT     : 1
TX DATA     : UNCONFIRMED
00

DATA RATE   : DR_5
U/L FREQ    : 868300000
TX POWER    : 0
CHANNEL MASK: 0007


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      378  ===== ######
RX WINDOW   : 1
RX PORT     : 200
RX DATA     :
04 00 32 83 3E 49 07 D2 AD 84 00

DATA RATE   : DR_5
RX RSSI     : -86
RX SNR      : 10

Time2SessionStart: 29000 ms

###### =========== MCPS-Request ============ ######
######           MCPS_UNCONFIRMED            ######
###### ===================================== ######
STATUS      : OK
ID          : 0
McAddr      : FFFFFFFE
McKey       : 01-5E-85-F4-B9-9D-C0-B9-44-06-6C-D0-74-98-33-0B
McFCountMin : 0
McFCountMax : 255
SessionTime : 1544798386
SessionTimeT: 7
Rx Freq     : 869525000
Rx DR       : DR_0

###### =========== MCPS-Confirm ============ ######
STATUS      : OK

###### =====   UPLINK FRAME        9   ===== ######

CLASS       : A

TX PORT     : 200
TX DATA     : UNCONFIRMED
02 00 1D 00 00

DATA RATE   : DR_5
U/L FREQ    : 868100000
TX POWER    : 0
CHANNEL MASK: 0007



###### ===== Switch to Class C done.  ===== ######


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      241  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 01 00 6A B8 00 4D 25 18 2F 58 65 89 6B AF 34
E0 07 AB 6A 8E 24 33 CC F7 50 B1 DF E4 16 0A 2B
E0 19 6B F4 88 B0 50 AF D5 05 BF

DATA RATE   : DR_0
RX RSSI     : -85
RX SNR      : 6


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    : 0/25 Fragments
              0/1000 Bytes
LOST        :    4608 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      242  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 02 00 FC BF 48 C9 1A 72 E8 4B 49 21 05 AD 36
44 7D 0E 92 25 96 B2 1A 6A B1 6A 0D 89 A9 83 F1
96 E6 23 24 CE 65 2A ED 59 BF B9

DATA RATE   : DR_0
RX RSSI     : -85
RX SNR      : 6


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    : 0/25 Fragments
              0/1000 Bytes
LOST        :    4608 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      243  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 03 00 E1 9F BF 59 45 E4 6D F6 A9 9F 1D F2 73
93 68 3C E6 92 01 9D 53 86 E2 5B E8 9F 04 ED B0
FE 2C 23 EE B2 11 C1 C0 EB B5 20

DATA RATE   : DR_0
RX RSSI     : -85
RX SNR      : 5


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    : 0/25 Fragments
              0/1000 Bytes
LOST        :    4608 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      244  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 04 00 05 07 07 3F DB 89 8D D7 20 47 95 F7 8F
05 CC 3B CE 63 DC BF 8E 23 0F 1F 0B 1B 3A B6 8F
03 12 F7 91 34 40 BE 32 94 F7 D8

DATA RATE   : DR_0
RX RSSI     : -85
RX SNR      : 6


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    : 0/25 Fragments
              0/1000 Bytes
LOST        :    4608 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      245  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 05 00 3D 7E 9E D3 28 05 12 7C 9B 91 51 FC 94
61 8C BE AB 44 11 5E A1 36 C0 11 42 CD 31 A3 86
EC 43 11 BB C5 E7 EE 04 3C 9D F2

DATA RATE   : DR_0
RX RSSI     : -84
RX SNR      : 6


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    : 0/25 Fragments
              0/1000 Bytes
LOST        :    4608 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      246  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 06 00 F2 8E E9 A4 63 7C 9A 8C EC EA 65 F6 2C
20 22 81 05 F2 D3 04 2D 55 21 CE 58 F0 94 E0 D7
E7 75 8B CC 49 7D 99 04 97 6F CE

DATA RATE   : DR_0
RX RSSI     : -85
RX SNR      : 6


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    : 0/25 Fragments
              0/1000 Bytes
LOST        :    4608 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      247  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 07 00 50 E4 93 2F C9 9C 0E 6B AD EA 00 F9 60
F8 9A D3 93 A0 49 96 BF DB C1 B2 DC 52 AB 73 61
69 66 51 9F 6E F8 AD 33 6D 58 CB

DATA RATE   : DR_0
RX RSSI     : -85
RX SNR      : 6


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    : 0/25 Fragments
              0/1000 Bytes
LOST        :    4608 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      248  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 08 00 E0 E6 A9 45 40 DA 87 CD 92 BB 84 C5 91
77 57 11 60 14 FB 2E CF DF B0 91 29 77 58 39 97
50 EA E8 42 1C 31 7F BA 35 3F D9

DATA RATE   : DR_0
RX RSSI     : -85
RX SNR      : 6


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    : 0/25 Fragments
              0/1000 Bytes
LOST        :    4608 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      249  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 09 00 6A 9D 3C 1A 84 7A 27 9F 8B A7 25 C0 39
84 C8 06 53 DF D7 89 DD F2 D3 DA 19 A6 B3 9C CC
09 C4 BB 42 42 A1 58 CB 72 C8 FD

DATA RATE   : DR_0
RX RSSI     : -85
RX SNR      : 6


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    : 0/25 Fragments
              0/1000 Bytes
LOST        :    4608 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      250  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 0A 00 4D 24 E6 8A F9 A2 FD 8B 86 23 5B 07 29
BE 08 5D DC B1 B0 30 71 94 FC 34 3F 43 BF 75 0F
82 36 CC 4C 07 97 D7 61 BF 82 8A

DATA RATE   : DR_0
RX RSSI     : -85
RX SNR      : 6


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    : 0/25 Fragments
              0/1000 Bytes
LOST        :    4608 Fragments


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    : 0/25 Fragments
              0/1000 Bytes
LOST        :    4608 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      251  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 0B 00 F5 CD 08 B5 77 F2 38 44 15 6D 1C A2 CD
B2 C3 57 D8 6D D2 A0 25 14 05 11 75 1D 07 C0 65
BE 73 73 7A 79 CD 67 E7 09 C5 20

DATA RATE   : DR_0
RX RSSI     : -85
RX SNR      : 6


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    : 0/25 Fragments
              0/1000 Bytes
LOST        :    4608 Fragments


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    : 0/25 Fragments
              0/1000 Bytes
LOST        :    4608 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      252  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 0C 00 9E 03 89 01 F3 E7 CB E9 25 28 30 9F E7
FC B5 BB E8 66 40 2C 1E CF 25 43 D1 4F FB 44 88
50 E8 5D 6F 83 EF 08 B7 E3 07 85

DATA RATE   : DR_0
RX RSSI     : -85
RX SNR      : 5


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    : 0/25 Fragments
              0/1000 Bytes
LOST        :    4608 Fragments


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    : 0/25 Fragments
              0/1000 Bytes
LOST        :    4608 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      253  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 0D 00 53 DB 8E B0 73 A0 4A 02 93 4F 84 EA 6D
3F 5F EE EF D7 EB 3A 16 3A 50 2D 9B 69 D0 2F B3
3D 92 59 0F 3A A9 7F 84 2D 92 FE

DATA RATE   : DR_0
RX RSSI     : -85
RX SNR      : 6


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    : 0/25 Fragments
              0/1000 Bytes
LOST        :    4608 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      254  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 0E 00 D0 98 F9 E6 98 08 18 11 73 60 F9 2B F8
C4 D2 A1 AB 7A 03 5A 7D BA 78 75 23 03 C1 52 FB
38 56 86 C0 76 20 50 81 AC C4 21

DATA RATE   : DR_0
RX RSSI     : -85
RX SNR      : 6


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    : 0/25 Fragments
              0/1000 Bytes
LOST        :    4608 Fragments


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    : 0/25 Fragments
              0/1000 Bytes
LOST        :    4608 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      255  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 0F 00 06 84 CF 03 AB AF 73 E9 A4 01 7C DB D4
A6 AC 94 46 8F AB 5A DA 32 BF 4A C5 6D CE 5A 36
C4 4F BB BE 38 37 33 24 60 07 1C

DATA RATE   : DR_0
RX RSSI     : -85
RX SNR      : 6


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    : 0/25 Fragments
              0/1000 Bytes
LOST        :    4608 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      256  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 10 00 AC CC 15 3B 35 EA B5 8D 4E D5 6F EB B4
7A 20 F9 29 34 6E 67 25 BA 30 A4 C0 36 99 BF A3
98 4B BB F1 6D C7 0E D5 31 65 4D

DATA RATE   : DR_0
RX RSSI     : -85
RX SNR      : 6


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    : 0/25 Fragments
              0/1000 Bytes
LOST        :    4608 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      257  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 11 00 14 E7 5E 87 7E 22 35 13 82 43 5B 1C C9
1B FB 2D 92 0B C9 30 87 BD 26 8D 37 C2 B8 2D DC
05 DB 8F 67 C1 B7 FC 47 01 EE DB

DATA RATE   : DR_0
RX RSSI     : -85
RX SNR      : 6


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    : 0/25 Fragments
              0/1000 Bytes
LOST        :    4608 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      258  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 12 00 BA 84 B4 C7 60 C4 BF 9C 66 B2 01 C6 E5
3D 1F 38 4D E1 8B 49 23 4A 9D 53 75 71 D3 6D 58
AC 38 77 50 A0 E0 72 C8 75 A7 22

DATA RATE   : DR_0
RX RSSI     : -85
RX SNR      : 6


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    : 0/25 Fragments
              0/1000 Bytes
LOST        :    4608 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      259  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 13 00 6E E8 9A C4 81 7F D7 11 92 F0 84 32 D8
40 B3 8A F2 9F D6 02 FC 14 52 F1 02 D2 DC 70 41
CD 7A 22 ED E4 7D DA 6B AE 65 81

DATA RATE   : DR_0
RX RSSI     : -85
RX SNR      : 6


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    : 0/25 Fragments
              0/1000 Bytes
LOST        :    4608 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      260  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 14 00 30 F6 4B 1A 25 04 B7 90 CB 81 CA B1 C6
68 A5 2E 52 2C 68 3E 68 F9 52 FB A2 60 DB 9E 40
CA 6E 5B 54 B2 45 CE 4B 8B 7C DA

DATA RATE   : DR_0
RX RSSI     : -85
RX SNR      : 6


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    : 0/25 Fragments
              0/1000 Bytes
LOST        :    4608 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      261  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 15 00 E3 2F 95 E5 72 EB 47 9B AE 3A 04 6A EF
57 C7 2D 57 25 B7 B2 B0 41 B1 3A 6C 5F 5B 0F A1
B4 9C A5 2B 26 83 DF 2F 76 6D 7F

DATA RATE   : DR_0
RX RSSI     : -85
RX SNR      : 6


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    : 0/25 Fragments
              0/1000 Bytes
LOST        :    4608 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      262  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 16 00 29 57 43 D7 CC 6D 9B 25 82 4C DB AB A2
20 78 FC F2 A4 27 A3 90 78 6D 99 D9 C0 94 EC 11
FD 0E 33 6C 1B 9F 0C 49 10 B3 AA

DATA RATE   : DR_0
RX RSSI     : -85
RX SNR      : 6


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    : 0/25 Fragments
              0/1000 Bytes
LOST        :    4608 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      263  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 17 00 61 30 BE 57 CB 7C 86 07 A4 59 3A 6F 62
78 FA 5D C5 8D E3 5B 3F E8 0B F2 8E 60 FE 0F 84
08 92 2E A1 FA DF 73 B5 C6 7E 87

DATA RATE   : DR_0
RX RSSI     : -85
RX SNR      : 5


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    : 0/25 Fragments
              0/1000 Bytes
LOST        :    4608 Fragments


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    : 0/25 Fragments
              0/1000 Bytes
LOST        :    4608 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      264  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 18 00 26 5E 24 B9 7A 72 E2 87 68 45 12 6B 07
4A 80 F6 1C AC 7F C6 25 15 66 CB 31 C4 4A 37 04
66 61 A8 12 27 04 1D A6 67 52 8E

DATA RATE   : DR_0
RX RSSI     : -85
RX SNR      : 6


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    : 0/25 Fragments
              0/1000 Bytes
LOST        :    4608 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      265  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 19 00 FD D5 B2 EA 0A 12 79 59 EF 7D 8A E4 72
E0 41 46 54 8C 38 AB 24 18 DE 3C 62 92 86 13 DF
F3 CF 48 87 57 8D 00 00 00 00 00

DATA RATE   : DR_0
RX RSSI     : -84
RX SNR      : 6


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    : 0/25 Fragments
              0/1000 Bytes
LOST        :    4608 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      266  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 1A 00 A4 8A AA 1F 99 F7 6E 2F 40 67 14 E1 1B
DF DD FA EB 7F 42 05 60 2A F8 D6 12 D5 7E BA 12
54 D4 84 AA 43 2A 1D 35 48 1E DA

DATA RATE   : DR_0
RX RSSI     : -85
RX SNR      : 6


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    : 0/25 Fragments
              0/1000 Bytes
LOST        :    4608 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      267  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 1B 00 EF 05 7B FD 41 47 76 D2 4B 90 02 1B 77
12 39 2A 49 C9 5C AA 10 DE 51 F6 64 2D 55 F1 78
6A BA 8E 34 04 DA 17 8F F8 07 6C

DATA RATE   : DR_0
RX RSSI     : -85
RX SNR      : 6


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    : 0/25 Fragments
              0/1000 Bytes
LOST        :    4608 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      268  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 1C 00 B7 EF AE CE 87 0C E9 B7 F9 0B B1 F4 ED
83 E3 D3 98 D5 A1 67 04 86 DC 40 67 AE 83 AD 2C
4B E3 3B B6 56 9F D8 D5 13 29 A0

DATA RATE   : DR_0
RX RSSI     : -85
RX SNR      : 6


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    : 0/25 Fragments
              0/1000 Bytes
LOST        :    4608 Fragments


###### ========== MCPS-Indication ========== ######
STATUS      : OK

###### =====  DOWNLINK FRAME      269  ===== ######
RX WINDOW   : C Multicast
RX PORT     : 201
RX DATA     :
08 1D 00 5C FC 01 97 A1 16 C2 FC 66 A8 04 61 DE
BD FC 7E 69 A2 D9 C7 2C 61 E8 E4 D4 41 8A 82 12
70 2D 79 A5 DC C9 4F 9C 23 8F BF

DATA RATE   : DR_0
RX RSSI     : -85
RX SNR      : 6


###### =========== FRAG_DECODER ============ ######
######               PROGRESS                ######
###### ===================================== ######
RECEIVED    : 0/25 Fragments
              0/1000 Bytes
LOST        :    4608 Fragments



###### ===== Switch to Class A done.  ===== ######


###### =========== MCPS-Request ============ ######
######           MCPS_UNCONFIRMED            ######
###### ===================================== ######
STATUS      : OK

###### =========== MCPS-Confirm ============ ######
STATUS      : OK

###### =====   UPLINK FRAME       10   ===== ######

CLASS       : A

TX PORT     : 1
TX DATA     : UNCONFIRMED
14

DATA RATE   : DR_5
U/L FREQ    : 868100000
TX POWER    : 0
CHANNEL MASK: 0007

```