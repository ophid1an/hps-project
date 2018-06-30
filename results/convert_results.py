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
threads_or_tasks_str = 'Threads' if data.columns.contains('Threads') else 'Tasks'
threads_or_tasks = data[threads_or_tasks_str].max()

if len(arr_lengths) * runs * threads_or_tasks != data.shape[0]:
    print("Invalid csv file.")
    sys.exit(1)

data_converted = pd.DataFrame(
    columns=list(data.drop(labels=['Registers', 'Run'], axis=1).columns) + ['Speedup', 'Efficiency'])

for arr_len in arr_lengths:
    for t in range(1, threads_or_tasks + 1):
        temp_df = data[(data['Array length'] == arr_len) & (data[threads_or_tasks_str] == t)]
        trim_mean_time = stats.trim_mean(temp_df['Time'], 0.25)  # Get interquartile mean of times
        data_converted = data_converted.append(
            pd.DataFrame([[arr_len, t, trim_mean_time, temp_df['Percent error'].iloc[0], 1.0, 1.0]],
                         columns=data_converted.columns), ignore_index=True)

if threads_or_tasks > 1:
    for arr_len in arr_lengths:
        time_one_t = \
            data_converted[(data_converted['Array length'] == arr_len) & (data_converted[threads_or_tasks_str] == 1)][
                'Time'].iloc[
                0]
        for t in range(2, threads_or_tasks + 1):
            idx = data_converted.index[
                (data_converted['Array length'] == arr_len) & (data_converted[threads_or_tasks_str] == t)]
            data_converted.loc[idx, 'Speedup'] = time_one_t / data_converted.loc[idx, 'Time']
            data_converted.loc[idx, 'Efficiency'] = data_converted.loc[idx, 'Speedup'] / t

data_converted['Array length'] = data_converted['Array length'].astype(str)
for idx in data_converted.index.values:
    data_converted.loc[idx, 'Array length'] = '2^' + data_converted.loc[idx, 'Array length']

data_converted.to_csv(filename.split('.')[0] + '_converted.csv',
                      columns=['Array length', threads_or_tasks_str, 'Time', 'Speedup', 'Efficiency', 'Percent error'],
                      index=False, float_format='%.3f')
