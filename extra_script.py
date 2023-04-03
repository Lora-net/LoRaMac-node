Import('env')
from os.path import join, realpath

# Includes

# Sources
#env.Append(SRC_FILTER=["+<*>","-<.git/>", "-<.svn/>"])
#env.Append(SRC_FILTER=["-<apps/>"])

def c_file(folder, name):
    return "+<"+folder+name+">"

# ==================
#      Boards
# ==================

#env.Append(SRC_FILTER=["-<boards/>"])
env.Append(CPPPATH=[realpath("src/boards")])


if env.get("PIOENV") == 'nucleo_l073rz':
    # Defines
    env.Append(CPPDEFINES=["USE_HAL_DRIVER", "STM32L073xx"])

    # Includes
    board_dir="boards/NucleoL073/"
    inc_dir="src/"+board_dir
    env.Append(CPPPATH=[realpath(inc_dir)])
    env.Append(CPPPATH=[realpath(inc_dir + "cmsis")])
    env.Append(CPPPATH=[realpath(inc_dir + "../mcu/stm32")])
    env.Append(CPPPATH=[realpath(inc_dir + "../mcu/stm32/cmsis")])
    env.Append(CPPPATH=[realpath(inc_dir + "../mcu/stm32/STM32L0xx_HAL_Driver/Inc")])

    # Files
    env.Append(SRC_FILTER=[c_file(board_dir,"adc-board.c")])
    env.Append(SRC_FILTER=[c_file(board_dir,"board.c")])
    env.Append(SRC_FILTER=[c_file(board_dir,"delay-board.c")])
    env.Append(SRC_FILTER=[c_file(board_dir,"eeprom-board.c")])
    env.Append(SRC_FILTER=[c_file(board_dir,"gpio-board.c")])
    env.Append(SRC_FILTER=[c_file(board_dir,"i2c-board.c")])
    env.Append(SRC_FILTER=[c_file(board_dir,"lpm-board.c")])
    env.Append(SRC_FILTER=[c_file(board_dir,"rtc-board.c")])
    env.Append(SRC_FILTER=[c_file(board_dir,"spi-board.c")])
    env.Append(SRC_FILTER=[c_file(board_dir,"sysIrqHandlers.c")])
    env.Append(SRC_FILTER=[c_file(board_dir,"uart-board.c")])
    env.Append(SRC_FILTER=[c_file(board_dir,"../mcu/utilities.c")])
    env.Append(SRC_FILTER=[c_file(board_dir,"cmsis/arm-gcc/startup_stm32l073xx.s")])
    env.Append(SRC_FILTER=[c_file(board_dir,"cmsis/system_stm32l0xx.c")])

    for item in env.get("CPPDEFINES", []):
        if item == "MCU_HAL_FROM_LIB":
            env.Append(SRC_FILTER=[c_file(board_dir,"../mcu/stm32/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal.c")])
            env.Append(SRC_FILTER=[c_file(board_dir,"../mcu/stm32/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_adc.c")])
            env.Append(SRC_FILTER=[c_file(board_dir,"../mcu/stm32/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_adc_ex.c")])
            env.Append(SRC_FILTER=[c_file(board_dir,"../mcu/stm32/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_cortex.c")])
            env.Append(SRC_FILTER=[c_file(board_dir,"../mcu/stm32/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_dma.c")])
            env.Append(SRC_FILTER=[c_file(board_dir,"../mcu/stm32/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_flash.c")])
            env.Append(SRC_FILTER=[c_file(board_dir,"../mcu/stm32/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_flash_ex.c")])
            env.Append(SRC_FILTER=[c_file(board_dir,"../mcu/stm32/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_gpio.c")])
            env.Append(SRC_FILTER=[c_file(board_dir,"../mcu/stm32/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_i2c.c")])
            env.Append(SRC_FILTER=[c_file(board_dir,"../mcu/stm32/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_i2c_ex.c")])
            env.Append(SRC_FILTER=[c_file(board_dir,"../mcu/stm32/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_pwr.c")])
            env.Append(SRC_FILTER=[c_file(board_dir,"../mcu/stm32/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_pwr_ex.c")])
            env.Append(SRC_FILTER=[c_file(board_dir,"../mcu/stm32/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_rcc.c")])
            env.Append(SRC_FILTER=[c_file(board_dir,"../mcu/stm32/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_rcc_ex.c")])
            env.Append(SRC_FILTER=[c_file(board_dir,"../mcu/stm32/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_rtc.c")])
            env.Append(SRC_FILTER=[c_file(board_dir,"../mcu/stm32/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_rtc_ex.c")])
            env.Append(SRC_FILTER=[c_file(board_dir,"../mcu/stm32/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_spi.c")])
            env.Append(SRC_FILTER=[c_file(board_dir,"../mcu/stm32/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_uart.c")])
            env.Append(SRC_FILTER=[c_file(board_dir,"../mcu/stm32/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_uart_ex.c")])

        if isinstance(item, tuple) and item[0] == "MBED_RADIO_SHIELD":
            env.Append(CPPDEFINES=[item[1]])
            if item[1] == "SX1272MB2DAS":
                env.Append(SRC_FILTER=[c_file(board_dir,"sx1272mb2das-board.c")])
                env.Append(CPPDEFINES=[("RADIO", "sx1272")])
            elif item[1] == "SX1276MB1LAS":
                env.Append(SRC_FILTER=[c_file(board_dir,"sx1276mb1las-board.c")])
                env.Append(CPPDEFINES=[("RADIO", "sx1276")])
            elif item[1] == "SX1276MB1MAS":
                env.Append(SRC_FILTER=[c_file(board_dir,"sx1276mb1mas-board.c")])
                env.Append(CPPDEFINES=[("RADIO", "sx1276")])
            elif item[1] == "SX1261MBXBAS":
                env.Append(SRC_FILTER=[c_file(board_dir,"sx1261mbxbas-board.c")])
                env.Append(CPPDEFINES=[("RADIO", "sx126x")])
            elif item[1] == "SX1262MBXCAS":
                env.Append(SRC_FILTER=[c_file(board_dir,"sx1262mbxcas-board.c")])
                env.Append(CPPDEFINES=[("RADIO", "sx126x")])
            elif item[1] == "SX1262MBXDAS":
                env.Append(SRC_FILTER=[c_file(board_dir,"sx1262mbxdas-board.c")])
                env.Append(CPPDEFINES=[("RADIO", "sx126x")])
            elif item[1] == "LR1110MB1XXS":
                env.Append(SRC_FILTER=[c_file(board_dir,"lr1110mb1xxs-board.c")])
                env.Append(CPPDEFINES=[("RADIO", "lr1110")])
            else:
                raise ValueError(" Please specify the MBED_RADIO_SHIELD !\nPossible values are: SX1272MB2DAS, SX1276MB1LAS, SX1276MB1MAS, SX1261MBXBAS, SX1262MBXCAS, SX1262MBXDAS and LR1110MB1XXS.")

