
# cmds:
#
# ttn-lw-cli end-devices create --application-id "icss-lora-tracker"  --device-id "testing-budgie1"  --frequency-plan-id EU_863_870 --lorawan-version 1.0.2 --lorawan-phy-version 1.0.2-b --abp --with-session --mac-settings.resets-f-cnt -c ttn-lw-cli.yml
# ttn-lw-cli end-devices list-frequency-plans -c ttn-lw-cli.yml
# ttn-lw-cli end-devices create --application-id "icss-lora-tracker"  --device-id "testing-budgiedsa"  --frequency-plan-id AS_920_923_TTN_AU --lorawan-version 1.0.2 --lorawan-phy-version 1.0.2-b --abp --with-session --mac-settings.resets-f-cnt -c ttn-lw-cli.yml
# ttn-lw-cli end-devices list --application-id "icss-lora-tracker" --all  -c ttn-lw-cli.yml
#

# NOTE: Must manually change the Device ID field on the things stack website. Very important!!


import time
from datetime import datetime
import secrets


frequency_plan_strings = ["EU_863_870_TTN",
                          "US_902_928_FSB_2",
                          "AU_915_928_FSB_2",
                          "AS_923",
                          "KR_920_923_TTN",
                          "IN_865_867",
                          "RU_864_870_TTN",
                          "CN_470_510_FSB_11"
                          ]

#device_id = "test-09-06-2021"


registered_devices = [
    "icspace26-hab-as-923",
    "icspace26-hab-au-915-928-fsb-2",
    "icspace26-hab-cn-470-510-fsb-11",
    "icspace26-hab-eu-863-870",
    "icspace26-hab-in-865-867",
    "icspace26-hab-kr-920-923-ttn",
    "icspace26-hab-ru-864-870-ttn",
    "icspace26-hab-us-902-928-fsb-2",
]


device_id = datetime.today().strftime('%Y-%m-%d') + "-" + secrets.token_hex(2)


def generate_commands(frequency_plan_string):
    dev_id_for_plan = device_id + "-" + \
        frequency_plan_string.replace("_", "-").lower()
    cmd = 'ttn-lw-cli end-devices create --application-id "icss-lora-tracker" ' \
          ' --device-id "{0}" ' \
          ' --frequency-plan-id {1}' \
          ' --lorawan-version 1.0.3 ' \
          '--lorawan-phy-version 1.0.3-a ' \
          '--abp ' \
          '--with-session' \
          ' --mac-settings.resets-f-cnt ' \
          ' --mac-settings.supports-32-bit-f-cnt true '\
          ' --mac-settings.status-count-periodicity 10000000'\
          ' --mac-settings.status-time-periodicity 20000h2m3s'\
          ' -c ttn-lw-cli.yml'.format(dev_id_for_plan, frequency_plan_string)

    return cmd


def generate_add_fcount_32bit_option(device_id):
    cmd = f'ttn-lw-cli end-devices set --application-id "icss-lora-tracker" --device-id "{device_id}" --mac-settings.supports-32-bit-f-cnt true -c ttn-lw-cli.yml'
    print(cmd)
    return cmd

def generate_add_status_count_periodicity_option(device_id):
    cmd = f'ttn-lw-cli end-devices set --application-id "icss-lora-tracker" --device-id "{device_id}" --mac-settings.status-count-periodicity 10000000 -c ttn-lw-cli.yml'
    print(cmd)
    return cmd

def generate_update_commands():
    for i in registered_devices:
        #generate_add_fcount_32bit_option(i)
        generate_add_status_count_periodicity_option(i)



def generate_code():
    for frequency_plan_string in frequency_plan_strings:
        cmd = generate_commands(frequency_plan_string)
        print(cmd)


if __name__ == "__main__":
    generate_code()
    generate_update_commands()

