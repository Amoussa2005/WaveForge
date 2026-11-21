
import numpy as np

from config import (
    FFT_SIZE,
    SAMPLING_FREQUENCY,
    HARMONIC_COUNT
)


class THDCalculation:

    def __init__(self):

        self.fundamental_amplitude = 0.0
        self.fundamental_frequency = 0.0

        self.harmonic_amplitudes = []
        self.harmonic_frequencies = []

        self.thd_percentage = 0.0
        self.valid = False

    # =================================================
    #          INTERPOLATED PEAK AMPLITUDE
    # =================================================

    def interpolate_peak(self, amplitudes, peak_bin):

        if peak_bin <= 0 or peak_bin >= len(amplitudes) - 1:
            return float(amplitudes[peak_bin])

        left = np.log(max(amplitudes[peak_bin - 1], 1e-12))
        center = np.log(max(amplitudes[peak_bin], 1e-12))
        right = np.log(max(amplitudes[peak_bin + 1], 1e-12))

        denominator = left - 2.0 * center + right

        if abs(denominator) < 1e-12:
            return float(amplitudes[peak_bin])

        offset = 0.5 * (left - right) / denominator

        # Interpolated peak magnitude
        peak_amplitude = np.exp(
            center - 0.25 * (left - right) * offset
        )

        return float(peak_amplitude)

    # =================================================
    #             FIND HARMONIC AMPLITUDE
    # =================================================

    def find_harmonic(self, frequencies, amplitudes, target_frequency):

        nyquist = SAMPLING_FREQUENCY / 2.0

        if target_frequency >= nyquist:
            return 0.0

        bin_width = SAMPLING_FREQUENCY / FFT_SIZE

        center_bin = int(round(target_frequency / bin_width))

        search_radius = 2

        first_bin = max(1, center_bin - search_radius)

        last_bin = min(
            len(amplitudes) - 2,
            center_bin + search_radius
        )

        if first_bin > last_bin:
            return 0.0

        peak_bin = first_bin + int(
            np.argmax(amplitudes[first_bin:last_bin + 1])
        )

        return self.interpolate_peak(amplitudes, peak_bin)

    # =================================================
    #                 CALCULATE THD
    # =================================================

    def calculate(self, fft_analyzer):

        frequencies = fft_analyzer.get_frequencies()
        amplitudes = fft_analyzer.get_amplitudes()

        fundamental_frequency = (
            fft_analyzer.get_fundamental_frequency()
        )

        fundamental_amplitude = (
            fft_analyzer.get_fundamental_amplitude()
        )

        self.harmonic_amplitudes = []
        self.harmonic_frequencies = []

        self.fundamental_frequency = fundamental_frequency
        self.fundamental_amplitude = fundamental_amplitude

        self.thd_percentage = 0.0
        self.valid = False

        if fundamental_frequency <= 0 or fundamental_amplitude <= 1e-9:
            return False

        harmonic_power = 0.0

        # Harmonics 2 through HARMONIC_COUNT
        for harmonic_number in range(2, HARMONIC_COUNT + 1):

            target_frequency = (
                harmonic_number * fundamental_frequency
            )

            harmonic_amplitude = self.find_harmonic(
                frequencies,
                amplitudes,
                target_frequency
            )

            self.harmonic_frequencies.append(target_frequency)
            self.harmonic_amplitudes.append(harmonic_amplitude)

            harmonic_power += harmonic_amplitude ** 2

        # THD calculation
        harmonic_rms = np.sqrt(harmonic_power)

        self.thd_percentage = (
            harmonic_rms / fundamental_amplitude
        ) * 100.0

        self.valid = True

        return True

    # =================================================
    #                    GETTERS
    # =================================================

    def get_thd(self):
        return self.thd_percentage

    def get_fundamental_frequency(self):
        return self.fundamental_frequency

    def get_fundamental_amplitude(self):
        return self.fundamental_amplitude

    def get_harmonic_amplitudes(self):
        return self.harmonic_amplitudes

    def get_harmonic_frequencies(self):
        return self.harmonic_frequencies

    def is_valid(self):
        return self.valid
