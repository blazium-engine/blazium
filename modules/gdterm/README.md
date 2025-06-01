# GDTerm

GDTerm the Godot In-Editor Terminal

## Description

This project was created to address the needs of developers who are working on their game in the Godot editor
and also need to perform actions at the command line (like start and stop servers, monitor logs, etc.).

It might also help people who use one of the [alternative programming languages](https://github.com/Godot-Languages-Support/godot-lang-support),
as it's possible to run Helix, Neovim, or Emacs within gdterm.

In other words this
is for those of us who like to keep the Godot Editor in full-screen and not shuffle their windows when needing to do command
line tasks.

It provides the following features:

* Multiple terminals in main screen area (the same place as the 2D/3D/Script/Assets go).
* Pseudo-terminal interface to default shell
* Emulates an ANSI terminal (16 colors)
* Independent scrollback in each terminal
* Copy and paste in each terminal
* Works fine with typical command line tools: vi, top, tail
* Supports unicode with caveats
* Supports Linux and Windows and Mac

## Getting Started

### Dependencies

* Developed against Godot 4.3-stable
* Godot supported Linux, Windows, or Mac distribution
* Environment suitable for compiling an extension (if compiling from source)
  * See: https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/gdextension_cpp_example.html

### Installing

Source is on GitHub and there is the latest stable version in the GD Asset Lib

To use the pre-compiled binaries:

* git clone http://github.com/markeel/gdterm
* copy addons directory to the Godot project you need this extension

To compile from source instead of using the pre-compiled library:

* git clone http://github.com/markeel/gdterm
* cd gdterm
* git submodule update --init --recursive
* scons
* copy addons directory to the Godot project you need this extension

### Executing program

* Within your Godot project
* From menubar: Project->Project Settings...
* Click Plugins tab
* Select Enabled checkbox next to "GDTerm"

### Using the terminal

A default instance of a terminal will be available when selecting the "Terminal" button

#### Copy and Paste

The mouse can be used to select text:
* single-click and drag to highlight characters
* double-click and drag to highlight words
* triple-click and drag to highlight lines

The context menu (Right-Click) includes a copy and paste which goes to the system clipboard.

#### New and Close

A new terminal can be created by adding one above, below, left, or right of the window the
context menu is in.

A terminal can be closed from the context menu as well.

#### Restart

A restart will clear the window and start a new terminal session.  This is also the way to
get a terminal session going again if the shell being used by this terminal has been exited.

## Settings

The GDTerm plugin has the following settings available under Editor->Editor Settings...

The settings are in the Gdterm section and are as follows:

- Layout: This indicates where the terminal should reside within the Godot editor.

  - main - The terminal will be in the main window (same place as 2D, 3D, Script, and AssetLib)
           this option gives the most room for doing tasks like editing configuration files
           or examining listings.

  - bottom - The terminal will be in the bottom panel area (where Output, Debugger, Audio, etc.)
             are located.

  - both - There will be 2 terminals (one in the main area, and the other in the bottom panel)

- Theme: This has one of the 3 themes supported, and changes the colors, foreground and background
         for all the terminal windows.

- Initial Commands: This is zero, one, or more commands that will be executed when a terminal window
                    starts up.  They are only executed when initially created or a "restart" is
                    performed on the window.

- Send Alt Meta as Esc: If checked, pressing the Alt or Meta keys with another character will send
                        an ESC to the application in front of the other character being pressed.
                        This is important for applications like Emacs that expect this behavior.

- Font: If not empty, it will use the font at the specified path as the font in the terminal window,
        it should always be a mono spaced font, or the terminal will not work properly

- Font Size: This defaults to 14 but can be adjusted to be any size.  This may be important for
             Mac with Retina display

## Unicode and UTF-8 encoding

The GDTerm plugin expects all input and output to be UTF-8.  This is the default for most Linux
distributions, if it isn't in your distribution, the easiest option may be to change your locale.
The underlying Windows pseudo-terminal also uses UTF-8 so you do not have to do anything
special in a Windows environment.

Having said that, not all programs in Windows are Unicode aware and you will likely see strange characters
since those programs (like "type") don't understand Unicode or UTF-8.   It is generally safer to use
Powershell on the Windows platform.

The GDTerm plugin attempts to handle special unicode characters that are composed from multiple unicode code points,
but it is limited by the available fonts to render those characters and how the applications
you are actually running treat those characters.

For instance the way that the bash shell behaves and the way that vi behave are not entirely consistent
when using composed unicode characters.  If you just have a few, it will probably
be sufficient.  For better or worse, most of the other terminals I compared GDTerm with also have issues
with those kinds of characters.

## Help

This is a brand new extension, so if you run into problems, create an issue, for general
questions you can use the Discussions tab.

## Authors

markeel

## Version History

* 1.0.1
    * Use Cmd key instead of Ctrl on Mac OS
    * Allow Font and Font size to be overridden in Editor settings

* 1.0
    * Support Mac OS
    * Fixes for background color issues
    * Allow Alt or Meta to be sent as as Esc for Emacs

* 0.99
    * Support for Editor Settings
    * Better support for Unicode

* 0.95
    * Support for Windows

* 0.9
    * Initial Release

## License

This project is licensed under the MIT License - see the LICENSE.md file for details

## Acknowledgments

### Godot

You wouldn't be here if you weren't already using the Godot Game Engine: See [Godot Engine](https://godotengine.org/)

### ANSI Terminal code logic

The ANSI code interpretation is built using libtmt, but slightly extended to support
scrollback.  See [libtmt](https://github.com/deadpixi/libtmt)

### Fonts

The fonts used are Source Code Pro from [Google Fonts](https://fonts.google.com/specimen/Source+Code+Pro), With license as follows:

Copyright 2010, 2012 Adobe Systems Incorporated (http://www.adobe.com/), with Reserved Font Name 'Source'. All Rights Reserved. Source is a trademark of Adobe Systems Incorporated in the United States and/or other countries.

This Font Software is licensed under the SIL Open Font License, Version 1.1 . This license is available with a FAQ at: https://openfontlicense.org

SIL OPEN FONT LICENSE Version 1.1 - 26 February 2007

### Sound

The bell sound:

```
Copyright: Dr. Richard Boulanger et al
URL: http://www.archive.org/details/Berklee44v12
License: CC-BY Attribution 3.0 Unported
```
