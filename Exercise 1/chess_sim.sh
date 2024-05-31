#!/bin/bash

# Check if a PGN file is provided as an argument
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <pgn-file>"
    exit 1
fi

PGN_FILE=$1

# Check if the provided file exists
if [ ! -f "$PGN_FILE" ]; then
    echo "File not found!"
    exit 1
fi

# Extract metadata and moves from the PGN file
METADATA=$(grep -E '^\[.*\]$' "$PGN_FILE")
MOVES=$(grep -vE '^\[.*\]$' "$PGN_FILE" | tr '\n' ' ')

# Print metadata with a newline at the end
echo "Metadata from PGN file:"
echo "$METADATA"
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
        echo " $i"
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

    # Handle castling
    if [[ "$move" == "e1g1" || "$move" == "e8g8" ]]; then
        # Kingside castling
        if [[ "$move" == "e1g1" ]]; then
            board[62]="K"
            board[60]="."
            board[61]="R"
            board[63]="."
        else
            board[6]="k"
            board[4]="."
            board[5]="r"
            board[7]="."
        fi
        return
    elif [[ "$move" == "e1c1" || "$move" == "e8c8" ]]; then
        # Queenside castling
        if [[ "$move" == "e1c1" ]]; then
            board[58]="K"
            board[60]="."
            board[59]="R"
            board[56]="."
        else
            board[2]="k"
            board[4]="."
            board[3]="r"
            board[0]="."
        fi
        return
    fi

    local from_file=${move:0:1}
    local from_rank=${move:1:1}
    local to_file=${move:2:1}
    local to_rank=${move:3:1}
    
    local from_index=$(( (8-${from_rank})*8 + $(echo ${from_file} | tr 'a-h' '0-7') ))
    local to_index=$(( (8-${to_rank})*8 + $(echo ${to_file} | tr 'a-h' '0-7') ))

    # Handle en passant
    if [[ "${board[$from_index]}" == "P" && "${board[$to_index]}" == "." && "$from_file" != "$to_file" && "${from_rank}" == "5" ]]; then
        board[$to_index]="P"
        board[$((to_index + 8))]="."
        board[$from_index]="."
        return
    elif [[ "${board[$from_index]}" == "p" && "${board[$to_index]}" == "." && "$from_file" != "$to_file" && "${from_rank}" == "4" ]]; then
        board[$to_index]="p"
        board[$((to_index - 8))]="."
        board[$from_index]="."
        return
    fi

    # Handle pawn promotion
    if [ ${#move} -eq 5 ]; then
        local promotion_piece=${move:4:1}
        if [[ "$promotion_piece" =~ [QRBNqrbn] ]]; then
            board[$to_index]=$promotion_piece
            board[$from_index]="."
            return
        fi
    fi

    board[$to_index]=${board[$from_index]}
    board[$from_index]="."
}

# Main simulation loop
initialize_board
current_move=0
total_moves=$(echo "$UCI_MOVES" | wc -w)
move_array=($UCI_MOVES)

while true; do
    echo "Move $current_move/$total_moves"
    display_board board[@]
    echo -n "Press 'd' to move forward, 'a' to move back, 'w' to go to the start, 's' to go to the end, 'q' to quit: "
    read -n 1 key
    echo ""
    case $key in
        d)
            if [ $current_move -lt $total_moves ]; then
                make_move ${move_array[$current_move]}
                current_move=$((current_move + 1))
            else
                echo "No more moves available."
            fi
            ;;
        a)
            if [ $current_move -gt 0 ]; then
                initialize_board
                for (( i=0; i<$current_move-1; i++ )); do
                    make_move ${move_array[$i]}
                done
                current_move=$((current_move - 1))
            fi
            ;;
        w)
            initialize_board
            current_move=0
            ;;
        s)
            initialize_board
            for (( i=0; i<$total_moves; i++ )); do
                make_move ${move_array[$i]}
            done
            current_move=$total_moves
            ;;
        q)
            echo "Exiting."
            echo "End of game."
            break
            ;;
        *)
            echo "Invalid key!"
            ;;
    esac
done
