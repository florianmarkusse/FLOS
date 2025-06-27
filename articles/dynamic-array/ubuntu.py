import matplotlib.pyplot as plt
from matplotlib.ticker import ScalarFormatter
import numpy as np

plt.style.use('dark_background')

# Data
pageSizes = ['4KiB', '2MiB']

baselineTimes = {
    'full': {
            'ms': [163, 124],
            'cycles': [571367989, 436341743],
    },
    'partial':  {
            'ms': [81, 59],
        'cycles': [286846880, 208957329],
    }
}
mappableTimes = {
        'full': {
            'ms': [339, 129],
            'cycles': [1186074432, 453931408],
    },
    'partial': {
            'ms': [162, 62],
        'cycles': [570306529, 219778049],
    }
}
copyTimes = {
    'full':    {
            'ms': [365, 365],
            'cycles': [1277500222, 1277500222],
        },
    'partial': {
            'ms': [183, 183],
        'cycles': [641601994, 641601994]
    }
}

for mode in ['full', 'partial']:
    for arraySize in ['ms', 'cycles']:
        # Create figure and axis
        fig, ax = plt.subplots()

        # Plot
        ax.plot(pageSizes, baselineTimes[mode][arraySize], marker='o', label='baseline')
        ax.plot(pageSizes, mappableTimes[mode][arraySize], marker='o', label='mappable')
        ax.plot(pageSizes, copyTimes[mode][arraySize], marker='o', label='copy')

        # Labels and title
        ax.set_xlabel('Page Size')
        saveTitle = ''
        plotTitle = ''
        if mode == 'full':
            saveTitle = 'ubuntu-1GiB'
            plotTitle = '1GiB Array Size'
        else:
            saveTitle = 'ubuntu-random'
            plotTitle = 'Random Array Size'
        if arraySize == 'ms':
            ax.set_ylim(0, 400)
            ax.set_ylabel('Time (ms)')
            saveTitle += '-ms'
            plotTitle += ' Miliseconds'
        else:
            formatter = ScalarFormatter(useMathText=True)
            formatter.set_powerlimits((9, 9))
            ax.yaxis.set_major_formatter(formatter)
            ax.yaxis.get_offset_text().set_visible(False)
            ax.set_ylabel('Clock Cycles (×10⁹)')
            ax.set_ylim(0, 1.4e9)
            saveTitle += '-cycles'
            plotTitle += ' CPU Cycles'

        ax.set_title(plotTitle)

        ax.legend()

        # Grid for clarity
        ax.grid(True, linestyle='--', alpha=0.5)

        fig.tight_layout()
        fig.savefig(f'{saveTitle}.png', dpi=300, bbox_inches='tight')

        plt.close(fig)
