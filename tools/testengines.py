import chess
import chess.engine
import chess.pgn
import sys
import os
import glob
import random
from tqdm import tqdm
from os import path
from plotting import plot_wins, plot_draws, plot_win_time
from datetime import datetime

options = {"Skill level": "sl"}


os.chdir(r"C:\Users\belle\OneDrive\Desktop\chess_thesis\BeautyEngine")
strong_engines = [fn for fn in glob.glob("engines/strong/**/*.exe", recursive=True)]
fair_engines = [fn for fn in glob.glob("engines/fair/**/*.exe", recursive=True)]
# myEngineDir
my_engine = "BeautyEngine.exe"
# removed due lc0 messages of logging...
# sys.stderr = open(os.devnull, 'w')


class Tournament:
    def __init__(
        self,
        number_matches,
        seconds_move,
        dir_engine_test,
        other_engine_options={},
        *other_engine_dirs
    ):
        self.number_matches = number_matches
        self.seconds_move = seconds_move
        self.other_engine_dirs = other_engine_dirs
        self.dir_engine_test = dir_engine_test
        self.other_engine_options = other_engine_options
        self.statistics = []

    def run(self):
        pbar = tqdm(
            range(len(self.other_engine_dirs)),
            file=sys.stdout,
            total=(len(self.other_engine_dirs) * self.number_matches),
        )
        for i in range(len(self.other_engine_dirs)):
            dir_engine_opponent = self.other_engine_dirs[i]
            engine_opponent = get_engine_name_from_dir(dir_engine_opponent, self.other_engine_options)
            round_stats = RoundStatistics(self.seconds_move, engine_opponent)
            info = {
                "Event": "Tournament for my engine",
                "Date": datetime.today().strftime("%d-%m-%Y"),
                "Site": "Italy",
            }
            for j in range(self.number_matches):
                info["Round"] = j + 1
                pbar.set_description(
                    "Playing against %s match number %d/%d"
                    % (engine_opponent, j + 1, self.number_matches)
                )
                game = Game(
                    self.seconds_move,
                    self.dir_engine_test,
                    dir_engine_opponent,
                    self.other_engine_options,
                    info,
                )
                game_result = game.play()
                if game_result.result() == "1/2-1/2":
                    round_stats.draws+=1
                    round_stats.draws_reasons[game_result.termination] += 1
                elif game_result.winner:
                    round_stats.wins += 1
                else:
                    round_stats.losses += 1
                pbar.update()
            i += 1
            self.statistics.append(round_stats)
        pbar.close()


class RoundStatistics:
    def __init__(self, seconds_move, engine_opponent):
        self.seconds_move = seconds_move
        self.engine_opponent = engine_opponent
        self.wins = 0
        self.losses = 0
        self.draws = 0
        self.draws_reasons = {
            chess.Termination.THREEFOLD_REPETITION: 0,
            chess.Termination.FIFTY_MOVES: 0,
            chess.Termination.STALEMATE: 0,
            chess.Termination.INSUFFICIENT_MATERIAL: 0,
        }


class Game:
    def __init__(
        self,
        seconds_move,
        dir_engine_test,
        dir_other_engine,
        other_engine_options,
        info,
    ):
        self.seconds_move = seconds_move
        self.dir_engine_to_test = dir_engine_test
        self.dir_other_engine = dir_other_engine
        self.other_engine_options = other_engine_options
        self.pgn = None
        self.info = info

    def play(
        self,
        starting_position="rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
    ):
        engine, engine2 = self.setting_engines()
        result = None
        engine_test_turn = bool(random.getrandbits(1))
        board = chess.Board(starting_position)
        node = self.init_pgn(starting_position, engine_test_turn)
        outcome = board.outcome(claim_draw=True)
        while not outcome:
            result = self.next_move(engine, engine2, engine_test_turn, board)
            board.push(result.move)
            node = node.add_variation(result.move)
            outcome = board.outcome(claim_draw=True)
        self.close_engines(engine, engine2)
        self.pgn.headers["Result"] = outcome.result()
        filename = "tools/games/games_" + str(self.seconds_move) + ".pgn"
        # https://stackoverflow.com/questions/20432912/writing-to-a-new-file-if-it-doesnt-exist-and-appending-to-a-file-if-it-does
        append_write = ""
        if os.path.exists:
            append_write = "a"
        else:
            append_write = "w"
        with open(filename, append_write) as f:
            print(self.pgn, file=f, end="\n\n")
        if outcome.winner != None:
            return chess.Outcome(
                outcome.termination, outcome.winner == engine_test_turn
            )
        else:
            return outcome

    def init_pgn(self, starting_position, engine_test_turn):
        self.pgn = chess.pgn.Game()
        self.pgn.setup(starting_position)
        self.pgn.headers["Event"] = self.info["Event"]
        self.pgn.headers["Date"] = self.info["Date"]
        self.pgn.headers["Round"] = self.info["Round"]
        self.pgn.headers["Site"] = self.info["Site"]
        if engine_test_turn == chess.WHITE:
            self.pgn.headers["White"] = get_engine_name_from_dir(
                self.dir_engine_to_test
            )
            other_engine = get_engine_name_from_dir(self.dir_other_engine, self.other_engine_options)
            self.pgn.headers["Black"] = other_engine
        else:
            self.pgn.headers["Black"] = get_engine_name_from_dir(
                self.dir_engine_to_test
            )
            other_engine = get_engine_name_from_dir(self.dir_other_engine, self.other_engine_options)
            self.pgn.headers["White"] = other_engine
        node = self.pgn
        return node

    def setting_engines(self):
        engine = chess.engine.SimpleEngine.popen_uci(self.dir_engine_to_test)
        engine2 = chess.engine.SimpleEngine.popen_uci(self.dir_other_engine)
        engine2.configure(self.other_engine_options)
        return engine, engine2

    def next_move(self, engine, engine2, engine_test_turn, board):
        if engine_test_turn == chess.WHITE:
            if board.turn == chess.WHITE:
                result = engine.play(board, chess.engine.Limit(self.seconds_move))
            else:
                result = engine2.play(board, chess.engine.Limit(self.seconds_move))
        else:
            if board.turn == chess.BLACK:
                result = engine.play(board, chess.engine.Limit(self.seconds_move))
            else:
                result = engine2.play(board, chess.engine.Limit(self.seconds_move))
        return result

    def close_engines(self, engine, engine2):
        engine.quit()
        engine2.quit()


