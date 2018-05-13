import matplotlib.pyplot as plt
import pandas as pd

THREADS = 8
filename = 'openmp.csv'
data = pd.read_csv(filename)
plot_lines = int(data.shape[0] / THREADS)
markers = ['o', 'v', '^', '<', '>', 's', 'p', '*', 'h',
           'H', '+', 'x', 'D', '8', '1', '2', '3', '4', 'd']
arr_lengths = []
subp_layout_right = 0.9

plt.suptitle('Speedup and efficiency')
plt.subplots_adjust(right=subp_layout_right)

plt.subplot(211)
for line in range(plot_lines):
    plt.plot('Threads', 'Speedup', linestyle='--', marker=markers[line], antialiased=True,
             linewidth=0.7, data=data.iloc[line * THREADS + 1:(line + 1) * THREADS])
    arr_lengths.append(data.iloc[line * THREADS]['Array length'])
plt.ylabel('Speedup')
plt.ylim((1, THREADS))
plt.grid(linestyle='--', linewidth=0.5, axis='y')

plt.subplot(212)
for line in range(plot_lines):
    plt.plot('Threads', 'Efficiency', linestyle='--', marker=markers[line], antialiased=True,
             linewidth=0.7, data=data.iloc[line * THREADS + 1:(line + 1) * THREADS])
plt.ylabel('Efficiency')
plt.xlabel('Number of threads')
plt.grid(linestyle='--', linewidth=0.5, axis='y')

plt.legend(arr_lengths, title='Array length', bbox_to_anchor=(subp_layout_right + 0.005, 0.5), loc='center left',
           ncol=2, bbox_transform=plt.gcf().transFigure)

plt.savefig(filename.split(sep='.')[0] + '.png', bbox_inches="tight")
plt.show()
