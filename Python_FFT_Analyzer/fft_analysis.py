
import numpy as np

from config import (
    FFT_SIZE,
    SAMPLING_FREQUENCY,
    MIN_FREQUENCY,
    MAX_FREQUENCY
)


class FFTAnalysis:

    def __init__(self):

        self.frequencies = None
        self.amplitudes = None

        self.fundamental_frequency = 0.0
        self.fundamental_amplitude = 0.0

        self.window = np.hanning(FFT_SIZE)

        # Coherent gain correction
        self.window_sum = np.sum(self.window)

    # =================================================
    #                  APPLY HANN WINDOW
    # =================================================

    def apply_window(self, samples):

        samples = np.asarray(samples, dtype=np.float64)

        if len(samples) != FFT_SIZE:
            raise ValueError(
                f"Expected {FFT_SIZE} samples, got {len(samples)}."
            )

        # Remove any remaining DC component
        samples = samples - np.mean(samples)

        # Apply Hann window
        windowed_samples = samples * self.window

        return windowed_samples

    # =================================================
    #                  PERFORM FFT
    # =================================================

    def perform_fft(self, samples):

        windowed_samples = self.apply_window(samples)

        # Real FFT
        fft_result = np.fft.rfft(windowed_samples)

        # Magnitude spectrum
        magnitudes = np.abs(fft_result)

        # Single-sided amplitude spectrum
        amplitudes = magnitudes / self.window_sum

        # Double non-DC and non-Nyquist components
        amplitudes[1:-1] *= 2.0

        # Frequency axis
        frequencies = np.fft.rfftfreq(
            FFT_SIZE,
            d=1.0 / SAMPLING_FREQUENCY
        )

        self.frequencies = frequencies
        self.amplitudes = amplitudes

        # Find fundamental
        self.find_fundamental()

        return frequencies, amplitudes

    # =================================================
    #             FUNDAMENTAL DETECTION
    # =================================================

    def find_fundamental(self):

        if self.amplitudes is None:
            return

        # Search within configured frequency range
        valid_bins = np.where(
            (self.frequencies >= MIN_FREQUENCY) &
            (self.frequencies <= MAX_FREQUENCY)
        )[0]

        if len(valid_bins) == 0:
            return

        # Find largest spectral component
        peak_index = valid_bins[
            np.argmax(self.amplitudes[valid_bins])
        ]

        # Parabolic interpolation
        if 0 < peak_index < len(self.amplitudes) - 1:

            left = self.amplitudes[peak_index - 1]
            center = self.amplitudes[peak_index]
            right = self.amplitudes[peak_index + 1]

            left = np.log(max(left, 1e-12))
            center = np.log(max(center, 1e-12))
            right = np.log(max(right, 1e-12))

            denominator = left - 2 * center + right

            if abs(denominator) > 1e-12:

                offset = 0.5 * (
                    left - right
                ) / denominator

            else:
                offset = 0.0

        else:
            offset = 0.0

        bin_width = SAMPLING_FREQUENCY / FFT_SIZE

        self.fundamental_frequency = (
            peak_index + offset
        ) * bin_width

        # Interpolated peak amplitude
        if abs(denominator) > 1e-12 if (
            0 < peak_index < len(self.amplitudes) - 1
        ) else False:

            self.fundamental_amplitude = np.exp(
                center - 0.25 * (left - right) * offset
            )

        else:
            self.fundamental_amplitude = (
                self.amplitudes[peak_index]
            )

    # =================================================
    #                  GETTERS
    # =================================================

    def get_frequencies(self):
        return self.frequencies

    def get_amplitudes(self):
        return self.amplitudes

    def get_fundamental_frequency(self):
        return self.fundamental_frequency

    def get_fundamental_amplitude(self):
        return self.fundamental_amplitude
