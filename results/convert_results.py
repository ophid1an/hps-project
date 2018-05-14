import re
import sys

import numpy as np
import pandas as pd
from scipy import stats

if len(sys.argv) != 2 or not re.search('\.csv$', sys.argv[-1]):
    print("Csv filename not entered.")
    sys.exit(1)

filename = sys.argv[-1]
data = pd.read_csv(filename)

arr_lengths = np.sort(data['Array length'].unique())
runs = data['Run'].max()
threads = data['Threads'].max()

if len(arr_lengths) * runs * threads != data.shape[0]:
    print("Invalid csv file.")
    sys.exit(1)

data_converted = pd.DataFrame(
    columns=list(data.drop(labels=['Registers', 'Seed', 'Run'], axis=1).columns) + ['Speedup', 'Efficiency'])

for arr_len in arr_lengths:
    for thread in range(1, threads + 1):
        temp_df = data[(data['Array length'] == arr_len) & (data['Threads'] == thread)]
        trim_mean_time = stats.trim_mean(temp_df['Time'], 0.25)  # Get interquartile mean of times
        data_converted = data_converted.append(
            pd.DataFrame([[arr_len, thread, trim_mean_time, temp_df['Percent error'].iloc[0], 1.0, 1.0]],
                         columns=data_converted.columns), ignore_index=True)

if threads > 1:
    for arr_len in arr_lengths:
        time_one_thread = \
            data_converted[(data_converted['Array length'] == arr_len) & (data_converted['Threads'] == 1)]['Time'].iloc[
                0]
        for thread in range(2, threads + 1):
            idx = data_converted.index[
                (data_converted['Array length'] == arr_len) & (data_converted['Threads'] == thread)]
            data_converted.loc[idx, 'Speedup'] = time_one_thread / data_converted.loc[idx, 'Time']
            data_converted.loc[idx, 'Efficiency'] = data_converted.loc[idx, 'Speedup'] / thread

data_converted.to_csv(filename.split('.')[0] + '_converted.csv',
                      columns=['Array length', 'Threads', 'Time', 'Speedup', 'Efficiency', 'Percent error'],
                      index=False, float_format='%.3f')
