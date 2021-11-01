/*
 * Created Date: Wednesday, July 7th 2021, 11:43:21 pm
 * Author: Medad Newman
 * This is the payload formatter used in the Helium Network to decode the uplink into human readable fields
 */


function Decoder(bytes, port) {

    "use strict";
    switch (port) {
        case 99:
            return {
                data: {
                    longitude: readInt16LE(bytes.slice(7, 9)) * 0xffff / 1e7,
                    latitude: readInt16LE(bytes.slice(5, 7)) * 0xffff / 1e7,
                    altitude: readUInt16LE(bytes.slice(9, 11)),
                    sats: bytes[3] >> 3 & 0x1F,
                    pressure: ((bytes[2] >> 1) & 0x7F) * 10,
                    app_ack: bytes[2] & 0x01,
                    reset_cnt: bytes[3] & 0x7,
                    boardTemp: readInt16LE(bytes.slice(4, 5)),
                    noloadVoltage: ((bytes[0] >> 3) & 0x1F) + 18,
                    loadVoltage: (((bytes[0] << 2) & 0x1C) | ((bytes[1] >> 6) & 0x3)) + 18,
                    days_of_playback: bytes[1] & 0x3F,
                }
            };
        default:
            return {
                errors: ['Unknown FPort - see device manual!'],
            };
    }
}

/* ******************************************
* bytes to number
********************************************/
function readUInt8LE(bytes) {
    return (bytes & 0xFF);
}

function readInt8LE(bytes) {
    var ref = readUInt8LE(bytes);
    return (ref > 0x7F) ? ref - 0x100 : ref;
}

function readUInt16LE(bytes) {
    var value = (bytes[1] << 8) + bytes[0];
    return (value & 0xFFFF);
}

function readInt16LE(bytes) {
    var ref = readUInt16LE(bytes);
    return (ref > 0x7FFF) ? ref - 0x10000 : ref;
}

function readUInt32LE(bytes) {
    var value = (bytes[3] << 24) + (bytes[2] << 16) + (bytes[1] << 8) + bytes[0];
    return (value & 0xFFFFFFFF);
}

function readInt32LE(bytes) {
    var ref = readUInt32LE(bytes);
    return (ref > 0x7FFFFFFF) ? ref - 0x100000000 : ref;
}

function readFloatLE(bytes) {
    // JavaScript bitwise operators yield a 32 bits integer, not a float.
    // Assume LSB (least significant byte first).
    var bits = bytes[3] << 24 | bytes[2] << 16 | bytes[1] << 8 | bytes[0];
    var sign = (bits >>> 31 === 0) ? 1.0 : -1.0;
    var e = bits >>> 23 & 0xff;
    var m = (e === 0) ? (bits & 0x7fffff) << 1 : (bits & 0x7fffff) | 0x800000;
    var f = sign * m * Math.pow(2, e - 150);
    return f;
}