# ==================
#       Mac
# ==================
#env.Append(SRC_FILTER=["-<mac/>"])
mac_dir="mac/"
mac_incdir="src/"+mac_dir

env.Append(CPPPATH=[realpath(mac_incdir)])
env.Append(CPPPATH=[realpath(mac_incdir + "region")])

env.Append(SRC_FILTER=[c_file(mac_dir,"region/RegionCommon.c")])
env.Append(SRC_FILTER=[c_file(mac_dir,"region/Region.c")])
env.Append(SRC_FILTER=[c_file(mac_dir,"LoRaMac.c")])
env.Append(SRC_FILTER=[c_file(mac_dir,"LoRaMacAdr.c")])
env.Append(SRC_FILTER=[c_file(mac_dir,"LoRaMacClassB.c")])
env.Append(SRC_FILTER=[c_file(mac_dir,"LoRaMacCommands.c")])
env.Append(SRC_FILTER=[c_file(mac_dir,"LoRaMacConfirmQueue.c")])
env.Append(SRC_FILTER=[c_file(mac_dir,"LoRaMacCrypto.c")])
env.Append(SRC_FILTER=[c_file(mac_dir,"LoRaMacParser.c")])
env.Append(SRC_FILTER=[c_file(mac_dir,"LoRaMacSerializer.c")])

