import random
from players import Player, RandomPlayer, MaxEmptyCellsPlayer, HumanPlayer, MonteCarloPlayer, HeuristicPlayer, MinMaxPlayer, ExpectimaxPlayer
from board import Board
from interfaces import GUI2048, CLI2048

MOVE_COUNT_PROGRESS_PRINT = 500

class Game2048:
    def __init__(self, board: Board | None = None, player: Player | None = None, move_count: int = 0, interface=None):
        if board is None:
            self.board = Board()
        else:
            self.board = board

        while len(Board.get_empty_tiles(self.board.get_state())) > 14:
            self.add_random_tile()

        self.move_count: int = move_count

        if player:
            self.player = player
        else:
            self.player = RandomPlayer()
            print("Random player selected.")

        self.interface = interface

    def add_random_tile(self):
        empty_tiles = Board.get_empty_tiles(self.board.get_state())
        if not empty_tiles:
            return
        row, col = random.choice(empty_tiles)
        self.board.set_state(Board.set_tile(self.board.get_state(), row, col, 1 if random.random() < 0.9 else 2))

    def play_move(self):
        valid_actions = Board.get_valid_move_actions(self.board.get_state())
        if valid_actions:
            action, next_state = self.player.choose_action(valid_actions)
            self.board.set_state(next_state)
            self.add_random_tile()
            return True
        return False

    def get_score(self) -> int:
        return sum([2 ** tile for tile in self.board.get_state(unpack=True) if tile > 0])

    def reset(self):
        self.board = Board()
        self.move_count = 0
        self.add_random_tile()
        self.add_random_tile()

    def play_game(self):
        self.reset()
        while self.play_move():
            self.move_count += 1
            if self.interface:
                self.interface.update(state=self.board.get_state(), move_count=self.move_count)
            # if self.move_count % MOVE_COUNT_PROGRESS_PRINT == 0:
            #     print(f"Move count: {self.move_count}")
        return self.get_score(), self.board.get_state(), self.move_count

if __name__ == "__main__":
    import cProfile
    import pstats
    import argparse

    # Define a mapping from string to Player subclass.
    player_mapping = {
        RandomPlayer.__name__.lower(): RandomPlayer,
        HumanPlayer.__name__.lower(): HumanPlayer,
        MaxEmptyCellsPlayer.__name__.lower(): MaxEmptyCellsPlayer,
        MinMaxPlayer.__name__.lower(): MinMaxPlayer,
        HeuristicPlayer.__name__.lower(): HeuristicPlayer,
        MonteCarloPlayer.__name__.lower(): MonteCarloPlayer,
        ExpectimaxPlayer.__name__.lower(): ExpectimaxPlayer
    }

    parser = argparse.ArgumentParser(description="2048 Game")
    parser.add_argument("--profile_en", action="store_true", help="Enable profiling")
    parser.add_argument("-n", "--num_games", type=int, default=1000, help="Number of games to play")
    parser.add_argument("-p", "--player", type=str, default="RandomPlayer",
                        help=f"Player type {list(player_mapping.keys())}")
    args = parser.parse_args()

    if args.profile_en:
        profiler = cProfile.Profile()
        profiler.enable()

    # Choose the player from the mapping; use random player as default
    player_key = args.player.lower()
    if player_key in player_mapping:
        player_cls = player_mapping[player_key]
    else:
        print(f"Unknown player type '{args.player}'. Using random player instead.")
        player_cls = RandomPlayer

    game = Game2048(player=player_cls(), interface=CLI2048())
    best_score = 0
    best_state = None
    best_move_count = 0
    num_of_games = args.num_games
    print(f"Playing {num_of_games} games with {player_cls.__name__}...")

    for i in range(num_of_games):
        score, state, move_count = game.play_game()
        if score > best_score:
            best_score = score
            best_state = state
            best_move_count = move_count
        game.reset()

    if args.profile_en:
        profiler.disable()
        from datetime import datetime
        date_time = datetime.now().strftime("%Y%m%d_%H%M%S")
        with open(f'profile_results_{date_time}.txt', 'w') as f:
            stats = pstats.Stats(profiler, stream=f).sort_stats('cumulative')
            stats.print_stats()

    print("Best game:")
    CLI2048.pretty_print(best_state, best_score, best_move_count)
