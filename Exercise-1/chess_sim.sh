#!/bin/bash

# Name: Evyatar Paz
# ID: 211788625

# it being said that the code will resive a pgn file as an argument.
#, with no need to check if the file exists

PGN_FILE=$1

# Check if the provided file exists
if [ ! -f "$PGN_FILE" ]; then
    echo "File not found!"
    exit 1
fi

# Extract metadata and moves from the PGN file
# it can also be done with head -n 12 $PGN_FILE because the metadata is always the first 12 lines
METADATA=$(grep -E '^\[.*\]$' "$PGN_FILE")

# Extract moves from the PGN file
MOVES=$(grep -vE '^\[.*\]$' "$PGN_FILE" | tr '\n' ' ')

# Print metadata with a newline at the end
echo "Metadata from PGN file:"
echo "$METADATA"
# optinal print a new line, work for me
echo ""

# Convert PGN moves to UCI format using parse_moves.py
UCI_MOVES=$(python3 parse_moves.py "$MOVES")

# Function to display the board
display_board() {
    local board=("${!1}")
    echo "  a b c d e f g h"
    for i in {8..1}; do
        echo -n "$i "
        for j in {0..7}; do
            echo -n "${board[$(( (8-i)*8 + j ))]} "
        done
        echo "$i"
    done
    echo "  a b c d e f g h"
}

# Initialize the chessboard
initialize_board() {
    board=(
        r n b q k b n r
        p p p p p p p p
        . . . . . . . .
        . . . . . . . .
        . . . . . . . .
        . . . . . . . .
        P P P P P P P P
        R N B Q K B N R
    )
}

# Function to update the board with a move
make_move() {
    local move=$1

    # Identify the piece being moved
    # get the starting and ending position of the move / get the column of the move
    local from_column=${move:0:1}
    local to_column=${move:2:1}

    # get the starting and ending row of the piece / get the row of the move
    local from_row=${move:1:1}
    local to_row=${move:3:1}
    
    # get the index of the starting and ending position of the move,
    # convert the chracter to a number that the array can hendle
    # get the initial and final position of the move by an array index.
    # using tr to map the letter to a number.
    local from_index=$(( (8-${from_row})*8 + $(echo ${from_column} | tr 'a-h' '0-7') ))
    local to_index=$(( (8-${to_row})*8 + $(echo ${to_column} | tr 'a-h' '0-7') ))
    
    # get the piece that is going to be moved
    local piece=${board[$from_index]}

    # Handle castling
    # extra / bonus
    if [[ ("$move" == "e1g1" && "$piece" == "K") || ("$move" == "e8g8" && "$piece" == "k") ]]; then
        # Kingside castling
        if [[ "$move" == "e1g1" ]]; then
            board[60]="."
            board[61]="R"
            board[62]="K"
            board[63]="."
        else
            board[4]="."
            board[5]="r"
            board[6]="k"
            board[7]="."
        fi
        return
    elif [[ ("$move" == "e1c1" && "$piece" == "K") || ("$move" == "e8c8" && "$piece" == "k") ]]; then
        # Queenside castling
        if [[ "$move" == "e1c1" ]]; then
            board[56]="."
            board[58]="K"
            board[59]="R"
            board[60]="."
        else
            board[0]="."
            board[2]="k"
            board[3]="r"
            board[4]="."
        fi
        return
    fi

    # Handle en passant
    # extra / bonus
    if [[ "${board[$from_index]}" == "P" && "${board[$to_index]}" == "." && "$from_column" != "$to_column" && "${from_row}" == "5" ]]; then
        board[$to_index]="P"
        board[$((to_index + 8))]="."
        board[$from_index]="."
        return
    elif [[ "${board[$from_index]}" == "p" && "${board[$to_index]}" == "." && "$from_column" != "$to_column" && "${from_row}" == "4" ]]; then
        board[$to_index]="p"
        board[$((to_index - 8))]="."
        board[$from_index]="."
        return
    fi

    # Handle pawn promotion
    # check if the move have 5 characters, if so, it is a promotion
    # the last character is the piece that the pawn is going to be promoted
    if [ ${#move} -eq 5 ]; then
        local promotion_piece=${move:4:1}
        # check if the promotion piece is a valid piece
        # =~ is a regolar expresion, it is used to check if the promotion piece is a valid piece
        if [[ "$promotion_piece" =~ [QRBNqrnb] ]]; then
            board[$to_index]=$promotion_piece
            board[$from_index]="."
            return
        fi
    fi

    # Handle normal move
    board[$to_index]=${board[$from_index]}
    board[$from_index]="."
}

# Initialize the board and variables
initialize_board
current_move=0
total_moves=$(echo "$UCI_MOVES" | wc -w)
# convert the moves to an array that seprates the moves by a space, so every move is an element of the array
move_array=($UCI_MOVES)

# print the current move and the total moves
echo "Move $current_move/$total_moves"
# display the board
display_board board[@]

# Loop to get user input
while true; do
    # get the user input
    echo -n "Press 'd' to move forward, 'a' to move back, 'w' to go to the start, 's' to go to the end, 'q' to quit:"
    read key
    echo ""
    valid_key=true
    case $key in
        d)
        # this charachter is used to go forward one move if possible
            # check if the current move is less than the total moves
            if [ $current_move -lt $total_moves ]; then
                # make the move
                make_move ${move_array[$current_move]}
                # increment the current move
                current_move=$((current_move + 1))
            else
                # if there are no more moves available
                echo "No more moves available."
                valid_key=false
            fi
            ;;
        a)
        # this charachter is used to go back one move if possible
            # the logic is to init the bora and loop through the moves until the current move - 1
            # and not to go back by one, that way it is easier to go back one move.
            # check if the current move is greater than 0
            if [ $current_move -gt 0 ]; then
                # init the borad
                initialize_board

                # loop through the moves antil the current move - 1
                for (( i=0; i<$current_move-1; i++ )); do
                    make_move ${move_array[$i]}
                done
                # decrement the current move
                current_move=$((current_move - 1))
            fi
            ;;
        w)
        # this charachter is used to go to the start of the game
            # init the board
            initialize_board
            # set the current move to 0
            current_move=0
            ;;
        s)
        # this charachter is used to go to the end of the game
            # init the board
            initialize_board
            # loop through the moves until the end of the game
            for (( i=0; i<$total_moves; i++ )); do
                make_move ${move_array[$i]}
            done
            # set the current move to the total moves
            current_move=$total_moves
            ;;
        q)
        # this charachter is used to quit the game
            echo "Exiting."
            echo "End of game."
            break
            ;;
        *)
        # if the key is not valid
            echo "Invalid key pressed: $key"
            valid_key=false
            ;;
    esac
    if [ "$valid_key" = true ]; then
        echo "Move $current_move/$total_moves"
        display_board board[@]
    fi
done
