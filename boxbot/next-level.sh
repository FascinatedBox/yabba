echo -n $1 | xclip -selection clipboard -i
notify-send "Level code ($1) added to clipboard."
