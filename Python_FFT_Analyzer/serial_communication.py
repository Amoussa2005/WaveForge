
import serial
import time
import numpy as np

from config import (
    SERIAL_PORT,
    BAUD_RATE,
    SERIAL_TIMEOUT,
    FFT_SIZE
)


class SerialCommunication:

    HEADER = b"WF"

    def __init__(self):

        self.connection = None

    # =================================================
    #                  CONNECTION
    # =================================================

    def connect(self):

        self.connection = serial.Serial(
            port=SERIAL_PORT,
            baudrate=BAUD_RATE,
            timeout=0.1
        )

        # Allow the serial connection to stabilize
        time.sleep(2)

        # Discard any old incoming data
        self.connection.reset_input_buffer()

        print("================================")
        print("WAVEFORGE SERIAL CONNECTION")
        print("================================")
        print(f"Port: {SERIAL_PORT}")
        print(f"Baud rate: {BAUD_RATE}")
        print("Connection established.")

    def disconnect(self):

        if self.connection and self.connection.is_open:
            self.connection.close()

            print("Serial connection closed.")

    # =================================================
    #                  READ EXACT BYTES
    # =================================================

    def read_exact(self, number_of_bytes, deadline):

        data = bytearray()

        while len(data) < number_of_bytes:

            if time.monotonic() >= deadline:
                raise TimeoutError("Serial packet timeout.")

            remaining = number_of_bytes - len(data)

            chunk = self.connection.read(remaining)

            if chunk:
                data.extend(chunk)

        return bytes(data)

    # =================================================
    #                  FIND PACKET HEADER
    # =================================================

    def find_header(self, timeout):

        deadline = time.monotonic() + timeout

        previous_byte = None

        while time.monotonic() < deadline:

            current_byte = self.connection.read(1)

            if not current_byte:
                continue

            if previous_byte == ord('W') and current_byte == b'F':
                return True

            previous_byte = current_byte[0]

        return False

    # =================================================
    #                  CRC16 CHECK
    # =================================================

    @staticmethod
    def calculate_crc16(data):

        crc = 0xFFFF

        for byte in data:

            crc ^= byte << 8

            for _ in range(8):

                if crc & 0x8000:
                    crc = ((crc << 1) ^ 0x1021) & 0xFFFF

                else:
                    crc = (crc << 1) & 0xFFFF

        return crc

    # =================================================
    #                  RECEIVE PACKET
    # =================================================

    def receive_packet(self, timeout=3):

        if self.connection is None or not self.connection.is_open:
            raise ConnectionError("ESP32 is not connected.")

        # Find packet header
        if not self.find_header(timeout):
            return None

        deadline = time.monotonic() + timeout

        # Read channel and sample count
        metadata = self.read_exact(3, deadline)

        channel = metadata[0]

        sample_count = int.from_bytes(
            metadata[1:3],
            byteorder="little"
        )

        if channel not in (0, 1, 2):
            raise ValueError("Invalid channel identifier.")

        if sample_count != FFT_SIZE:
            raise ValueError(
                f"Invalid sample count: {sample_count}"
            )

        # Read ADC data and CRC
        payload = self.read_exact(
            sample_count * 2,
            deadline
        )

        received_crc_bytes = self.read_exact(2, deadline)

        received_crc = int.from_bytes(
            received_crc_bytes,
            byteorder="little"
        )

        # Verify packet integrity
        calculated_crc = self.calculate_crc16(
            metadata + payload
        )

        if received_crc != calculated_crc:
            raise ValueError("CRC error: corrupted packet.")

        # Convert binary data into NumPy array
        samples = np.frombuffer(
            payload,
            dtype="<u2"
        ).copy()

        return channel, samples
