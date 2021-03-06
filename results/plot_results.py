import re
import sys

import matplotlib.pyplot as plt
import pandas as pd

if len(sys.argv) != 2 or not re.search('\.csv$', sys.argv[-1]):
    print("Csv filename not entered.")
    sys.exit(1)

filename = sys.argv[-1]
data = pd.read_csv(filename)
threads_or_tasks_str = 'Threads' if data.columns.contains('Threads') else 'Tasks'
threads_or_tasks = data[threads_or_tasks_str].max()
plot_lines = int(data.shape[0] / threads_or_tasks)
markers = ['o', 'v', '^', '<', '>', 's', 'p', '*', 'h',
           'H', '+', 'x', 'D', '8', '1', '2', '3', '4', 'd']
arr_lengths = []
subp_layout_right = 0.8

fig, (ax1, ax2) = plt.subplots(2, 1, sharex=True)

plt.suptitle('Speedup and efficiency')
plt.subplots_adjust(right=subp_layout_right)

for line in range(plot_lines):
    ax1.plot(threads_or_tasks_str, 'Speedup', linestyle='--', marker=markers[line], antialiased=True,
             linewidth=0.7, data=data.iloc[line * threads_or_tasks + 1:(line + 1) * threads_or_tasks])
    arr_lengths.append(data.iloc[line * threads_or_tasks]['Array length'])
ax1.set_ylabel('Speedup')
ax1.set_ylim((1, threads_or_tasks))
ax1.grid(linestyle='--', linewidth=0.5, axis='y')

for line in range(plot_lines):
    ax2.plot(threads_or_tasks_str, 'Efficiency', linestyle='--', marker=markers[line], antialiased=True,
             linewidth=0.7, data=data.iloc[line * threads_or_tasks + 1:(line + 1) * threads_or_tasks])
ax2.set_ylabel('Efficiency')
ax2.set_xlabel('Number of ' + threads_or_tasks_str.lower())
ax2.grid(linestyle='--', linewidth=0.5, axis='y')

fig.legend(arr_lengths, title='Array length', bbox_to_anchor=(subp_layout_right + 0.005, 0.5), loc='center left',
           ncol=2, bbox_transform=plt.gcf().transFigure)

fig.savefig(filename.split(sep='.')[0] + '.png', bbox_inches="tight")

plt.show()