for item in env.get("CPPDEFINES", []):

    # Region
    if item == "REGION_AS923":
        env.Append(SRC_FILTER=[c_file(mac_dir,"region/RegionAS923.c")])
    elif item == "REGION_AU915":
        env.Append(SRC_FILTER=[c_file(mac_dir,"region/RegionAU915.c")])
    elif item == "REGION_CN470":
        env.Append(SRC_FILTER=[c_file(mac_dir,"region/RegionCN470.c")])
        env.Append(SRC_FILTER=[c_file(mac_dir,"region/RegionCN470A20.c")])
        env.Append(SRC_FILTER=[c_file(mac_dir,"region/RegionCN470B20.c")])
        env.Append(SRC_FILTER=[c_file(mac_dir,"region/RegionCN470A26.c")])
        env.Append(SRC_FILTER=[c_file(mac_dir,"region/RegionCN470B26.c")])
    elif item == "REGION_CN779":
        env.Append(SRC_FILTER=[c_file(mac_dir,"region/RegionCN779.c")])
    elif item == "REGION_EU433":
        env.Append(SRC_FILTER=[c_file(mac_dir,"region/RegionEU433.c")])
    elif item == "REGION_EU868":
        env.Append(SRC_FILTER=[c_file(mac_dir,"region/RegionEU868.c")])
    elif item == "REGION_IN865":
        env.Append(SRC_FILTER=[c_file(mac_dir,"region/RegionIN865.c")])
    elif item == "REGION_KR920":
        env.Append(SRC_FILTER=[c_file(mac_dir,"region/RegionKR920.c")])
    elif item == "REGION_RU864":
        env.Append(SRC_FILTER=[c_file(mac_dir,"region/RegionRU864.c")])
    elif item == "REGION_US915":
        env.Append(SRC_FILTER=[c_file(mac_dir,"region/RegionBaseUS.c")])
        env.Append(SRC_FILTER=[c_file(mac_dir,"region/RegionUS915.c")])
        
    # AS923 channel plan
    env.Append(CPPDEFINES=[("REGION_AS923_DEFAULT_CHANNEL_PLAN", "CHANNEL_PLAN_GROUP_AS923_1")])
    if isinstance(item, tuple) and item[0] == "REGION_AS923_DEFAULT_CHANNEL_PLAN":
        env.Replace(CPPDEFINES=[("REGION_AS923_DEFAULT_CHANNEL_PLAN", "CHANNEL_PLAN_GROUP_AS923_"+item[1])])

    # CN470 channel plan
    env.Append(CPPDEFINES=[("REGION_CN470_DEFAULT_CHANNEL_PLAN", "CHANNEL_PLAN_GROUP_CN470_1")])
    if isinstance(item, tuple) and item[0] == "REGION_CN470_DEFAULT_CHANNEL_PLAN":
        env.Replace(CPPDEFINES=[("REGION_CN470_DEFAULT_CHANNEL_PLAN", "CHANNEL_PLAN_GROUP_CN470_"+item[1])])

# ==================
#    Peripherals
# ==================

#env.Append(SRC_FILTER=["-<peripherals/>"])
env.Append(CPPPATH=[realpath("src/peripherals")])

periph_dir="peripherals/"

