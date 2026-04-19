import matplotlib.pyplot as plt
import pandas as pd

data = pd.read_csv('output.csv', header=None, names=['Time', 'value1', 'value2', 'value3', 'value4'])

plot1, ax1 = plt.subplots()
ax1.plot(data['Time'], data['value1'])
ax2 = ax1.twinx()
ax2.plot(data['Time'], data['value2'])
ax3 = ax1.twinx()
ax3.plot(data['Time'], data['value 3'])

plt.show()