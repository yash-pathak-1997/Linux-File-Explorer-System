# File Explorer Application

The File Explorer application works in two modes: Normal mode and Command mode. This document provides an overview of the features and functionality of each mode.

The application display data starting from the top-left corner of the terminal window, line-by- line.

The application also handles text rendering if the terminal window is resized.

The last line of the display screen is to be used as a status bar.

## Normal Mode

Normal mode is the default mode of the application and is used for exploring the current directory and navigating the filesystem.

### Display

- Display a list of directories and files in the current folder.
- Each file in the directory is displayed on a new line with the following attributes:
  - File Name
  - File Size
  - Ownership (user and group)
  - Permissions
  - Last Modified
- Entries "." and ".." are displayed for the current and parent directory, respectively.
- The application handles scrolling using the up and down arrow keys. Users can navigate up and down in the file list using the corresponding arrow keys.

### Navigation

- Enter key opens directories and files:
  - Directory: Clears the screen and navigates into the directory, showing the directory contents.
  - File: Opens the file in the vi editor.
- Left arrow key goes back to the previously visited directory.
- Right arrow key goes forward to the next directory.
- Backspace key goes up one level in the directory structure.
- "h" key takes the user to the home folder.

## Command Mode

The Command mode is entered by pressing the colon (":") key. In this mode, users can enter different commands, and all commands appear in the status bar at the bottom.

### Commands

- Copy:
  - Syntax: `$ copy <source_file(s)> <destination_directory>`
- Move:
  - Syntax: `$ move <source_file(s)> <destination_directory>`
- Rename:
  - Syntax: `$ rename <old_filename> <new_filename>`
- Create File:
  - Syntax: `$ create_file <file_name> <destination_path>`
- Create Directory:
  - Syntax: `$ create_dir <dir_name> <destination_path>`
- Delete File:
  - Syntax: `$ delete_file <file_path>`
- Delete Directory:
  - Syntax: `$ delete_dir <dir_path>` (Recursively deletes all content inside the directory)
- Goto:
  - Syntax: `$ goto <directory_path>`
- Search:
  - Syntax: `$ search <file_name>` or `$ search <directory_name>`
  - Searches for a given file or folder under the current directory recursively, returning True or False based on existence.
- Pressing ESC key in Command Mode goes back to Normal Mode.
- Pressing "q" key in Normal Mode closes the application.
- Entering the "quit" command in Command Mode also closes the application.

Please note that copying and moving directories should be implemented while preserving ownership and permissions.

This File Explorer application is designed to provide a user-friendly interface for navigating, managing, and interacting with files and directories in both Normal and Command modes.
