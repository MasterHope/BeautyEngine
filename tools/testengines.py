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
second_for_move = 0.1
episodes = 10

class Game:
    def __init__(self, time_seconds, dir_engine_test, dir_other_engine, other_engine_options):
        self.time_seconds = time_seconds
        self.dir_engine_to_test = dir_engine_test
        self.dir_other_engine = dir_other_engine
        self.other_engine_name = path.basename(path.normpath(dir_other_engine))
        self.other_engine_options = other_engine_options
        self.win = False
        self.draw = False
        self.lose = False

    def play(self, starting_position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"):
        engine = chess.engine.SimpleEngine.popen_uci(self.dir_engine_to_test)
        engine2 = chess.engine.SimpleEngine.popen_uci(self.dir_other_engine)
        engine2.configure(self.other_engine_options)
        result = None
        turn = random.randrange(0,1)
        board = chess.Board(starting_position)
        while True:
            outcome = board.outcome()
            if self.game_ended(board, outcome): break
            if turn == 0:
                if board.turn == chess.WHITE:
                    result = engine.play(board, chess.engine.Limit(self.time_seconds))
                else:
                    result = engine2.play(board, chess.engine.Limit(self.time_seconds))
            else:
                if board.turn == chess.BLACK:
                    result = engine.play(board, chess.engine.Limit(self.time_seconds))
                else:
                    result = engine2.play(board, chess.engine.Limit(self.time_seconds))
            """  """
            board.push(result.move)
        #game ended  
        if board.is_fifty_moves() :
            self.draw = True
            engine.quit()
            engine2.quit()
            return chess.Outcome(chess.Termination.FIFTY_MOVES)
        if board.can_claim_threefold_repetition() :
            self.draw = True
            engine.quit()
            engine2.quit()
            return chess.Outcome(chess.Termination.THREEFOLD_REPETITION)
        player_to_win = outcome.winner
        if turn == 0:
            if player_to_win == chess.WHITE:
                self.win = True
            elif player_to_win == chess.BLACK:
                self.lose = True
            else:
                self.draw = True
        else:
            if player_to_win == chess.BLACK:
                self.win = True
            elif player_to_win == chess.WHITE:
                self.lose = True
            else:
                self.draw = True
        engine.quit()
        engine2.quit()
        return chess.Outcome(outcome.termination, self.win)

    def game_ended(self, board, outcome):
        return outcome == chess.Outcome(chess.Termination.CHECKMATE, not board.turn) or outcome == chess.Outcome(chess.Termination.INSUFFICIENT_MATERIAL, None) \
                or outcome == chess.Outcome(chess.Termination.STALEMATE, None) or board.is_fifty_moves() or board.can_claim_threefold_repetition()
        


game = Game(0.1, dirMyEngine, dirFairyStockfish, {"Skill level":4})
end = game.play()
print(end)
