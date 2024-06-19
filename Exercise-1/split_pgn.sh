#!/bin/bash

# Name: Evyatar Paz
# ID: 211788625

# Check if the number of arguments is correct
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <source_pgn_file> <destination_directory>"
    exit 1
fi

# Assign the arguments to variables
input_file=$1
dest_dir=$2

# Check if the source file exists

if [ ! -f "$input_file" ]; then
    echo "Error: File '$input_file' does not exist."
    exit 1
fi

# Check if the destination directory exists

if [ ! -d "$dest_dir" ]; then
    mkdir -p "$dest_dir"
    echo "Created directory '$dest_dir'."
fi


# Function to save a game to a file
save_game() {
  local game_number="$1"
  local game_content="$2"
  local output_file="$dest_dir/$(basename "$input_file" .pgn)_$game_number.pgn"
  echo -e "$game_content" > "$output_file"
  echo "Saved game to $output_file"
}

# Initialize variables
game_number=1
game_content=""
in_game=false

# Read the file line by line
while IFS= read -r line || [[ "$line" == *$'\r' ]] || [[ -n "$line" ]]; do
    # Detect the start of a new game/ a change so it will work with linux and windows
    if [[ "$line" == *$'\r' ]]; then
        line="${line%$'\r'}"
    fi

    # check if the line is the start of a new game/ a new game starts with [Event
    if [[ $line == "[Event "* ]]; then
        # If we are already in a game, save it before starting the new one
        if $in_game; then
            save_game "$game_number" "$game_content"
            game_number=$((game_number + 1))
            game_content=""
        fi
        # Set the flag to indicate that we are in a game
        in_game=true
    fi
    # Accumulate game content
    game_content+="$line\n"
done < "$input_file"

# Save the last game if there's any game content left
if [[ -n "$game_content" ]]; then
    save_game "$game_number" "$game_content"
fi

echo "All games have been split and saved to '$dest_dir'."