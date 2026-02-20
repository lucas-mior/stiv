# stiv

`stiv_draw` is a command-line tool for displaying image previews on the terminal using ueberzug.
`stiv_clear` is a command-line tool for clearing image previews on the terminal using ueberzug.
`fifo_write_nonblock` is a command-line tool for writing to fifos in non blocking mode. If the fifo can't be opened right away, the program exits.

## Usage
```sh
# Draw `FILE` with maximum `WIDTH` and `HEIGHT` on position (`X`, `Y`).
$ stiv_draw FILE WIDTH HEIGHT [X Y]
# Clear previous drawing if no arguments or if OLD_FILE was an image and FILE is not
$ stiv_clear [FILE WIDTH HEIGHT X Y OLD_FILE]
# Send STRING to FIFO in non blocking mode.
$ fifo_write_nonblock string=<STRING> fifo=<FIFO>
```


`stiv` is released under the GNU AFFERO GENERAL PUBLIC LICENSE.
