import matplotlib.pyplot as plt
import numpy as np
import chess
#https://matplotlib.org/stable/gallery/lines_bars_and_markers/barchart.html#sphx-glr-gallery-lines-bars-and-markers-barchart-py
def plot_wins(tournamentStats, number_matches, file_name):
    width = 0.25
    multiplier = 0
    fig, ax = plt.subplots(layout="constrained")
    engines = [round.engine_opponent for round in tournamentStats]
    x = np.arange(len(engines))
    values = {
        'wins' : [round.wins for round in tournamentStats],
        'draws' : [round.draws for round in tournamentStats],
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
    plt.savefig(file_name)

def plot_draws(tournamentStats, number_matches, file_name):
    width = 0.10
    multiplier = 0
    fig, ax = plt.subplots(layout="constrained")
    engines = [round.engine_opponent for round in tournamentStats]
    x = np.arange(len(engines))
    threefold = [round.draws_reasons[chess.Termination.THREEFOLD_REPETITION] for round in tournamentStats]
    fifty = [round.draws_reasons[chess.Termination.FIFTY_MOVES] for round in tournamentStats]
    stalemate = [round.draws_reasons[chess.Termination.STALEMATE] for round in tournamentStats]
    insufficient = [round.draws_reasons[chess.Termination.INSUFFICIENT_MATERIAL] for round in tournamentStats]
    values = {
        'threefold': threefold,
        'fifty': fifty,
        'stalemate' : stalemate,
        'insufficient' :insufficient
    }
    for attribute, measurement in values.items():
        offset = width * multiplier
        rects = ax.bar(x + offset, measurement, width, label=attribute)
        ax.bar_label(rects, padding=3)
        multiplier += 1

    ax.set_ylabel('Number of games')
    ax.set_title('Game draws divided by reason with ' +  str(tournamentStats[0].seconds_move) + ' seconds')
    ax.set_xticks(x + width, engines)
    ax.legend(loc='upper left', ncols=len(engines))
    ax.set_ylim(0, number_matches + 2)
    plt.savefig(file_name)

def plot_win_time(time_seconds_arr, file_name, **engines_stats):
    num_engines = len(engines_stats) 
    seconds = [str(s) + " seconds" for s in time_seconds_arr]
    x = np.arange(len(seconds))  

    fig, axes = plt.subplots(num_engines, 1, figsize=(8, 4 * num_engines), layout="constrained")

    if num_engines == 1:
        axes = [axes]
    engine_keys = list(engines_stats.keys())
    for i, engine in enumerate(engines_stats.values()):
        ax = axes[i]  
        width = 0.8  
        rects = ax.bar(x, engine, width, label='% wins')  
        ax.bar_label(rects, padding=2) 
        ax.set_ylabel('% wins')
        ax.set_title(f'Engine {engine_keys[i]} wins')
        ax.set_xticks(x)
        ax.set_xticklabels(seconds)  
        ax.set_ylim(0, 1)  
        ax.legend(loc='upper left')
        
    plt.savefig(file_name)

plot_win_time([0.1,0.5], r"C:\Users\belle\OneDrive\Desktop\chess_thesis\BeautyEngine\tools\file.png",stockfish=[0.3,0.1],strong=[0.2,0.1] ,fair=[0.6,0.5])
