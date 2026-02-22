import matplotlib.pyplot as plt
import pandas as pd

data = pd.read_csv('bin\\Debug\\net10.0\\pulse.csv', header=None, names=['Time', 'V', 'I', 'W'])
plot1, ax1 = plt.subplots()
plt.plot(data['V'], data['I'])
plt.xlabel('Voltage / V')
plt.ylabel('Current / A')
plt.title('Hysteresis Loop [av853]')
plt.axhline(y=0, color='black', linewidth=0.2)
plt.axvline(x=0, color='black', linewidth=0.2)

plot2, ax2 = plt.subplots()
ax2.plot(data['Time'], data['W'], label='State', color = 'blue')
ax2.set_ylabel('State', color='blue')

ax3 = ax2.twinx()
ax3.plot(data['Time'], data['V'], label='Voltage', color='red')
ax3.set_ylabel('Voltage / V', color='red')

'''
ax4 = ax2.twinx()
ax4.plot(data['Time'], data['V']/data['I'], label="Current", color='orange')
ax4.set_ylabel('Memristance / Ω', color='orange')
'''

ax2.set_title('Voltage and State vs Time [av853]')
ax2.set_xlabel('Time / s')

plot2.tight_layout()

plt.show()