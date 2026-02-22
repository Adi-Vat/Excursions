import matplotlib.pyplot as plt
import pandas as pd

#rawData = pd.read_csv('bin\\Debug\\net10.0\\ac.csv', header=None, names=['Time', 'V', 'I', 'W'])
rawData = pd.read_csv('bin\\Debug\\net10.0\\dwBydt.csv', header=None, names=['W', 'dwForward', 'dwBackward'])
data = rawData

#plot1, ax1 = plt.subplots()
'''
plt.plot(data['V'], data['I'])
plt.xlabel('Voltage / V')
plt.ylabel('Current / A')
plt.title('Transient Hysteresis Loop [av853]')
plt.axhline(y=0, color='black', linewidth=0.2)
plt.axvline(x=0, color='black', linewidth=0.2)

'''
plot2, ax2 = plt.subplots()
ax2.plot(data['W'], data['dwForward'], color = 'blue')
ax2.set_ylabel('Forward rate / /s', color='blue')

ax3 = ax2.twinx()
ax3.plot(data['W'], data['dwBackward'], color='red')
ax3.set_ylabel('Backward rate / /s', color='red')

ax2.set_title('Change in dw/dt across w [av853]')
ax2.set_xlabel('State')

plot2.tight_layout()

plt.show()