# This program generates the byte instructions to send to the tracker to poll specific
# time ranges
# Set the two date_time_str to the time range you want to poll.
# Send the output to downlink port 18 to get a response
#
# By Medad Newman, Imperial College Space Society
# 18/12/2020

import struct
import time
from datetime import datetime


epoch_unix = 1577840461


def convert_datatime_to_special_epoch_min(ts: datetime):
    return int((ts.timestamp() - epoch_unix)/60)


def gen_instruction(older, newer):

    if older >= newer:
        raise ValueError("Older is equal or greater than newer")
    newer = struct.pack('<I', newer)
    older = struct.pack('<I', older)

    return newer.hex()+older.hex()


def unix_to_datetime_str(unix):
    return datetime.utcfromtimestamp(unix).strftime('%Y-%m-%d %H:%M:%S')


def convert_special_epoch_min_to_datetime(ts_min: int) -> str:
    unix = ts_min*60+epoch_unix

    return datetime.utcfromtimestamp(unix).strftime('%Y-%m-%d %H:%M:%S')


if __name__ == "__main__":
    date_time_str = '2021-10-15 10:30:01'
    date_time_older = datetime.strptime(date_time_str, '%Y-%m-%d %H:%M:%S')

    date_time_str = '2021-10-15 11:55:01'
    date_time_newer = datetime.strptime(date_time_str, '%Y-%m-%d %H:%M:%S')

    print(gen_instruction(convert_datatime_to_special_epoch_min(date_time_older),
                          convert_datatime_to_special_epoch_min(date_time_newer)))

    print(convert_special_epoch_min_to_datetime(811758))
    print(convert_special_epoch_min_to_datetime(811718))
