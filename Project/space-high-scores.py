import zmq
import score_message_pb2
import curses
from curses import wrapper
import os
import re
from collections import defaultdict

# utils.h file location
current_dir = os.path.dirname(os.path.abspath(__file__))
header_file_path = os.path.join(current_dir, 'utils.h')

def parse_pubsub_address(header_file_path):
    with open(header_file_path, 'r') as file:
        content = file.read()
        match = re.search(r'#define\s+PUBSUB_ADDRESS\s+"([^"]+)"', content)
        if match:
            return match.group(1)
        else:
            raise ValueError("PUBSUB_ADDRESS not found in the header file")

def initialize_curses(stdscr):
    curses.curs_set(0)
    stdscr.clear()
    draw_border(stdscr)
    stdscr.refresh()

def draw_border(stdscr):
    h, w = stdscr.getmaxyx()
    stdscr.box()
    stdscr.addstr(0, w//2 - 4, " SpcInvdrs ")
    stdscr.refresh()

def draw_grid_border(stdscr, start_y, start_x, height, width):
    # Draw horizontal lines
    stdscr.addch(start_y - 1, start_x - 1, '┌')
    stdscr.addch(start_y - 1, start_x + width, '┐')
    stdscr.addch(start_y + height, start_x - 1, '└')
    stdscr.addch(start_y + height, start_x + width, '┘')
    
    for x in range(start_x, start_x + width):
        stdscr.addch(start_y - 1, x, '─')
        stdscr.addch(start_y + height, x, '─')
    
    # Draw vertical lines
    for y in range(start_y, start_y + height):
        stdscr.addch(y, start_x - 1, '│')
        stdscr.addch(y, start_x + width, '│')

def draw_scores_header(stdscr, start_x):
    header = "| Player | Score | High Score |"
    stdscr.addstr(4, start_x, " SCORE BOARD ")
    stdscr.addstr(5, start_x, header)
    stdscr.addstr(6, start_x, "-" * len(header))

def update_display(stdscr, message, high_scores):
    h, w = stdscr.getmaxyx()
    
    # Clear the previous content (except border)
    for i in range(1, h-1):
        stdscr.addstr(i, 1, " " * (w-2))

    # Display status messages at the top
    server_status = f"Server Shutdown: {'Yes' if message.server_shutdown else 'No'}"
    game_status = f"Game Over: {'Yes' if message.game_over else 'No'}"
    stdscr.addstr(1, w//2 - len(server_status)//2, server_status)
    stdscr.addstr(2, w//2 - len(game_status)//2, game_status)

    # Calculate positions
    grid_width = len(message.grid[0])
    scores_width = 25
    total_width = grid_width + scores_width + 4
    start_x = (w - total_width) // 2

    # Draw grid on the left side
    grid_start_x = start_x
    grid_start_y = 5
    stdscr.addstr(grid_start_y - 1, grid_start_x, "GAME GRID")
    
    # Draw grid border
    draw_grid_border(stdscr, grid_start_y, grid_start_x, len(message.grid), grid_width)
    
    # Draw grid content
    for i, grid_row in enumerate(message.grid):
        stdscr.addstr(grid_start_y + i, grid_start_x, grid_row)

    # Draw scores on the right side
    scores_start_x = grid_start_x + grid_width + 4
    draw_scores_header(stdscr, scores_start_x)
    
    # Display players, scores, and high scores
    for i, score in enumerate(message.scores):
        if i < len(message.current_players):
            player = message.current_players[i]
            if player != ' ':
                table_row = f"| {player:^6} | {score:^5} | {high_scores[player]:^10} |"
                stdscr.addstr(6 + i, scores_start_x, table_row)

    # Draw bottom border of the scores table
    if any(p != ' ' for p in message.current_players):
        bottom_border = "-" * len("| Player | Score | High Score |")
        stdscr.addstr(6 + len(message.scores), scores_start_x, bottom_border)

    stdscr.refresh()

def main(stdscr):
    # Parse the PUBSUB_ADDRESS from the C header file
    PUBSUB_ADDRESS = parse_pubsub_address(header_file_path)

    # ZeroMQ socket setup
    context = zmq.Context()
    subscriber = context.socket(zmq.SUB)
    subscriber.connect(PUBSUB_ADDRESS)
    subscriber.setsockopt_string(zmq.SUBSCRIBE, "PB_UPDATE")

    initialize_curses(stdscr)
    
    high_scores = defaultdict(int)
    
    while True:
        # Receive message
        topic = subscriber.recv_string()
        message = subscriber.recv()

        # Parse the display update message
        display_message = score_message_pb2.DisplayUpdateMessage()
        display_message.ParseFromString(message)

        # Update high scores
        for player, score in zip(display_message.current_players, display_message.scores):
            if player != ' ':
                high_scores[player] = max(high_scores[player], score)

        # Update the display
        update_display(stdscr, display_message, high_scores)

    # Clean up
    subscriber.close()
    context.term()

# Run the main function
if __name__ == "__main__":
    wrapper(main)