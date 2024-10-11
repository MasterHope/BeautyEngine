import chess
import chess.engine
import chess.pgn
import sys
import asyncio

import random
from tqdm import tqdm
from os import listdir, path

#used dir
dirMyEngine = r"C:\Users\belle\OneDrive\Desktop\chess_thesis\BeautyEngine\BeautyEngine.exe"
dirStockfish = r"C:\Users\belle\OneDrive\Desktop\chess_thesis\BeautyEngine\engines\stockfish-windows-x86-64-avx2.exe"
dirFairyStockfish = r"C:\Users\belle\OneDrive\Desktop\chess_thesis\BeautyEngine\engines\fairy-stockfish_x86-64.exe"
dirLC0 = r"C:\Users\belle\OneDrive\Desktop\chess_thesis\BeautyEngine\engines\lc0-v0.31.1-windows-cpu-dnnl\lc0.exe"
strongEngines = [dirStockfish, dirFairyStockfish, dirLC0]
fairEnginesDir = [f for f in listdir(r"C:\Users\belle\OneDrive\Desktop\chess_thesis\BeautyEngine\engines\fair")]


class Tournament:
    def __init__(self, number_matches, time_seconds_arr, dir_engine_test, *other_engine_dirs):
        self.number_matches = number_matches
        self.time_seconds_arr = time_seconds_arr
        self.other_engine_dirs = other_engine_dirs


class Game:
    def __init__(self, seconds_move, dir_engine_test, dir_other_engine, other_engine_options):
        self.seconds_move = seconds_move
        self.dir_engine_to_test = dir_engine_test
        self.dir_other_engine = dir_other_engine
        self.other_engine_name = path.basename(path.normpath(dir_other_engine))
        self.other_engine_options = other_engine_options

    def play(self, starting_position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"):
        engine = chess.engine.SimpleEngine.popen_uci(self.dir_engine_to_test)
        engine2 = chess.engine.SimpleEngine.popen_uci(self.dir_other_engine)
        engine2.configure(self.other_engine_options)
        result = None
        engine_test_turn = bool(random.getrandbits(1))
        board = chess.Board(starting_position)
        outcome = board.outcome()
        while not self.game_ended(board, outcome):
            result = self.next_move(engine, engine2, engine_test_turn, board)
            board.push(result.move)
            outcome = board.outcome()
        """ game ended  
        these two conditions are treated in a separate way... (they are not checked from engine because we should claim for obtaining that.)
        we assume that if condition met, then we reached end of the game. """
        if board.is_fifty_moves() :
            self.close_engines(engine, engine2)
            return chess.Outcome(chess.Termination.FIFTY_MOVES, None)
        if board.can_claim_threefold_repetition() :
            self.close_engines(engine, engine2)
            return chess.Outcome(chess.Termination.THREEFOLD_REPETITION, None)
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

    def game_ended(self, board, outcome):
        return outcome == chess.Outcome(chess.Termination.CHECKMATE, not board.turn) or outcome == chess.Outcome(chess.Termination.INSUFFICIENT_MATERIAL, None) \
                or outcome == chess.Outcome(chess.Termination.STALEMATE, None) or board.is_fifty_moves() or board.can_claim_threefold_repetition()
        


game = Game(0.1, dirMyEngine, dirFairyStockfish, {"Skill level":4})
end = game.play()
print(end)
