import matplotlib.pyplot as plt
from matplotlib.ticker import ScalarFormatter
import numpy as np

plt.style.use('dark_background')

# Data
pageSizes = ['4KiB', '8KiB', '16KiB', '32KiB', '64KiB', '128KiB', '256KiB', '512KiB', '1MiB', '2MiB']

baselineTimes = {
    'full':  [
        412151675,
        412151675,
        412151675,
        412151675,
        412151675,
        412151675,
        412151675,
        412151675,
        412151675,
        412151675,
        ],
    'partial':   [
        197411003,
        197411003,
        197411003,
        197411003,
        197411003,
        197411003,
        197411003,
        197411003,
        197411003,
        197411003,
    ],
}
mappableTimes = {
        'full':
           [
804633890,
680136734,
618507788,
583336356,
569517646,
562461534,
557894614,
561249240,
566379819,
401222490,
    ],
    'partial':
         [
384460853,
320858290,
295480659,
278870259,
269779252,
260260630,
261239140,
268476012,
270906293,
192855687,


    ],
}
copyTimes = {
    'full':
     [
        1074941751,
        1074941751,
        1074941751,
        1074941751,
        1074941751,
        1074941751,
        1074941751,
        1074941751,
        1074941751,
        1074941751,
    ],
    'partial':
         [
        613948315,
        613948315,
        613948315,
        613948315,
        613948315,
        613948315,
        613948315,
        613948315,
        613948315,
        613948315,
    ]
}

for mode in ['full', 'partial']:
    # Create figure and axis
    fig, ax = plt.subplots()

    # Plot
    ax.plot(pageSizes, baselineTimes[mode], marker='o', label='baseline')
    ax.plot(pageSizes, mappableTimes[mode], marker='o', label='mappable')
    ax.plot(pageSizes, copyTimes[mode], marker='o', label='copy')

    # Labels and title
    ax.set_xlabel('Page Size')
    saveTitle = ''
    plotTitle = ''
    if mode == 'full':
        saveTitle = 'FLOS-1GiB'
        plotTitle = '1GiB Array Size'
    else:
        saveTitle = 'FLOS-random'
        plotTitle = 'Random Array Size'

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
