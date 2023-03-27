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

Mandatory arguments to long options are mandatory for short options too.

## Author

Written by Lucas Mior.

## License

`stiv` is released under the GPLv3+ license. See `LICENSE` for more details.

## See Also

- [link(2)](https://man7.org/linux/man-pages/man2/link.2.html)
- [symlink(2)](https://man7.org/linux/man-pages/man2/symlink.2.html)
