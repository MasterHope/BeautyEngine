import chess
import chess.engine
import chess.pgn
import sys
import asyncio

import random
from tqdm import tqdm

def engine_test(dir1, dir2, timeSeconds, engine2_options, dict_result_game):
    engine = chess.engine.SimpleEngine.popen_uci(dir1)
    engine2 = chess.engine.SimpleEngine.popen_uci(dir2)
    engine2.configure(engine2_options)
    result = None
    turn = random.randrange(0,1)
    board = chess.Board()
    while True:
        outcome = board.outcome()
        if outcome == chess.Outcome(chess.Termination.CHECKMATE, not board.turn) or outcome == chess.Outcome(chess.Termination.INSUFFICIENT_MATERIAL, None) \
            or outcome == chess.Outcome(chess.Termination.STALEMATE, None) or board.is_fifty_moves() or board.can_claim_threefold_repetition(): break
        if turn == 0:
            if board.turn == chess.WHITE:
                result = engine.play(board, chess.engine.Limit(timeSeconds))
            else:
                result = engine2.play(board, chess.engine.Limit(timeSeconds))
        else:
            if board.turn == chess.BLACK:
                result = engine.play(board, chess.engine.Limit(timeSeconds))
            else:
                result = engine2.play(board, chess.engine.Limit(timeSeconds))
        """  """
        board.push(result.move)
    #game ended  
    if board.is_fifty_moves() or board.can_claim_threefold_repetition() :
        dict_result_game["draw"] += 1
    else:
        player_to_win = board.outcome().winner
        if turn == 0:
            if player_to_win == chess.WHITE:
                dict_result_game["win"] += 1
            elif player_to_win == chess.BLACK:
                dict_result_game["loss"] += 1
            else:
                dict_result_game["draw"] += 1
        else:
            if player_to_win == chess.BLACK:
                dict_result_game["win"] += 1
            elif player_to_win == chess.WHITE:
                dict_result_game["loss"] += 1
            else:
                dict_result_game["draw"] += 1

    engine.quit()
    engine2.quit()

import logging

# Enable debug logging.
logging.basicConfig(level=logging.DEBUG)
    
dir1 = r"C:\Users\belle\OneDrive\Desktop\chess_thesis\BeautyEngine\play.exe"
dir2 = r"C:\Users\belle\OneDrive\Desktop\chess_thesis\BeautyEngine\stockfish-windows-x86-64-avx2.exe"
dir3 = r"C:\Users\belle\OneDrive\Desktop\chess_thesis\BeautyEngine\fairy-stockfish_x86-64.exe"
second_for_move = 0.1
episodes = 40
dict_result_game = {"win":0, "draw":0, "loss":0}
for i in tqdm(range(episodes), file=sys.stdout):
    engine_test(dir1, dir2, second_for_move,{"Skill level":4}, dict_result_game)
print(dict_result_game)


