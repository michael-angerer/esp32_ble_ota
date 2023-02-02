import asyncio
import datetime
from bleak import BleakClient, BleakScanner


OTA_DATA_UUID = '23408888-1F40-4CD8-9B89-CA8D45F8A5B0'
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
        if device.name == "esp32":
            esp32 = device

    if esp32 is not None:
        print("ESP32 found!")
    else:
        print("ESP32 has not been found.")
        assert esp32 is not None

    return esp32


async def send_ota(file_path):
    t0 = datetime.datetime.now()
    queue = asyncio.Queue()
    firmware = []

    esp32 = await _search_for_esp32()
    async with BleakClient(esp32) as client:

        async def _ota_notification_handler(sender: int, data: bytearray):
            if data == SVR_CHR_OTA_CONTROL_REQUEST_ACK:
                print("ESP32: OTA request acknowledged.")
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

        # subscribe to OTA control
        await client.start_notify(
            OTA_CONTROL_UUID,
            _ota_notification_handler
        )

        # compute the packet size
        packet_size = (client.mtu_size - 3)

        # write the packet size to OTA Data
        print(f"Sending packet size: {packet_size}.")
        await client.write_gatt_char(
            OTA_DATA_UUID,
            packet_size.to_bytes(2, 'little'),
            response=True
        )

        # split the firmware into packets
        with open(file_path, "rb") as file:
            while chunk := file.read(packet_size):
                firmware.append(chunk)

        # write the request OP code to OTA Control
        print("Sending OTA request.")
        await client.write_gatt_char(
            OTA_CONTROL_UUID,
            SVR_CHR_OTA_CONTROL_REQUEST
        )

        # wait for the response
        await asyncio.sleep(1)
        if await queue.get() == "ack":

            # sequentially write all packets to OTA data
            for i, pkg in enumerate(firmware):
                print(f"Sending packet {i+1}/{len(firmware)}.")
                # if we are writing a bunch of data, set response to false and havea worker mechanisim
                # client responds periodically to a service with details on how much data has been received :)
                # this coulld also speed up OTA consideraby 
                await client.write_gatt_char(
                    OTA_DATA_UUID,
                    pkg,
                    response=True
                )

            # write done OP code to OTA Control
            print("Sending OTA done.")
            await client.write_gatt_char(
                OTA_CONTROL_UUID,
                SVR_CHR_OTA_CONTROL_DONE
            )

            # wait for the response
            await asyncio.sleep(1)
            if await queue.get() == "ack":
                dt = datetime.datetime.now() - t0
                print(f"OTA successful! Total time: {dt}")
            else:
                print("OTA failed.")

        else:
            print("ESP32 did not acknowledge the OTA request.")


if __name__ == '__main__':
    asyncio.run(send_ota("esp.bin"))
