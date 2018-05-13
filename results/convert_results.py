import re
import sys

import pandas as pd

if len(sys.argv) != 2 or not re.search('\.csv$', sys.argv[-1]):
    print("Csv filename not entered.")
    sys.exit(1)

filename = sys.argv[-1]
data = pd.read_csv(filename)

data.drop(labels=['Registers', 'Seed'], axis=1) \
    .to_csv(filename.split('.')[0] + '_converted.csv', index=False, float_format='%.3f')
