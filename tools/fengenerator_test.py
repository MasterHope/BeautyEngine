#from chess bot adventure...
import chess.pgn
import random
import math
import tqdm

pgn = open(r"C:\Users\belle\OneDrive\Desktop\chess_thesis\BeautyEngine\pgns\lichess_db_standard_rated_2015-03.pgn")
positions = []
for i in range(100000):
    game = chess.pgn.read_game(pgn)
    moves = game.mainline_moves()
    board = game.board()

    plyToPlay = math.floor(16+20*random.random()) & ~1
    numPly = 0
    for move in moves:
        board.push(move)
        numPly += 1
        if numPly == plyToPlay:
            fen = board.fen()

    numPieces = sum(fen.lower().count(char) for char in "rnbq")
    if numPly > plyToPlay + 20 * 2 and numPieces >= 10:
        positions.append(fen)


dir = r"C:\Users\belle\OneDrive\Desktop\chess_thesis\BeautyEngine\pgns\output.txt"
with open(dir, "w") as file:
    for string in positions:
        file.write(string + "\n")