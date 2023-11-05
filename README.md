# Linux-File-Explorer-System

File Explorer works in two modes :-   

1. Normal mode (default mode) - used to explore the current directory and navigate the filesystem 
1. Command mode - used to enter shell commands   

Root of application is same as the system, and the home of the application is same as home of the current user.   

The application display data starting from the top-left corner of the terminal window, line-by- line. 

The application also handles text rendering if the terminal window is resized. 

The last line of the display screen is to be used as a status bar.   

Normal mode: 

Normal mode is the default mode of your application. It should have the following functionalities -   

1. Display a list of directories and files in the current folder   
1. Every file in the directory should be displayed on a new line with the following    

attributes for each file -   

1. File Name  
1. File Size   
1. Ownership (user and group) and Permissions   
4. Last modified   
2. The file explorer should show entries “.” and “..” for current and parent directory   respectively   
2. The file explorer should handle scrolling using the up and down arrow keys.   
2. User should be able to navigate up and down in the file list using the corresponding up and down arrow keys. The up and down arrow keys should also handle scrolling during vertical overflow.  
2. Open directories and files When enter key is pressed - 
1. Directory - Clear the screen and navigate into the directory and show 

the directory contents as specified in point 1   

2. File - Open the file in vi editor   
3. Traversal 
1. Go back - Left arrow key should take the user to the previously visited directory   
1. Go forward - Right arrow key should take the user to the next directory   
1. Up one level - Backspace key should take the user up one level   
1. Home – h key should take the user to the home folder   

Command Mode: 

The application should enter the Command button whenever “:” (colon) key is pressed. In the command mode, the user should be able to enter different commands. All commands appear in the status bar at the bottom.   

1. Copy – 

‘$ copy <source\_file(s)> <destination\_directory>’  

Move – 

‘$ move <source\_file(s)> <destination\_directory>’  

Rename – 

‘$ rename <old\_filename> <new\_filename>’  

1. Eg –  

‘$ copy foo.txt bar.txt baz.mp4 ~/foobar’  ‘$ move foo.txt bar.txt baz.mp4 ~/foobar’  ‘$ rename foo.txt bar.txt’ 

2. Assume that the destination directory exists, and you have write permissions.   
2. Copying/Moving directories should also be implemented   
2. The file ownership and permissions should remain intact   
2. Create File –  

‘$ create\_file <file\_name> <destination\_path>’   

Create Directory –  

‘$ create\_dir <dir\_name> <destination\_path>’   

a.  Eg – ‘$ create\_file foo.txt ~/foobar create\_file foo.txt’.   ‘$ create\_dir foo ~/foobar’

3. Delete File –  

‘$ delete\_file <file\_path>’   

Delete Directory –  

‘$ delete\_dir <dir\_path>’ 

a.  On deleting directory, you must recursively delete all content present inside it 

.   

4. Goto –  

‘$ goto <location>’   

a.  Eg – ‘$ goto <directory\_path>’

5. Search –  

‘$ search <file\_name>’  

or  

‘$ search <directory\_name>’   

1. Search for a given file or folder under the current directory recursively.   
1. Output should be True or False depending on whether the file or folder exists   
6. On pressing ESC key, the application should go back to Normal Mode   
6. On pressing q key in normal mode, the application should close. Similarly, entering the ‘quit’ command in command mode should also close the application.  


