import matplotlib.pyplot as plt
import numpy as np
#https://matplotlib.org/stable/gallery/lines_bars_and_markers/barchart.html#sphx-glr-gallery-lines-bars-and-markers-barchart-py
def plot_wins(tournamentStats, number_matches):
    width = 0.25
    multiplier = 0
    fig, ax = plt.subplots(layout="constrained")
    engines = [round.engine_opponent for round in tournamentStats]
    x = np.arange(len(engines))
    values = {
        'wins' : [round.wins for round in tournamentStats],
        'draws' : [len(round.draws) for round in tournamentStats],
        'losses' : [round.losses for round in tournamentStats]
    }
    for attribute, measurement in values.items():
        offset = width * multiplier
        rects = ax.bar(x + offset, measurement, width, label=attribute)
        ax.bar_label(rects, padding=3)
        multiplier += 1

    ax.set_ylabel('Number of games')
    ax.set_title('Game wins/draws/losses with ' +  str(tournamentStats[0].seconds_move) + ' seconds')
    ax.set_xticks(x + width, engines)
    ax.legend(loc='upper left', ncols=len(engines))
    ax.set_ylim(0, number_matches + 2)
    plt.show()


