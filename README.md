# LinuxKernelFileManager

![Preview failed to load!](https://github.com/aatanas/LinuxKernelFileManager/blob/main/preview.png?raw=true)

The goal of this project is to provide two tools for Linux - file explorer and clipboard.
It is written in C and assembly within the Linux kernel. The tool also works while
some user application is running. It supports the following functionalities:

- One of the two tools is displayed at one point. The F2 key selects which tool is displayed.
The tool is displayed in the upper right corner of the screen.
- The Explorer tool allows browsing the files on the disk and copying the path to the selected one
file to the terminal.
- The Clipboard tool allows you to record strings and copy them to the terminal.


#### Display of the tools
Both tools will be displayed in the upper right corner of the screen and are located in the
a rectangle made up of the '#' character. The dimension of this "window" is 20x10 characters of useful space, i.e. 22x12 including frame.
Both tools display text centered.
The explorer tool uses different colors for different types of files on the disk:

- Regular files - standard gray color
- Directories - light blue
- Executable files - green
- Character devices - yellow

With the explorer tool, the first 10 items in the directory are displayed.
In both tools, the currently selected item is marked with a changed color
backgrounds.
The top of the "window" shows the current path of the explorer tool, and the text "clipboard" in
clipboard tool.
At the start of the system, no tools are displayed. After pressing the F2 key for the first time,
the explorer tool is shown, placed in the root ("/") directory. Tool change is activated
by pressing the F2 key.

#### Working with explorer
Explorer commands are entered via the keyboard, and they are:
|No.| Key| Behavior|
|--- |---|---|
|1. |F1 |Activates arrows and space|
|2. |F2 |Changes the currently active tool from explorer to clipboard|
|3. |Up arrow |Previous file (only works with F1 pressed)|
|4. |Down arrow |Next file (only works with F1 pressed)|
|5. |Left arrow |Parent directory (only works with F1 pressed)|
|6. |Right arrow |Enter directory (only works with F1 pressed)|
|7. |Space |Copies the path to the selected item in the terminal (works only with F1 pressed)|

Arrows are used to navigate the file system, and space is used to copy the path to
selected files to the terminal, but they function in this way only while
holding F1. In case F1 is pressed, arrows and space only perform this function, their normal is disabled.
In case F1 is not pressed, the arrows and space are displayed
they behave normally. If we are on the last item, the down arrow takes us to the first item,
if we are on the first item, the up arrow takes us to the last item.

The F keys do not perform their normal operation of printing the letters A / B / C on
the terminal.

The right arrow only works if a directory is selected, and in that case, it becomes that
a new active directory, and its contents are printed in the explorer. If any is selected
file, an error is printed.

The left arrow takes us back to the parent directory (".."). If we are in the root
directory, the left arrow has no effect. Both of these arrows clear the explorer block
before writing the contents of the new directory.

Space copies the full path to the currently selected file/directory to
terminal. It also works when we enter commands for the operating system (shell) and
when we are in the user application.

#### Working with the clipboard
Commands for the clipboard are entered via the keyboard, and they are:
|No.| Key| Behavior|
|---|---|---|
|1. |F1 |Activates arrows and space|
|2. |F2 |Changes the currently active tool from clipboard to explorer|
|3. |F3 |Starts / finishes recording a new item in the clipboard|
|4. |Up arrow |Previous item (only works with F1 pressed)|
|5. |Down arrow |Next item (only works with F1 pressed)|
|6. |Space |Copies the current item to the terminal (only works with F1 pressed)|

Arrows are used to select items within the clipboard, and space is used to copy
those items into the terminal, but they only function in this way while holding down F1.
If F1 is pressed, the arrows and space only perform this function, not their own
normal function. In case F1 is not pressed, arrows and spacebar behave
normally. If we are on the last item, the down arrow takes us to the first item, and if we are on
the first item, the up arrow takes us to the last item.

The F keys do not perform their normal operation of printing the letters A / B / C on
the terminal.

Space copies the currently selected item to the terminal. This works too
when we enter commands for the operating system (shell) and when we are in the user interface
application.

The F3 key starts writing an item to the clipboard. The currently selected item is not deleted
at the beginning of the entry, work continues from the end of the item. Backspace key enables
erasing the text inside the clipboard, if it exists. If there is no text, backspace has no effect.
Pressing the F3 key again interrupts the clipboard entry. While entering data into the clipboard
it is not possible to interact with the terminal.
