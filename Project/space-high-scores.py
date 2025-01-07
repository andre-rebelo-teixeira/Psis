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
    draw_table_header(stdscr)
    stdscr.refresh()

def draw_border(stdscr):
    h, w = stdscr.getmaxyx()
    stdscr.box()
    stdscr.addstr(0, w//2 - 6, " SCORE BOARD ")
    stdscr.refresh()

def draw_table_header(stdscr):
    h, w = stdscr.getmaxyx()
    header = "| Player | Score |"
    stdscr.addstr(2, w//2 - len(header)//2, header)
    stdscr.addstr(3, w//2 - len(header)//2, "-" * len(header))

def update_display(stdscr, message):
    h, w = stdscr.getmaxyx()

    # Clear the previous content
    for i in range(4, h-2):
        stdscr.addstr(i, 1, " " * (w-2))

    # Display server status
    server_status = f"Server Shutdown: {'Yes' if message.server_shutdown else 'No'}"
    game_status = f"Game Over: {'Yes' if message.game_over else 'No'}"
    stdscr.addstr(4, w//2 - len(server_status)//2, server_status)
    stdscr.addstr(5, w//2 - len(game_status)//2, game_status)

    # Display the grid
    row = 7
    for grid_row in message.grid:
        stdscr.addstr(row, w//2 - len(grid_row)//2, grid_row)
        row += 1

    # Display players and scores
    row += 1
    for i, score in enumerate(message.scores):
        if i < len(message.current_players):
            player = message.current_players[i]
            table_row = f"| {player:^6} | {score:^5} |"
            stdscr.addstr(row, w//2 - len(table_row)//2, table_row)
            row += 1

    stdscr.refresh()

def main(stdscr):
    # Parse the PUBSUB_ADDRESS from the C header file
    PUBSUB_ADDRESS = parse_pubsub_address(header_file_path)

    # ZeroMQ socket setup
    context = zmq.Context()
    subscriber = context.socket(zmq.SUB)
    subscriber.connect(PUBSUB_ADDRESS)
    subscriber.setsockopt_string(zmq.SUBSCRIBE, "UPDATE")

    initialize_curses(stdscr)

    while True:
        # Receive message
        topic = subscriber.recv_string()
        message = subscriber.recv()

        # Parse the display update message
        display_message = score_message_pb2.DisplayUpdateMessage()
        display_message.ParseFromString(message)

        # Update the display
        update_display(stdscr, display_message)

    # Clean up
    subscriber.close()
    context.term()

# Run the main function
if __name__ == "__main__":
    wrapper(main)
