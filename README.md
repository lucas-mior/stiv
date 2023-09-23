# stiv

`stiv` is a command-line tool for displaying image previews on the terminal using ueberzug.

## Usage
```
stiv FILE WIDTH HEIGHT [X Y]
```
Draw `FILE` with maximum `WIDTH` and `HEIGHT` on position (`X`, `Y`).

```
stiv -c|--clear [ 0 | 1 ]
```
Clear previous draws. If `WHICH` is `1`, clear previews. Else, clear others.

```
stiv -h|--help
```
Display help message.

## License

`stiv` is released under the GNU AFFERO GENERAL PUBLIC LICENSE.
