import chess
import chess.engine
import chess.pgn
import sys
import asyncio

import random
from tqdm import tqdm
from os import listdir, path
from plotting import plot_wins

#used dir
dirMyEngine = r"C:\Users\belle\OneDrive\Desktop\chess_thesis\BeautyEngine\BeautyEngine.exe"
dirStockfish = r"C:\Users\belle\OneDrive\Desktop\chess_thesis\BeautyEngine\engines\stockfish-windows-x86-64-avx2.exe"
dirFairyStockfish = r"C:\Users\belle\OneDrive\Desktop\chess_thesis\BeautyEngine\engines\fairy-stockfish_x86-64.exe"
dirLC0 = r"C:\Users\belle\OneDrive\Desktop\chess_thesis\BeautyEngine\engines\lc0-v0.31.1-windows-cpu-dnnl\lc0.exe"
strongEngines = [dirStockfish, dirFairyStockfish, dirLC0]
fairEnginesDir = [f for f in listdir(r"C:\Users\belle\OneDrive\Desktop\chess_thesis\BeautyEngine\engines\fair")]


class Tournament:
    def __init__(self, number_matches, seconds_move, dir_engine_test, other_engine_options = {},*other_engine_dirs):
        self.number_matches = number_matches
        self.seconds_move = seconds_move
        self.other_engine_dirs = other_engine_dirs
        self.dir_engine_test = dir_engine_test
        self.other_engine_options = other_engine_options
        self.statistics = []

    def run(self):
        pbar = tqdm(range(len(self.other_engine_dirs)))
        for i in pbar:
            dir_engine_opponent = self.other_engine_dirs[i]
            engine_opponent = self.get_engine_name_from_dir(dir_engine_opponent)
            round_stats = RoundStatistics(self.seconds_move, engine_opponent)
            for j in range(self.number_matches):
                pbar.set_description("Playing against %s match number %d/%d" % (engine_opponent , j, self.number_matches) )   
                game = Game(self.seconds_move, self.dir_engine_test, dir_engine_opponent, self.other_engine_options)
                game_result = game.play()
                if game_result.result()=="1/2-1/2":
                    round_stats.draws.append(game_result)
                elif game_result.winner:
                    round_stats.wins+=1
                else:
                    round_stats.losses+=1
            self.statistics.append(round_stats)
    
    def get_engine_name_from_dir(self,dir_other_engine):
        return path.basename(path.normpath(dir_other_engine)).split('.')[0]

class RoundStatistics:
    def __init__(self, seconds_move, engine_opponent):
        self.seconds_move = seconds_move
        self.engine_opponent = engine_opponent
        self.wins = 0
        self.losses = 0
        self.draws = []

        


class Game:
    def __init__(self, seconds_move, dir_engine_test, dir_other_engine, other_engine_options):
        self.seconds_move = seconds_move
        self.dir_engine_to_test = dir_engine_test
        self.dir_other_engine = dir_other_engine
        self.other_engine_options = other_engine_options

    def play(self, starting_position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"):
        engine = chess.engine.SimpleEngine.popen_uci(self.dir_engine_to_test)
        engine2 = chess.engine.SimpleEngine.popen_uci(self.dir_other_engine)
        engine2.configure(self.other_engine_options)
        result = None
        engine_test_turn = bool(random.getrandbits(1))
        board = chess.Board(starting_position)
        outcome = board.outcome(claim_draw=True)
        while not outcome:
            result = self.next_move(engine, engine2, engine_test_turn, board)
            board.push(result.move)
            outcome = board.outcome(claim_draw=True)
        self.close_engines(engine, engine2)
        if not outcome.winner:
            return chess.Outcome(outcome.termination, outcome.winner == engine_test_turn)
        else:
            return outcome

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


t = Tournament(10,0.05, dirMyEngine, {},*strongEngines)
t.run()
plot_wins(t.statistics, t.number_matches)