for item in env.get("CPPDEFINES", []):

    # Secure Element
    if isinstance(item, tuple) and item[0] == "SECURE_ELEMENT":
        if item[1] == "SOFT_SE":
            env.Append(CPPDEFINES=["SOFT_SE"])
            env.Append(SRC_FILTER=[c_file(periph_dir,"*.c")])
            env.Append(SRC_FILTER=[c_file(periph_dir,"soft-se/*.c")])
            env.Append(CPPPATH=[realpath("src/peripherals/soft-se")])
        elif item[1] == "LR1110_SE":
            for itemx in env.get("CPPDEFINES", []):
                if isinstance(itemx, tuple) and itemx[0] == "RADIO":
                    if itemx[1] == "lr1110":
                        env.Append(SRC_FILTER=[c_file(periph_dir,"*.c")])
                        env.Append(SRC_FILTER=[c_file(periph_dir,"lr1110-se/*.c")])
                        env.Append(CPPPATH=[realpath("src/peripherals/lr1110-se")])
                    else: 
                        raise ValueError("LR1110_SE secure elemeent can only be used when LR1110 radio is selected.")
        elif item[1] == "ATECC608A_TNGLORA_SE":
            env.Append(SRC_FILTER=[c_file(periph_dir,"*.c")])
            env.Append(SRC_FILTER=[c_file(periph_dir,"atecc608a-tnglora-se/*.c")])
            env.Append(SRC_FILTER=[c_file(periph_dir,"atecc608a-tnglora-se/cryptoauthlib/lib/*.c")])
            env.Append(SRC_FILTER=[c_file(periph_dir,"atecc608a-tnglora-se/cryptoauthlib/lib/basic/*.c")])
            env.Append(SRC_FILTER=[c_file(periph_dir,"atecc608a-tnglora-se/cryptoauthlib/lib/crypto/*.c")])
            env.Append(SRC_FILTER=[c_file(periph_dir,"atecc608a-tnglora-se/cryptoauthlib/lib/crypto/hashes/*.c")])
            env.Append(SRC_FILTER=[c_file(periph_dir,"atecc608a-tnglora-se/cryptoauthlib/lib/hal/atca_hal.c")])
            env.Append(SRC_FILTER=[c_file(periph_dir,"atecc608a-tnglora-se/cryptoauthlib/lib/host/*.c")])

            env.Append(CCPPATH=[realpath("src/peripherals/atecc608a-tnglora-se")])
            env.Append(CCPPATH=[realpath("src/peripherals/atecc608a-tnglora-se/cryptoauthlib/lib")])
            env.Append(CCPPATH=[realpath("src/peripherals/atecc608a-tnglora-se/cryptoauthlib/lib/basic")])
            env.Append(CCPPATH=[realpath("src/peripherals/atecc608a-tnglora-se/cryptoauthlib/lib/crypto")])
            env.Append(CCPPATH=[realpath("src/peripherals/atecc608a-tnglora-se/cryptoauthlib/lib/crypto/hashes")])
            env.Append(CCPPATH=[realpath("src/peripherals/atecc608a-tnglora-se/cryptoauthlib/lib/hal")])
            env.Append(CCPPATH=[realpath("src/peripherals/atecc608a-tnglora-se/cryptoauthlib/lib/host")])
        else:
            raise ValueError("No secure-element selected.")
        
# ==================
#       Radio
# ==================
#env.Append(SRC_FILTER=["-<radio/>"])
env.Append(CPPPATH=[realpath("src/radio")])
radio_dir = "radio/"

