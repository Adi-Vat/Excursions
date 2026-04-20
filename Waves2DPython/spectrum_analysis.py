import numpy as np
import matplotlib.pyplot as plt
import csv

def load_csv(filename):
    """
    Loads time series data from a CSV file.

    Parameters:
        filename (str): path to the CSV file

    Returns:
        tuple: (time, signal) as numpy arrays
    """
    time = []
    signal = []

    with open(filename, 'r') as f:
        reader = csv.reader(f)
        for row in reader:
            time.append(float(row[0]))
            signal.append(float(row[1]))

    return np.array(time), np.array(signal)


def compute_spectrum(time, signal):
    """
    Computes the single-sided amplitude spectrum of a signal.

    Parameters:
        time (np.array): time values
        signal (np.array): amplitude values

    Returns:
        tuple: (frequencies, amplitudes) as numpy arrays
    """
    n = len(signal)
    # interpolate onto uniform time grid to correct for irregular sampling
    uniform_time = np.linspace(time[0], time[-1], n)
    uniform_signal = np.interp(uniform_time, time, signal)
    sample_interval = uniform_time[1] - uniform_time[0]

    # apply Hamming window to reduce spectral leakage
    window = np.hamming(n)
    windowed_signal = uniform_signal * window

    fft_result = np.fft.fft(windowed_signal)
    frequencies = np.fft.fftfreq(n, d=sample_interval)

    positive = frequencies > 0
    frequencies = frequencies[positive]
    amplitudes = (2 / np.sum(window)) * np.abs(fft_result[positive])

    return frequencies, amplitudes


def plot_spectrum(frequencies, amplitudes):
    """
    Plots the amplitude spectrum.

    Parameters:
        frequencies (np.array): frequency values in Hz
        amplitudes (np.array): amplitude values
    """
    plt.figure(figsize=(10, 5))
    plt.plot(frequencies, amplitudes)
    plt.xlabel('Frequency [Hz]')
    plt.ylabel('Amplitude [a.u.]')
    plt.title('Amplitude Spectrum - av853')
    plt.xlim(0, 10)
    plt.grid(True)
    plt.tight_layout()
    plt.savefig('spectrum.png', dpi=150)
    plt.show()


def plot_time_domain(time, signal):
    """
    Plots the raw time domain signal.

    Parameters:
        time (np.array): time values
        signal (np.array): amplitude values
    """
    plt.figure(figsize=(10, 5))
    plt.plot(time, signal)
    plt.xlabel('Time [s]')
    plt.ylabel('Amplitude')
    plt.title('Time Domain Signal - av853')
    plt.grid(True)
    plt.tight_layout()
    plt.savefig('time_domain.png', dpi=150)
    plt.show()


if __name__ == "__main__":
    time, signal = load_csv('output.csv')

    plot_time_domain(time, signal)

    frequencies, amplitudes = compute_spectrum(time, signal)
    plot_spectrum(frequencies, amplitudes)

    dominant_idx = np.argmax(amplitudes)
    print(f"Dominant frequency: {frequencies[dominant_idx]:.2f} Hz")
    print(f"Dominant amplitude: {amplitudes[dominant_idx]:.4f}")
    print(f"Sample rate: {1/(time[1]-time[0]):.1f} Hz")
    print(f"Frequency resolution: {1/(time[-1]-time[0]):.3f} Hz")
    print(f"Total samples: {len(time)}")
