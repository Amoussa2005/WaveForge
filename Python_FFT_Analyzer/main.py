
import time
import traceback
import serial
import matplotlib.pyplot as plt

from config import (
    SERIAL_PORT,
    FFT_SIZE,
    CHANNEL_NAMES
)

from serial_communication import SerialCommunication

from signal_processing import (
    convert_to_voltage,
    remove_dc_offset,
    calculate_signal_measurements
)

from fft_analysis import FFTAnalysis
from thd_calculation import THDCalculation
from dashboard import WaveForgeDashboard


# =========================================================
#                 SIGNAL PROCESSING ADAPTER
# =========================================================

class SignalProcessorAdapter:

    def __init__(self):

        self.raw_samples = None
        self.voltage_samples = None
        self.ac_samples = None

        self.dc_offset = 0.0
        self.measurements = {}

    def process(self, raw_samples):

        self.raw_samples = raw_samples

        self.voltage_samples = convert_to_voltage(
            raw_samples
        )

        self.ac_samples, self.dc_offset = remove_dc_offset(
            self.voltage_samples
        )

        self.measurements = calculate_signal_measurements(
            self.voltage_samples
        )

    def get_raw_samples(self):
        return self.raw_samples

    def get_voltage_samples(self):
        return self.voltage_samples

    def get_ac_samples(self):
        return self.ac_samples

    def get_dc_offset(self):
        return self.measurements["dc_offset"]

    def get_minimum(self):
        return self.measurements["minimum"]

    def get_maximum(self):
        return self.measurements["maximum"]

    def get_peak_to_peak(self):
        return self.measurements["peak_to_peak"]

    def get_rms(self):
        return self.measurements["rms"]


# =========================================================
#                  PRINT MEASUREMENTS
# =========================================================

def print_measurements(
    channel,
    signal_processor,
    fft_analyzer,
    thd_analyzer
):

    channel_name = CHANNEL_NAMES.get(
        channel,
        "UNKNOWN"
    )

    print("--------------------------------------")
    print(f"Channel:       {channel_name}")
    print(f"Samples:       {FFT_SIZE}")

    print(
        f"DC Offset:     "
        f"{signal_processor.get_dc_offset():.3f} V"
    )

    print(
        f"Minimum:       "
        f"{signal_processor.get_minimum():.3f} V"
    )

    print(
        f"Maximum:       "
        f"{signal_processor.get_maximum():.3f} V"
    )

    print(
        f"Peak-to-Peak:  "
        f"{signal_processor.get_peak_to_peak():.3f} V"
    )

    print(
        f"RMS Voltage:   "
        f"{signal_processor.get_rms():.3f} V"
    )

    print(
        f"Frequency:     "
        f"{fft_analyzer.get_fundamental_frequency():.2f} Hz"
    )

    if thd_analyzer.is_valid():

        print(
            f"THD:           "
            f"{thd_analyzer.get_thd():.3f} %"
        )

    else:
        print("THD:           Invalid")

    print("--------------------------------------")


# =========================================================
#                       MAIN PROGRAM
# =========================================================

def main():

    print("======================================")
    print("       WAVEFORGE FFT ANALYZER")
    print("======================================")

    serial_comm = SerialCommunication()

    signal_processor = SignalProcessorAdapter()

    fft_analyzer = FFTAnalysis()

    thd_analyzer = THDCalculation()

    dashboard = None

    try:

        # -------------------------------------------------
        # SERIAL CONNECTION
        # -------------------------------------------------

        print(f"\nConnecting to {SERIAL_PORT}...")

        serial_comm.connect()

        if (
            serial_comm.connection is None
            or not serial_comm.connection.is_open
        ):

            raise ConnectionError(
                "Unable to establish serial connection."
            )

        print("Serial connection established.")

        # -------------------------------------------------
        # DASHBOARD INITIALIZATION
        # -------------------------------------------------

        print("\nInitializing dashboard...")

        dashboard = WaveForgeDashboard()

        plt.pause(0.1)

        print("Dashboard initialized.")

        print("\nWaiting for waveform samples...\n")

        # -------------------------------------------------
        # CONTINUOUS PROCESSING
        # -------------------------------------------------

        while True:

            # Check whether dashboard is still open
            if not plt.fignum_exists(dashboard.fig.number):

                print("Dashboard window closed.")
                break

            # Receive ESP32 packet
            try:

                packet = serial_comm.receive_packet(
                    timeout=3
                )

            except TimeoutError:

                plt.pause(0.01)
                continue

            except ValueError as error:

                print(f"Packet error: {error}")
                continue

            if packet is None:

                plt.pause(0.01)
                continue

            channel, raw_samples = packet

            # -------------------------------------------------
            # VALIDATE PACKET
            # -------------------------------------------------

            if len(raw_samples) != FFT_SIZE:

                print(
                    f"Invalid sample count: {len(raw_samples)}"
                )

                continue

            if channel not in CHANNEL_NAMES:

                print(f"Invalid channel ID: {channel}")
                continue

            # -------------------------------------------------
            # SIGNAL PROCESSING
            # -------------------------------------------------

            signal_processor.process(raw_samples)

            ac_samples = signal_processor.get_ac_samples()

            # -------------------------------------------------
            # FFT ANALYSIS
            # -------------------------------------------------

            frequencies, amplitudes = fft_analyzer.perform_fft(
                ac_samples
            )

            # -------------------------------------------------
            # THD CALCULATION
            # -------------------------------------------------

            thd_analyzer.calculate(fft_analyzer)

            # -------------------------------------------------
            # DASHBOARD UPDATE
            # -------------------------------------------------

            dashboard.update(
                channel,
                signal_processor,
                fft_analyzer,
                thd_analyzer
            )

            # -------------------------------------------------
            # TERMINAL OUTPUT
            # -------------------------------------------------

            print_measurements(
                channel,
                signal_processor,
                fft_analyzer,
                thd_analyzer
            )

    except KeyboardInterrupt:

        print("\nAnalyzer stopped by user.")

    except serial.SerialException as error:

        print(f"\nSerial communication error: {error}")

    except Exception as error:

        print(f"\nUnexpected error: {error}")

        traceback.print_exc()

    finally:

        print("\nClosing WaveForge...")

        try:
            serial_comm.disconnect()
        except Exception as error:
            print(f"Serial cleanup error: {error}")

        if dashboard is not None:

            try:
                dashboard.close()
            except Exception as error:
                print(f"Dashboard cleanup error: {error}")

        print("WaveForge analyzer stopped.")


# =========================================================
#                       ENTRY POINT
# =========================================================

if __name__ == "__main__":

    main()
