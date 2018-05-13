import pandas as pd

filename = 'openmp.csv'
data = pd.read_csv(filename)

data.drop(labels=['Registers', 'Seed'], axis=1) \
    .to_csv(filename.split('.')[0] + '_converted.csv', index=False, float_format='%.3f')
