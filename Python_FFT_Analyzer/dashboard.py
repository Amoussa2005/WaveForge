
import numpy as np
import matplotlib.pyplot as plt

from config import (
    SAMPLING_FREQUENCY,
    FFT_SIZE,
    DISPLAY_MAX_FREQUENCY,
    CHANNELS
)


class WaveForgeDashboard:

    def __init__(self):

        # Enable interactive plotting
        plt.ion()

        self.fig, self.axes = plt.subplots(
            2, 2,
            figsize=(14, 8)
        )

        self.fig.canvas.manager.set_window_title(
            "WaveForge - FFT Analyzer"
        )

        self.fig.suptitle(
            "WAVEFORGE | DIGITAL SIGNAL ANALYZER",
            fontsize=16,
            fontweight="bold"
        )

        self.fig.subplots_adjust(
            hspace=0.38,
            wspace=0.28,
            top=0.90
        )

        self.ax_wave = self.axes[0, 0]
        self.ax_fft = self.axes[0, 1]
        self.ax_harmonics = self.axes[1, 0]
        self.ax_measurements = self.axes[1, 1]

        self.setup_plots()

        plt.show(block=False)

    # =================================================
    #                  INITIALIZE PLOTS
    # =================================================

    def setup_plots(self):

        # Time-domain waveform
        self.ax_wave.set_title("Time-Domain Signal")
        self.ax_wave.set_xlabel("Time (ms)")
        self.ax_wave.set_ylabel("Voltage (V)")
        self.ax_wave.grid(True, alpha=0.3)

        # FFT spectrum
        self.ax_fft.set_title("Frequency Spectrum")
        self.ax_fft.set_xlabel("Frequency (Hz)")
        self.ax_fft.set_ylabel("Amplitude (V peak)")
        self.ax_fft.set_xlim(0, DISPLAY_MAX_FREQUENCY)
        self.ax_fft.grid(True, alpha=0.3)

        # Harmonic amplitudes
        self.ax_harmonics.set_title("Harmonic Analysis")
        self.ax_harmonics.set_xlabel("Harmonic Number")
        self.ax_harmonics.set_ylabel("Amplitude (V peak)")
        self.ax_harmonics.grid(True, alpha=0.3)

        # Measurements
        self.ax_measurements.set_title("Signal Measurements")
        self.ax_measurements.axis("off")

    # =================================================
    #                  UPDATE WAVEFORM
    # =================================================

    def update_waveform(self, voltage_samples):

        time_axis = (
            np.arange(len(voltage_samples))
            / SAMPLING_FREQUENCY
        ) * 1000.0

        self.ax_wave.clear()

        self.ax_wave.plot(
            time_axis,
            voltage_samples,
            linewidth=1.0,
            label="Signal"
        )

        self.ax_wave.set_title("Time-Domain Signal")
        self.ax_wave.set_xlabel("Time (ms)")
        self.ax_wave.set_ylabel("Voltage (V)")
        self.ax_wave.grid(True, alpha=0.3)

    # =================================================
    #                  UPDATE FFT
    # =================================================

    def update_fft(self, frequencies, amplitudes, fundamental):

        self.ax_fft.clear()

        self.ax_fft.plot(
            frequencies,
            amplitudes,
            linewidth=1.0,
            label="FFT Spectrum"
        )

        self.ax_fft.set_xlim(0, DISPLAY_MAX_FREQUENCY)

        self.ax_fft.set_title("Frequency Spectrum")
        self.ax_fft.set_xlabel("Frequency (Hz)")
        self.ax_fft.set_ylabel("Amplitude (V peak)")
        self.ax_fft.grid(True, alpha=0.3)

        if fundamental > 0:

            self.ax_fft.axvline(
                fundamental,
                linestyle="--",
                label=f"Fundamental: {fundamental:.2f} Hz"
            )

            self.ax_fft.legend(fontsize=8)

    # =================================================
    #                UPDATE HARMONICS
    # =================================================

    def update_harmonics(self, thd_analyzer):

        self.ax_harmonics.clear()

        harmonic_amplitudes = (
            thd_analyzer.get_harmonic_amplitudes()
        )

        harmonic_numbers = np.arange(
            2,
            len(harmonic_amplitudes) + 2
        )

        self.ax_harmonics.bar(
            harmonic_numbers,
            harmonic_amplitudes,
            width=0.65
        )

        self.ax_harmonics.set_title("Harmonic Analysis")
        self.ax_harmonics.set_xlabel("Harmonic Number")
        self.ax_harmonics.set_ylabel("Amplitude (V peak)")
        self.ax_harmonics.set_xticks(harmonic_numbers)
        self.ax_harmonics.grid(True, axis="y", alpha=0.3)

    # =================================================
    #                UPDATE MEASUREMENTS
    # =================================================

    def update_measurements(
        self,
        channel,
        signal_processor,
        fft_analyzer,
        thd_analyzer
    ):

        self.ax_measurements.clear()
        self.ax_measurements.axis("off")

        channel_name = CHANNELS.get(channel, "UNKNOWN")

        fundamental_frequency = (
            fft_analyzer.get_fundamental_frequency()
        )

        fundamental_amplitude = (
            fft_analyzer.get_fundamental_amplitude()
        )

        thd_value = thd_analyzer.get_thd()

        measurements = [
            ("Selected Channel", channel_name),
            ("Fundamental Frequency", f"{fundamental_frequency:.2f} Hz"),
            ("Fundamental Amplitude", f"{fundamental_amplitude:.4f} V"),
            ("DC Offset", f"{signal_processor.get_dc_offset():.4f} V"),
            ("Peak-to-Peak", f"{signal_processor.get_peak_to_peak():.4f} V"),
            ("AC RMS", f"{signal_processor.get_rms():.4f} V"),
            ("THD", f"{thd_value:.2f} %" if thd_analyzer.is_valid() else "Invalid")
        ]

        text = "\n\n".join(
            f"{name}:   {value}"
            for name, value in measurements
        )

        self.ax_measurements.text(
            0.05,
            0.95,
            text,
            transform=self.ax_measurements.transAxes,
            verticalalignment="top",
            fontsize=11,
            family="monospace"
        )

        self.ax_measurements.set_title("Signal Measurements")

    # =================================================
    #                  UPDATE DASHBOARD
    # =================================================

    def update(
        self,
        channel,
        signal_processor,
        fft_analyzer,
        thd_analyzer
    ):

        self.update_waveform(
            signal_processor.get_voltage_samples()
        )

        self.update_fft(
            fft_analyzer.get_frequencies(),
            fft_analyzer.get_amplitudes(),
            fft_analyzer.get_fundamental_frequency()
        )

        self.update_harmonics(thd_analyzer)

        self.update_measurements(
            channel,
            signal_processor,
            fft_analyzer,
            thd_analyzer
        )

        self.fig.canvas.draw_idle()
        self.fig.canvas.flush_events()

        plt.pause(0.001)

    # =================================================
    #                    CLOSE
    # =================================================

    def close(self):

        plt.close(self.fig)
