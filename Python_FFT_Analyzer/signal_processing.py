
import numpy as np


# =====================================================
#              VOLTAGE CONVERSION
# =====================================================

def convert_to_voltage(raw_samples, channel=None):
    """
    Convert calibrated millivolt samples received from
    the ESP32 into voltage values.

    The ESP32 has already applied the square-wave
    voltage-divider correction.

    Parameters
    ----------
    raw_samples : array-like
        uint16 samples received from the ESP32, in mV.

    channel : int, optional
        Channel identifier:
        0 = Square
        1 = Triangle
        2 = Sine

        Kept for compatibility with the existing project.
        No additional channel gain is applied.

    Returns
    -------
    numpy.ndarray
        Voltage samples in volts.
    """

    samples = np.asarray(raw_samples, dtype=np.float64)

    # Convert millivolts to volts
    voltage_samples = samples / 1000.0

    return voltage_samples


# =====================================================
#                 DC OFFSET REMOVAL
# =====================================================

def remove_dc_offset(voltage_samples):
    """
    Remove the average DC component from a signal.

    Returns
    -------
    ac_samples : numpy.ndarray
        Signal with its DC offset removed.

    dc_offset : float
        Original average voltage.
    """

    samples = np.asarray(voltage_samples, dtype=np.float64)

    if samples.size == 0:
        raise ValueError("The input signal is empty.")

    dc_offset = np.mean(samples)

    ac_samples = samples - dc_offset

    return ac_samples, dc_offset


# =====================================================
#                SIGNAL MEASUREMENTS
# =====================================================

def calculate_signal_measurements(voltage_samples):
    """
    Calculate basic voltage measurements.

    Returns
    -------
    dict
        DC offset, minimum, maximum, peak-to-peak,
        and RMS voltage.
    """

    samples = np.asarray(voltage_samples, dtype=np.float64)

    if samples.size == 0:
        raise ValueError("The input signal is empty.")

    dc_offset = np.mean(samples)

    minimum = np.min(samples)
    maximum = np.max(samples)

    peak_to_peak = maximum - minimum

    ac_samples = samples - dc_offset

    rms_voltage = np.sqrt(np.mean(ac_samples ** 2))

    return {
        "dc_offset": float(dc_offset),
        "minimum": float(minimum),
        "maximum": float(maximum),
        "peak_to_peak": float(peak_to_peak),
        "rms": float(rms_voltage)
    }