def get_engine_name_from_dir(dir_other_engine, other_options = {}):
    if other_options=={}:
        return path.basename(path.normpath(dir_other_engine)).replace("-", ".").split(".")[0]
    else:
        return path.basename(path.normpath(dir_other_engine)).replace("-", ".").split(".")[0] + format_options(other_options)

def format_options(other_options):
    str_final = ""
    for key, value in other_options.items():
        if key in options:
            key = options[key]
        str_final = "".join([str_final + "_", key + ":", str(value)])
    return str_final




def test_stockfisk_skill_level(number_matches, seconds_time, range_level):
    flat_stats = []
    for i in range_level:
        t = Tournament(number_matches, seconds_time, my_engine, {"Skill level": i}, *strong_engines[:-1])
        t.run()
        flat_stats.append(t.statistics)
    flat_stats = [round for tournament in flat_stats for round in tournament]
    plot_wins(flat_stats, t.number_matches,  os.getcwd() + "/tools/results/wskill"+ str(i) + "-" + "t"+str(seconds_time) + ".png")
    plot_draws(flat_stats, t.number_matches, os.getcwd() +"/tools/results/dskill"+ str(i) + "-" + "t"+str(seconds_time)+".png")
    return flat_stats

def test_fair_engines(number_matches, seconds_time):
    t = Tournament(number_matches, seconds_time, my_engine, {} ,*fair_engines)
    t.run()
    plot_wins(t.statistics, t.number_matches, os.getcwd() + "/tools/results/wfair"+ "t"+str(seconds_time) + ".png")
    plot_draws(t.statistics, t.number_matches,os.getcwd() + "/tools/results/dfair"+ "t"+str(seconds_time) + ".png")
    return t.statistics

def test_strong_engines(number_matches, seconds_time):
    t = Tournament(number_matches, seconds_time, my_engine, {},*strong_engines)
    t.run()
    plot_wins(t.statistics, t.number_matches, os.getcwd() + "/tools/results/wstrong"+ "t"+str(seconds_time) + ".png")
    plot_draws(t.statistics, t.number_matches, os.getcwd() + "/tools/results/dstrong"+ "t"+str(seconds_time) + ".png")
    return t.statistics

time_seconds_arr = [0.05,0.1]
number_matches = 2
low = 3
up = 5
range_stockfish_level = range(low,up)
stockfish_sl_stats = []
fair_engines_stats = []
strong_stats = []
win_stockfish_sl = []
win_fair_engine = []
win_strong = []

for i in range(len(time_seconds_arr)):
    stockfish_skill = test_stockfisk_skill_level(number_matches,time_seconds_arr[i],range_stockfish_level)
    fair = test_fair_engines(number_matches, time_seconds_arr[i])
    strong = test_strong_engines(number_matches, time_seconds_arr[i])
    stockfish_sl_stats.append(sum([round.wins for round in stockfish_skill])/(number_matches * (up-low)))
    fair_engines_stats.append(sum([round.wins for round in fair])/(number_matches * len(fair_engines)))
    strong_stats.append(sum([round.wins for round in strong])/(number_matches * len(strong_engines)))
    
plot_win_time(time_seconds_arr, os.getcwd() + "/tools/results/winrate.png",{"stockfish_sl":stockfish_sl_stats}, {"strong":strong_stats}, {"fair":fair_engines_stats})

