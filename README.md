# stiv

`stiv_draw` is a command-line tool for displaying image previews on the terminal using ueberzug.
`stiv_clear` is a command-line tool for clearing image previews on the terminal using ueberzug. It also supports `pause` (clear and suppress previews during a shell) and `resume`. Draw and clear commands share a FIFO lock and reject requests superseded by a later request.
`fifo_write_nonblock` is a command-line tool for writing to fifos in non blocking mode. If the fifo can't be opened right away, the program exits.

## Usage
```sh
# Draw `FILE` with maximum `WIDTH` and `HEIGHT` on position (`X`, `Y`).
$ stiv_draw FILE WIDTH HEIGHT [X Y]
# Clear the overlay (the lf cleaner arguments are accepted but ignored)
$ stiv_clear [PREVIOUS_FILE WIDTH HEIGHT X Y NEXT_FILE]
$ stiv_clear pause
$ stiv_clear resume
# Send STRING to FIFO in non blocking mode.
$ fifo_write_nonblock string=<STRING> fifo=<FIFO>
```


`stiv` is released under the GNU AFFERO GENERAL PUBLIC LICENSE.