for item in env.get("CPPDEFINES", []):
    if isinstance(item, tuple) and item[0] == "RADIO":
        if item[1] == "lr1110":
            env.Append(SRC_FILTER=[c_file(radio_dir,"lr1110/radio.c")])
            env.Append(SRC_FILTER=[c_file(radio_dir,"lr1110/lr1110_driver/src/lr1110_bootloader.c")])
            env.Append(SRC_FILTER=[c_file(radio_dir,"lr1110/lr1110_driver/src/lr1110_crypto_engine.c")])
            env.Append(SRC_FILTER=[c_file(radio_dir,"lr1110/lr1110_driver/src/lr1110_driver_version.c")])
            env.Append(SRC_FILTER=[c_file(radio_dir,"lr1110/lr1110_driver/src/lr1110_gnss.c")])
            env.Append(SRC_FILTER=[c_file(radio_dir,"lr1110/lr1110_driver/src/lr1110_radio.c")])
            env.Append(SRC_FILTER=[c_file(radio_dir,"lr1110/lr1110_driver/src/lr1110_regmem.c")])
            env.Append(SRC_FILTER=[c_file(radio_dir,"lr1110/lr1110_driver/src/lr1110_system.c")])
            env.Append(SRC_FILTER=[c_file(radio_dir,"lr1110/lr1110_driver/src/lr1110_wifi.c")])

            env.Append(CCPPATH=[realpath("src/radio/lr1110")])
            env.Append(CCPPATH=[realpath("src/radio/lr1110/lr1110_driver/src")])
        elif item[1] == "sx126x":
            env.Append(SRC_FILTER=[c_file(radio_dir,"sx126x/radio.c")])
            env.Append(SRC_FILTER=[c_file(radio_dir,"sx126x/sx126x.c")])

            env.Append(CCPPATH=[realpath("src/radio/sx126x")])
        elif item[1] == "sx1272":
            env.Append(SRC_FILTER=[c_file(radio_dir,"sx1272/sx1272.c")])

            env.Append(CCPPATH=[realpath("src/radio/sx1272")])
        elif item[1] == "sx1276":
            env.Append(SRC_FILTER=[c_file(radio_dir,"sx1276/sx1276.c")])

            env.Append(CCPPATH=[realpath("src/radio/sx1276")])
        else:
            raise ValueError("Unsupported radio driver selected...")

# ==================
#      System
# ==================
env.Append(CPPPATH=[realpath("src/system")])
env.Append(SRC_FILTER=[c_file("system/","*.c")])

# ==================
#    LoRaMac App
# ==================
loramac_dir = "apps/LoRaMac/"
for item in env.get("CPPDEFINES", []):
    if item == "LORAMAC":
        env.Append(SRC_FILTER=[c_file(loramac_dir,"common/CayenneLpp.c")])
        env.Append(SRC_FILTER=[c_file(loramac_dir,"common/cli.c")])
        env.Append(SRC_FILTER=[c_file(loramac_dir,"common/LmHandlerMsgDisplay.c")])
        env.Append(SRC_FILTER=[c_file(loramac_dir,"common/NvmDataMgmt.c")])
        env.Append(SRC_FILTER=[c_file(loramac_dir,"common/LmHandler/LmHandler.c")])
        env.Append(SRC_FILTER=[c_file(loramac_dir,"common/LmHandler/packages/FragDecoder.c")])
        env.Append(SRC_FILTER=[c_file(loramac_dir,"common/LmHandler/packages/LmhpClockSync.c")])
        env.Append(SRC_FILTER=[c_file(loramac_dir,"common/LmHandler/packages/LmhpCompliance.c")])
        env.Append(SRC_FILTER=[c_file(loramac_dir,"common/LmHandler/packages/LmhpFragmentation.c")])
        env.Append(SRC_FILTER=[c_file(loramac_dir,"common/LmHandler/packages/LmhpRemoteMcastSetup.c")])

        env.Append(CPPPATH=[realpath("src/apps/LoRaMac/common")])
        env.Append(CPPPATH=[realpath("src/apps/LoRaMac/common/LmHandler")])
        env.Append(CPPPATH=[realpath("src/apps/LoRaMac/common/LmHandler/packages")])

# Dump global construction environment (for debug purpose)
print(env.Dump())
