# imageViewer

`imageViewer` is a small program where I try and understand simple rendering
of shapes and concepts that could be useful for future projects.

*CURRENT EXAMPLE:* The current example of `imageViewer` just displays a small
square that can be dragged around with the mouse, along with a line that renders
from the origin of the square to the mouse. 

Both the square and the line are made from scratch (even though SDL has ways to
render them already) and their implementations can be found in `main.cpp`.

## Dependencies

The only dependency of `imageViewer` is SDL2 for opening the window and handling
the pixels, which should already be installed in most GNU/Linux distributions.

If unsure of whether SDL2 is installed, run:
```
sdl2-config --version
```
if the command fails, install the SDL2 package from your distribution's package
manager.

## Compiling

For unoptimized, standard compilation just run:
```console
make
```

For optimized (-O2 in gcc) compilation run:
```console
make release
```

For compilation with debug information, run:
```console
make debug
```

## Usage

To actually launch `imageViewer`, run the following:
```console
./imageViewer [WINDOW_WIDTH] [WINDOW_HEIGHT]
```
where `[WINDOW_WIDTH]` `[WINDOW_HEIGHT]` are the dimensions of the desired window
size.

## Uninstalling

`imageViewer` is such a small program that it does not generate any stray files,
the easiest way to erase it from the system is to `make clean` and `rm -rf` the
cloned directory from your machine.
