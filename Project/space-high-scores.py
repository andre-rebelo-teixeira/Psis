import zmq
import score_message_pb2
import curses
from curses import wrapper
import os
import re

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

def update_display(stdscr, players, scores):
    h, w = stdscr.getmaxyx()
    
    # Clear the previous content
    for i in range(4, h-1):
        stdscr.addstr(i, 1, " " * (w-2))
    
    # Display players and scores in a table format
    row = 4
    for player, score in zip(players, scores):
        if player != ' ':
            table_row = f"| {player:^6} | {score:^5} |"
            stdscr.addstr(row, w//2 - len(table_row)//2, table_row)
            row += 1
    
    # Draw bottom border of the table
    if row > 4:
        bottom_border = "-" * len("| Player | Score |")
        stdscr.addstr(row, w//2 - len(bottom_border)//2, bottom_border)
    
    stdscr.refresh()

def main(stdscr):
    # Parse the PUBSUB_ADDRESS from the C header file
    PUBSUB_ADDRESS = parse_pubsub_address(header_file_path)

    # ZeroMQ socket setup
    context = zmq.Context()
    subscriber = context.socket(zmq.SUB)
    subscriber.connect(PUBSUB_ADDRESS)
    subscriber.setsockopt_string(zmq.SUBSCRIBE, "SCORE_UPDATE")

    initialize_curses(stdscr)
    
    previous_players = None
    previous_scores = None
    
    while True:
        try:
            # Receive message
            topic = subscriber.recv_string()
            message = subscriber.recv()
            
            # Parse the score update
            score_message = score_message_pb2.ScoreUpdate()
            score_message.ParseFromString(message)
            
            players = score_message.current_players
            scores = list(score_message.scores)  # Convert to list for comparison
            
            # Check if players or scores have changed
            if players != previous_players or scores != previous_scores:
                update_display(stdscr, players, scores)
                previous_players = players
                previous_scores = scores
            
        except KeyboardInterrupt:
            break

    # Clean up
    subscriber.close()
    context.term()

# Run the main function
if __name__ == "__main__":
    wrapper(main)