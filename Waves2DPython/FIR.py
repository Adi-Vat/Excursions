from scipy.signal import firwin, lfilter
import spectrum_analysis as spc
import numpy as np

def design_bandpass_filter(sample_rate, low_cutoff, high_cutoff, num_taps):
    """
    Designs a FIR bandpass filter using the Hamming window method.

    Parameters:
        sample_rate (float): sampling frequency in Hz
        low_cutoff (float): lower cutoff frequency in Hz
        high_cutoff (float): upper cutoff frequency in Hz
        num_taps (int): number of filter coefficients

    Returns:
        np.array: filter coefficients
    """
    nyquist = sample_rate / 2
    coefficients = firwin(
        num_taps,
        [low_cutoff / nyquist, high_cutoff / nyquist],
        pass_zero=False,
        window='hamming'
    )
    return coefficients

def apply_filter(signal, coefficients):
    """
    Applies FIR filter to a signal.

    Parameters:
        signal (np.array): input signal
        coefficients (np.array): filter coefficients

    Returns:
        np.array: filtered signal
    """
    return lfilter(coefficients, 1.0, signal)

coefficients = design_bandpass_filter(
    sample_rate=200,
    low_cutoff=3.5,
    high_cutoff=4.5,
    num_taps=3201  # must be odd for bandpass
)

if __name__ == "__main__":
    time, signal = spc.load_csv('output.csv')
    spc.plot_time_domain(time, signal)
    (frequencies, amplitudes) = spc.compute_spectrum(time, signal)
    spc.plot_spectrum(frequencies, amplitudes)
    
    n = len(signal)
    # interpolate onto uniform time grid to correct for irregular sampling
    uniform_time = np.linspace(time[0], time[-1], n)
    uniform_signal = np.interp(uniform_time, time, signal)
    filtered_signal = apply_filter(uniform_signal, coefficients)
    spc.plot_time_domain(time, filtered_signal)

    (frequencies, amplitudes) = spc.compute_spectrum(uniform_time, filtered_signal)
    spc.plot_spectrum(frequencies, amplitudes)

    

