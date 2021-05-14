import asyncio
import datetime
from bleak import BleakClient, BleakScanner


OTA_PACKET_UUID = '23408888-1F40-4CD8-9B89-CA8D45F8A5B0'
OTA_CONTROL_UUID = '7AD671AA-21C0-46A4-B722-270E3AE3D830'

SVR_CHR_OTA_CONTROL_NOP = bytearray.fromhex("00")
SVR_CHR_OTA_CONTROL_REQUEST = bytearray.fromhex("01")
SVR_CHR_OTA_CONTROL_REQUEST_ACK = bytearray.fromhex("02")
SVR_CHR_OTA_CONTROL_REQUEST_NAK = bytearray.fromhex("03")
SVR_CHR_OTA_CONTROL_DONE = bytearray.fromhex("04")
SVR_CHR_OTA_CONTROL_DONE_ACK = bytearray.fromhex("05")
SVR_CHR_OTA_CONTROL_DONE_NAK = bytearray.fromhex("06")


async def _search_for_esp32():
    print("Searching for ESP32...")
    esp32 = None

    devices = await BleakScanner.discover()
    for device in devices:
        print(device.name)
        if device.name == "esp32":
            esp32 = device

    if esp32 is not None:
        print("ESP32 found!")
    else:
        print("ESP32 has not been found.")
        assert esp32 is not None

    return esp32


async def _send_ota():
    t0 = datetime.datetime.now()
    queue = asyncio.Queue()
    firmware = []

    with open("esp32_ble_ota.bin", "rb") as file:
        while junk := file.read(200):
            firmware.append(junk)
    firmware.reverse()
    sum_pkgs = len(firmware)
    num_sent_pkgs = 0

    esp32 = await _search_for_esp32()
    async with BleakClient(esp32) as client:

        async def _send_pkg(pkg_to_sent):
            print(f"Sending pkg {num_sent_pkgs}/{sum_pkgs}.")
            await client.write_gatt_char(
                OTA_PACKET_UUID,
                pkg_to_sent,
            )

        async def _ota_notification_handler(sender: int, data: bytearray):
            if data == SVR_CHR_OTA_CONTROL_REQUEST_ACK:
                print("ESP32: OTA request acknowledged.")
                await _send_pkg(firmware.pop())
                await queue.put("ack")
            elif data == SVR_CHR_OTA_CONTROL_REQUEST_NAK:
                print("ESP32: OTA request NOT acknowledged.")
                await queue.put("nak")
                await client.stop_notify(OTA_CONTROL_UUID)
            elif data == SVR_CHR_OTA_CONTROL_DONE_ACK:
                print("ESP32: OTA done acknowledged.")
                await queue.put("ack")
                await client.stop_notify(OTA_CONTROL_UUID)
            elif data == SVR_CHR_OTA_CONTROL_DONE_NAK:
                print("ESP32: OTA done NOT acknowledged.")
                await queue.put("nak")
                await client.stop_notify(OTA_CONTROL_UUID)
            else:
                print(f"Notification received: sender: {sender}, data: {data}")

        await client.start_notify(
            OTA_CONTROL_UUID,
            _ota_notification_handler
        )

        print("Sending OTA request.")
        await client.write_gatt_char(
            OTA_CONTROL_UUID,
            SVR_CHR_OTA_CONTROL_REQUEST
        )
        await asyncio.sleep(1)

        if await queue.get() == "ack":
            for pkg in firmware:
                num_sent_pkgs += 1
                await _send_pkg(pkg)

            print("Sending OTA done.")
            await client.write_gatt_char(
                OTA_CONTROL_UUID,
                SVR_CHR_OTA_CONTROL_DONE
            )
            await asyncio.sleep(1)

            if await queue.get() == "ack":
                dt = datetime.datetime.now() - t0
                print(f"OTA successful! Total time: {dt}")
            else:
                print("OTA failed.")

        else:
            print("ESP32 did not acknowledge the OTA request.")


def main():
    asyncio.run(_send_ota())


if __name__ == '__main__':
    main()

