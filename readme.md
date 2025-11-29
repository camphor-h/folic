# ------FoLic-----A modern terminal text editor-----

FoLic is designed to provide a more user-friendly text editor
in terminal. It adopts certain characteristics of modern text editors
to simplify terminal text modification. Though it still need some
improment, which will be done in future days.

FoLic is cross-platform and portable. So you can compile and run it at
any device. The only third-party requirement (currently) is ncurses.
However it doesn't highly depend on it. So you can do something to replace
the code depended on ncurses. So that it can be ported to more platform!



# ------Some basic control tutorial-----------------

You can use Esc Key to switch between toolbar and text edit area.
When in toolbar, use arrow keys to select different option.
The content after the name of operation in the option is the shortcut
key of control. For example:

-------------New (C-n)

(Operation name) (Shortcut key)

The "C" in the shortcut key stands Ctrl key, and the "S" stands Shift key.

Unfortunately, the shortkey can only be use when the focus is in the text
area currently. I'm sure that we will fix it.

You can use C-z to undo, and C-y to redo.

C-s to save file.

C-f to find text. C-r to replace.

C-g to go to target line.

Use Home key to move to the begin of line, the End key to the end of line.





# ------Compile and install--------------------------

Use "make" or "make release" to compile the release version.

Use "make debug" to compile the debug version.

Both of the two above method will put exec file in ./build directory.

Use "make install" to install to /usr/local




All Community contributions are welcome. Contact us if you want to help build!





