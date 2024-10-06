import chess
import chess.engine
import chess.pgn
import sys
import asyncio

import random
from tqdm import tqdm

async def engine_test(dir1, dir2, timeSeconds, engine2_options, dict_result_game):
    _, engine = await chess.engine.popen_uci(dir1)
    _, engine2 = await chess.engine.popen_uci(dir2)
    await engine2.configure(engine2_options)
    result = None
    turn = random.randrange(0,1)
    board = chess.Board()
    while not board.is_game_over():
        if turn == 0:
            if board.turn == chess.WHITE:
                result = await engine.play(board, chess.engine.Limit(timeSeconds))
            else:
                result = await engine2.play(board, chess.engine.Limit(timeSeconds))
        else:
            if board.turn == chess.BLACK:
                result = await engine.play(board, chess.engine.Limit(timeSeconds))
            else:
                result = await engine2.play(board, chess.engine.Limit(timeSeconds))
        """  """
        board.push(result.move)
    #game ended  
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

    await engine.quit()
    await engine2.quit()

import logging

# Enable debug logging.
# logging.basicConfig(level=logging.DEBUG)
    
dir1 = r"C:\Users\belle\OneDrive\Desktop\chess_thesis\BeautyEngine\play.exe"
dir2 = r"C:\Users\belle\OneDrive\Desktop\chess_thesis\BeautyEngine\stockfish-windows-x86-64-avx2.exe"
dir3 = r"C:\Users\belle\OneDrive\Desktop\chess_thesis\BeautyEngine\fairy-stockfish_x86-64.exe"
second_for_move = 0.1
episodes = 10
dict_result_game = {"win":0, "draw":0, "loss":0}
for i in tqdm(range(episodes), file=sys.stdout):
    asyncio.run(engine_test(dir1, dir2, second_for_move,{"Skill level":4}, dict_result_game))
print(dict_result_game)


