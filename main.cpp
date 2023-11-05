#include <ctime>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <iostream>
#include <vector>
#include <stack>
#include <algorithm>
#include <iomanip>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <strings.h>
#include <fstream>
#include <signal.h>
#include <math.h>

#define NAME_INX 0  // name of file/dir
#define PATH_INX 1  // location of file/dir
#define FULLPATH_INX 2  // full path of file/dir
#define PERM_INX 3  // permissions of file/dir
#define USR_INX 4  // user of file/dir
#define GRP_INX 5  // user group of file/dir
#define SIZE_INX 6  // size of file/dir
#define DATE_INX 7  // date created/last modified of file/dir
#define TYPE_INX 8  // type of file ()
#define SNAME_INX 9  // name of file/dir in lower case
#define CM_MODE "Command Mode"
#define NM_MODE "Normal Mode"
#define CLEAR_SCREEN "\033[H\033[2J\033[3J"

using namespace std;

string mode = NM_MODE;

// Command Mode Entity
class CommandMode {
    public:
        // Initializing curr_command and curr_output
        CommandMode(){
            curr_command = "Move to Command Mode. `exitmode` - move to Normal Mode";
            curr_output = "Success";
        }

        // Member vars
        string curr_path;
        string curr_command;
        string curr_output;
        inline static const char * ROOT_PATH = "/";
        inline static const char * HOME_PATH = getpwuid(getuid())->pw_dir;

}cm;

// Normal Mode Entity
class NormalMode {

    public:
        // Initializing curr_path, cursor and scroll
        NormalMode(){
            curr_path = HOME_PATH;
        }

        // Member vars
        vector <vector <string>> files;
        stack <string> f_hist;
        stack <string> b_hist;
        string curr_path;
        int curr_cursor;
        int scroll_size;
        int start_scroll; 
        int end_scroll;
        int space_lines;
        inline static const char * ROOT_PATH = "/";
        inline static const char * HOME_PATH = getpwuid(getuid())->pw_dir;

        // Member functions
        void getFilesAndDirectories();
        string getPermissions(const char *);
        string getFileType(const char *);
        void sortFilesAndDirectories();
        void displayFilesAndDirectories();
        void gotoDirectory();
        friend void displayOnResize(int);
        string stuffString(string, int);
        void setScroll();
        void resetCursorScroll();
        bool pathResolution(string);
        bool removeDirectory(const char []);
        bool searchName(const char [], const char []);
        string formatSize(string); 
}nm;

// Get file and directory names with details
void NormalMode::getFilesAndDirectories(){
    const char * path = curr_path.data();
    string spath = path; 
    files.clear();
    struct dirent *entry;
    DIR *dir = opendir(path);
    
    if (!dir) 
        return;
    
    while ((entry = readdir(dir)) != NULL) {
        vector <string> file(10,"");
        string file_name = entry->d_name;
        string s_file_name = file_name;
        for(auto& c : s_file_name)
            c = tolower(c);
        string path_name = spath;
        string full_path_name = path_name+"/"+file_name;

        file[NAME_INX] = file_name;
        file[SNAME_INX] = s_file_name;
        file[PATH_INX] = spath;
        file[FULLPATH_INX] = full_path_name;
        file[TYPE_INX] = getFileType(file[FULLPATH_INX].data());
        file[PERM_INX] = getPermissions(file[FULLPATH_INX].data());

        struct stat FILE_INFO;
        stat(file[FULLPATH_INX].data(), &FILE_INFO);
        
        file[USR_INX] = getpwuid(FILE_INFO.st_uid)->pw_name;
        file[GRP_INX] = getgrgid(FILE_INFO.st_gid)->gr_name;
        file[DATE_INX] = (string)ctime(&FILE_INFO.st_mtime);
        file[SIZE_INX] = to_string((long long)FILE_INFO.st_size);
        files.push_back(file);
    }
    closedir(dir);
}

// Get file type of a given file/directory
string NormalMode::getFileType(const char * path){
    struct stat st;
    stat(path, &st);
    switch (st.st_mode & S_IFMT) {
        case S_IFBLK:  
            return "block";            
        case S_IFCHR:  
            return "char device";
        case S_IFDIR:  
            return "directory";
        case S_IFIFO:  
            return "fifo pipe";
        case S_IFLNK:  
            return "symlink";
        case S_IFREG:  
            return "regular file";
        case S_IFSOCK: 
            return "socket";
        default:       
            return "unknown";
    }
}

// Get permissions of a given file/directory
string NormalMode::getPermissions(const char * path){
    struct stat st;
    char modeval[9] = {'-','-','-','-','-','-','-','-','-'};
    if(stat(path, &st) == 0){
        mode_t perm = st.st_mode;
        modeval[0] = (perm & S_IRUSR) ? 'r' : '-';
        modeval[1] = (perm & S_IWUSR) ? 'w' : '-';
        modeval[2] = (perm & S_IXUSR) ? 'x' : '-';
        modeval[3] = (perm & S_IRGRP) ? 'r' : '-';
        modeval[4] = (perm & S_IWGRP) ? 'w' : '-';
        modeval[5] = (perm & S_IXGRP) ? 'x' : '-';
        modeval[6] = (perm & S_IROTH) ? 'r' : '-';
        modeval[7] = (perm & S_IWOTH) ? 'w' : '-';
        modeval[8] = (perm & S_IXOTH) ? 'x' : '-';
        string perms = modeval;
        return perms;     
    }
    else{
        return "Error";
    }   
}

// Sort the files and directories in ascending order based on name
void NormalMode::sortFilesAndDirectories(){
    sort(files.begin(), files.end(),
            [](const vector<string>& a, const vector<string>& b) {
    return a[SNAME_INX] < b[SNAME_INX];
    });
}

// Display file and directory details
void NormalMode::displayFilesAndDirectories(){

    cout << CLEAR_SCREEN;  // clear screen

    for(int i=start_scroll;i<=end_scroll;i++){
        vector <string> file = files[i];
        string temp;
        if(curr_cursor == i)
            temp = (string)"==>"+"   "+file[PERM_INX]+"      "+stuffString(file[USR_INX], 6)+"       "+stuffString(file[GRP_INX],6)+"       "+formatSize(file[SIZE_INX])+"      "+file[DATE_INX]+"      "+stuffString(file[NAME_INX],25);
        else
            temp = (string)"   "+"   "+file[PERM_INX]+"      "+stuffString(file[USR_INX], 6)+"       "+stuffString(file[GRP_INX],6)+"       "+formatSize(file[SIZE_INX])+"      "+file[DATE_INX]+"      "+stuffString(file[NAME_INX],25);

        temp.erase(std::remove(temp.begin(), temp.end(), '\n'), temp.cend());  // remove '\n' from string if any
        cout<<temp<<endl;
        
    }
    if (mode == NM_MODE){
        // print front and back pointers
        string bptr = "<#-";
        string fptr = "-#>";
        if(!nm.f_hist.empty())
            fptr = "-->";
        if(!nm.b_hist.empty())
            bptr = "<--";
        string ptr = bptr + "  " + fptr;
        
        for(int i=1;i<=space_lines-2;i++)
            cout<<endl;
        cout<<"NORMAL MODE : Current Directory - "<<nm.curr_path<<"  "<<ptr<<endl;
    }

    else {
        for(int i=1;i<=space_lines-4;i++)
            cout<<endl;
        cout<<"COMMAND MODE : Current Directory - "<<cm.curr_path<<endl;
        cout<<"Prev command : "<<cm.curr_command<<"  ||  "<<"Prev output : "<<cm.curr_output<<endl;
        cout<<"ENTER COMMAND : "<<endl;
    }
}

// Set scroll
void NormalMode::setScroll(){
    int len = files.size();
    
    if (curr_cursor == end_scroll+1) {
        start_scroll = (start_scroll+1) % len;
        end_scroll = (end_scroll+1) % len;
    }

    if(curr_cursor == start_scroll-1) {
        start_scroll = (start_scroll-1) % len;
        end_scroll = (end_scroll-1) % len;
    }
}

// Reset cursor and scroll on DIR change
void NormalMode::resetCursorScroll(){
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int len = files.size();
    curr_cursor = 0;
    scroll_size = min(w.ws_row-5,len);
    start_scroll = 0;
    end_scroll = scroll_size-1;
    space_lines = w.ws_row - scroll_size;
}

// Open/Move to new directory
void NormalMode::gotoDirectory(){
    nm.getFilesAndDirectories();
    nm.sortFilesAndDirectories();
    nm.resetCursorScroll();
    nm.displayFilesAndDirectories();
}

// Resolve path and handle relative and absolute paths
bool NormalMode::pathResolution(string usr_dir){
    if(usr_dir[0] == '~')
        usr_dir.replace(0,1,nm.HOME_PATH);

    if(usr_dir[0] != '/')
        usr_dir = nm.curr_path + "/" + usr_dir;  

    char * p_usr_dir = usr_dir.data();
    char * new_path;
    char actualpath [PATH_MAX+1];
    new_path = realpath(p_usr_dir, actualpath);
    if(new_path){
        cm.curr_path = new_path;
        nm.curr_path = new_path;
        cm.curr_output = "Success!";
        return true;
    }
    else{
        cm.curr_output = "Error! Wrong path entered";
        return false;
    }
        
}

// Stuff strings
string NormalMode::stuffString(string s, int l){
    string result_s = "";
    if(s.length() > l){
        for(int i=0;i<l;i++)
            result_s += s[i];
        result_s += "..";
        return result_s;
    }
    return s;
}

//Format size
string NormalMode::formatSize(string s) {
    long double sz = stold(s);
    static const char *SIZES[] = { "B", "KB", "MB", "GB" };
    int index=0;
    
    while(sz>1024){
        sz = sz/1024;
        index++;
    }

    sz = round( sz * 10.0 ) / 10.0;

    ostringstream convert;
    convert << sz;
    string result = convert.str()+SIZES[index];
    if(result.length()==2)
        result = "     "+result;
    else if (result.length()==3)
        result = "    "+result;
    else if (result.length()==4)
        result = "   "+result;
    else if (result.length()==5)
        result = "  "+result;
    else if (result.length()==6)
        result = " "+result;
    else
        return result;
    return result;
}

// Delete directory
bool NormalMode::removeDirectory(const char path[]){
    size_t path_len;
    char *full_path;
    DIR *dir;
    struct stat stat_path, stat_entry;
    struct dirent *entry;
    stat(path, &stat_path);
    path_len = strlen(path);

    // error handling
    if (S_ISDIR(stat_path.st_mode) == 0) {
        cm.curr_output = "Error! Please enter a valid directory.";
        return false;
    }
    if ((dir = opendir(path)) == NULL) {
        cm.curr_output = "Error! Directory cannot be opened. Please try again.";
        return false;
    }

    // iteration through entries in the directory
    while ((entry = readdir(dir)) != NULL) {
        // skip entries "." and ".."
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;
        // determinate a full path of an entry (max 1 MB)
        full_path = (char*)calloc(1000000, sizeof(char));
        strcpy(full_path, path);
        strcat(full_path, "/");
        strcat(full_path, entry->d_name);
        stat(full_path, &stat_entry);

        // If entry = dir, call recursively to remove a nested directory
        if (S_ISDIR(stat_entry.st_mode) != 0) {
            removeDirectory(full_path);
            continue;
        }
        // else delete file
        if (unlink(full_path) != 0) {
            cm.curr_output = "Error! File cannot be deleted. Please try again.";
            return false;
        }    
        free(full_path);
    }
    // delete directory
    if (rmdir(path) != 0) {
        cm.curr_output = "Error! Directory cannot be deleted. Please try again.";
        return false;
    }
    closedir(dir);
    return true;
}

// Search name if present in Current Directory
bool NormalMode::searchName(const char path[], const char name[]){
    size_t path_len;
    char *full_path;
    DIR *dir;
    struct stat stat_path, stat_entry;
    struct dirent *entry;
    stat(path, &stat_path);
    path_len = strlen(path);
    
    S_ISDIR(stat_path.st_mode);
    dir = opendir(path);

    // iteration through entries in the directory
    while ((entry = readdir(dir)) != NULL) {
        // skip entries "." and ".."
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;
        // determinate a full path of an entry (max 1 MB)
        full_path = (char*)calloc(1000000, sizeof(char));
        strcpy(full_path, path);
        strcat(full_path, "/");
        strcat(full_path, entry->d_name);
        stat(full_path, &stat_entry);
        string res = entry->d_name;
        // if name matches, return true
        if (res == name)
            return true;

        // If entry = dir, call recursively to enter a nested directory
        if (S_ISDIR(stat_entry.st_mode) != 0) {
            bool result = searchName(full_path, name);
            if(result) 
                return true;
            continue;
        }
    }
    return false;
}


// Display on resize  (Friend Function to NormalMode)
void displayOnResize(int s){
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    cout << CLEAR_SCREEN;  // clear screen
    nm.curr_cursor = 0;
    int len = nm.files.size();

    if (w.ws_row >= 12) {
        nm.scroll_size = min(w.ws_row-5,len);
        nm.start_scroll = 0;
        nm.end_scroll = nm.scroll_size-1;
        nm.space_lines = w.ws_row - nm.scroll_size;
    }
        
    else {
        cout<<CLEAR_SCREEN;  // clear screen
        cout<<"Please increase the screen size";
        return;
    }

    nm.displayFilesAndDirectories();
}


int main(){

    // Application starts in Normal mode at HOME PATH
    nm.getFilesAndDirectories();
    nm.sortFilesAndDirectories();
    nm.resetCursorScroll();
    nm.displayFilesAndDirectories();

    // Take signal when terminal is resized
    signal(SIGWINCH, displayOnResize);

    // Change terminal to non-canonical mode
    int ret;
    char *term_name;
    struct termios termios_new, termios_backup;
    term_name = getenv("TERM");
    bzero(&termios_new, sizeof(struct termios));
    tcgetattr(STDIN_FILENO, &termios_backup);
    termios_new = termios_backup;
    termios_new.c_lflag &= ~(ICANON);  // un-set ICANON to enter non-canonical
    termios_new.c_lflag &= ~(ECHO);
    termios_new.c_cc[VMIN] = 1;  // reads is completed when one input is taken
    termios_new.c_cc[VTIME] = 0;  // no time buffer between inputs
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &termios_new);

    // Fetch input non-canonically
    while((ret = getchar()) > 0)
    {
        cout << CLEAR_SCREEN;  // clear screen

        // UP key
        if (ret == 65){
            if(nm.curr_cursor == 0)
                nm.displayFilesAndDirectories();
            else {
                int len = nm.files.size();
                nm.curr_cursor = (nm.curr_cursor-1)%len;
                nm.setScroll();
                nm.displayFilesAndDirectories();
            }
        }

        // DOWN Key
        else if (ret == 66){
            int len = nm.files.size();
            if(nm.curr_cursor == len-1)
                nm.displayFilesAndDirectories();
            else {
                nm.curr_cursor = (nm.curr_cursor+1)%len;
                nm.setScroll();
                nm.displayFilesAndDirectories();
            }
        }

        // RIGHT Key
        else if (ret == 67){
            if (nm.f_hist.empty()) {
                nm.displayFilesAndDirectories();
                continue;
            }
            else {
                nm.curr_path = nm.f_hist.top();
                nm.f_hist.pop();
                nm.gotoDirectory();
            }
        }
            
        // LEFT Key
        else if (ret == 68){
            if (nm.b_hist.empty()) {
                nm.displayFilesAndDirectories();
                continue;
            }
            else {
                nm.curr_path = nm.b_hist.top();
                nm.b_hist.pop();
                nm.gotoDirectory();
            }
        }
            
        // ENTER Key
        else if (ret == '\n'){
            if (nm.files[nm.curr_cursor][TYPE_INX] == "directory"){
                nm.b_hist.push(nm.curr_path);
                nm.curr_path = nm.curr_path+"/"+nm.files[nm.curr_cursor][NAME_INX];
                char actualpath [PATH_MAX+1];
                nm.curr_path = realpath(nm.curr_path.c_str(), actualpath);
                nm.gotoDirectory();
            }

            else if (nm.files[nm.curr_cursor][TYPE_INX] == "regular file"){
                int pid = fork();
                if (pid == 0) {
                    nm.getFilesAndDirectories();
                    nm.sortFilesAndDirectories();
                    nm.setScroll();
                    nm.displayFilesAndDirectories();
                    execlp("open", "open", nm.files[nm.curr_cursor][FULLPATH_INX].c_str(), NULL);
                    cout <<"Exec Error Code: "<<errno<<" : "<<strerror(errno)<<endl;  // Error is execlp returns
                    exit(1);  // Exit child process
                }
            }
        }

        // BACKSPACE Key
        else if (ret == 127){
            if (nm.curr_path == nm.ROOT_PATH){
                nm.gotoDirectory();
            }
            else {
                nm.f_hist.push(nm.curr_path);
                while(nm.curr_path[nm.curr_path.length()-1] != '/')
                    nm.curr_path.pop_back();
                if(nm.curr_path != "/")
                    nm.curr_path.pop_back();
                nm.gotoDirectory();
            }
        }

        // h Key
        else if (ret == 104){
            nm.curr_path = nm.HOME_PATH;
            nm.gotoDirectory();
        }
        
        // q Key
        else if (ret == 113){
            break;
        }
            
        // : Key -> Switch to command mode
        else if (ret == 58){
            cm.curr_path = nm.curr_path;
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &termios_backup);
            mode = CM_MODE;
            nm.displayFilesAndDirectories();
            
            while(1){
                // Enter command in command mode
                getline(cin, cm.curr_command);
                
                // split commands in words and push in vector
                vector <string> cmd_args;
                istringstream ss(cm.curr_command);
                string word;
                while (ss >> word)
                    cmd_args.push_back(word);

                // 6. - exitmode command
                if (cmd_args[0] == "exitmode"){
                    cm.curr_output = "Successfully switched modes";
                    break;
                }
                    
                // 7 - quit command
                else if (cmd_args[0] == "quit"){
                    return (0);
                }
                    
                // 4 - goto command
                else if (cmd_args[0] == "goto"){
                    if (cmd_args.size() != 2)
                        cm.curr_output = "Error! Please enter -> goto <destination_path>";
                    else {
                        string bkup = nm.curr_path;
                        string usr_dir = cmd_args[1];
                        nm.pathResolution(usr_dir);
                        if (nm.getFileType(nm.curr_path.data()) == "regular file") {
                            nm.curr_path = bkup;
                            cm.curr_path = bkup;
                            cm.curr_output = "Error! Please enter directory path";
                            nm.gotoDirectory();
                        }
                        else
                            nm.gotoDirectory();
                    }
                }

                // 2a - create_file command
                else if (cmd_args[0] == "create_file"){
                    if (cmd_args.size()!= 3)
                        cm.curr_output = "Error! Please enter -> create_file <file_name> <destination_path>";
                    else {
                        string file_name = cmd_args[1];
                        string usr_dir = cmd_args[2];
                        if (nm.pathResolution(usr_dir)){
                            FILE *fp;
                            if (nm.curr_path[nm.curr_path.length()-1] == '/')
                                fp  = fopen ((nm.curr_path+file_name).data(), "w+");
                            else
                                fp  = fopen ((nm.curr_path+"/"+file_name).data(), "w+");
                            fclose (fp);
                        }
                    }
                    nm.gotoDirectory();
                }

                // 2b - create_dir command
                else if (cmd_args[0] == "create_dir"){
                     if (cmd_args.size()!= 3)
                        cm.curr_output = "Error! Please enter -> create_dir <dir_name> <destination_path>";
                    else {
                        string folder = cmd_args[1];
                        string usr_dir = cmd_args[2];
                        if (nm.pathResolution(usr_dir)){
                            char * final;
                            if (nm.curr_path[nm.curr_path.length()-1] == '/'){
                                if(mkdir((nm.curr_path+folder).data(),0777) == -1)
                                    cm.curr_output = "Unable to create folder. Error!";
                            }
                            else {
                                if(mkdir((nm.curr_path+"/"+folder).data(),0777) == -1)
                                    cm.curr_output = "Unable to create folder. Error!";
                            }
                        }
                    }
                    
                    nm.gotoDirectory();
                }

                // 3a - delete_file command
                else if (cmd_args[0] == "delete_file"){
                    if (cmd_args.size()!= 2)
                        cm.curr_output = "Error! Please enter -> delete_file <file_path>";
                    else {
                        string usr_dir = cmd_args[1];
                        string file_name;
                        if (nm.pathResolution(usr_dir)){
                            if (nm.getFileType(nm.curr_path.data()) == "regular file"){
                                char buf[256];
                                file_name = nm.curr_path.substr(nm.curr_path.rfind("/") + 1);
                                while(nm.curr_path[nm.curr_path.length()-1] != '/')
                                    nm.curr_path.pop_back();
                                if(nm.curr_path != "/")
                                    nm.curr_path.pop_back();
                                sprintf(buf, "%s/%s", nm.curr_path.c_str(), file_name.data());
                                remove(buf);
                                cm.curr_path = nm.curr_path;
                            }
                            else {
                                while(nm.curr_path[nm.curr_path.length()-1] != '/')
                                    nm.curr_path.pop_back();
                                if(nm.curr_path != "/")
                                    nm.curr_path.pop_back();
                                cm.curr_path = nm.curr_path;
                                cm.curr_output = "Error! Please enter a correct file path";
                            }
                        }
                    }
                    nm.gotoDirectory();
                }

                // 3b - delete_dir command
                else if (cmd_args[0] == "delete_dir"){
                    if (cmd_args.size()!= 2)
                        cm.curr_output = "Error! Please enter -> delete_file <dir_path>";
                    else {
                        string usr_dir = cmd_args[1];
                        if (nm.pathResolution(usr_dir)){
                            if (nm.getFileType(nm.curr_path.data()) == "directory" && nm.curr_path != nm.HOME_PATH && nm.curr_path != nm.ROOT_PATH){
                                bool status = nm.removeDirectory(nm.curr_path.c_str());
                                if(status){
                                    while(nm.curr_path[nm.curr_path.length()-1] != '/')
                                        nm.curr_path.pop_back();
                                    if(nm.curr_path != "/")
                                        nm.curr_path.pop_back();
                                    cm.curr_path = nm.curr_path;
                                }
                            }
                            else {
                                while(nm.curr_path[nm.curr_path.length()-1] != '/')
                                    nm.curr_path.pop_back();
                                if(nm.curr_path != "/")
                                    nm.curr_path.pop_back();
                                cm.curr_path = nm.curr_path;
                                cm.curr_output = "Error! Please enter a correct directory path";
                            }
                        }
                    }
                    nm.gotoDirectory();
                }

                // 2a - copy command (only for files)
                else if (cmd_args[0] == "copy"){
                    
                    string infilename; 
                    string outfileDir = cmd_args[cmd_args.size()-1];
                    string cp = nm.curr_path;
                    if (nm.pathResolution(outfileDir)) {
                        for(int i=1;i<cmd_args.size()-1;i++){
                            infilename  = cmd_args[i];
                            // path resolve only for file name
                            if(infilename[0] == '~')
                                infilename.replace(0,1,nm.HOME_PATH);
                            if(infilename[0] != '/')
                                infilename = cp + "/" + infilename;  
                            char * p_usr_dir = infilename.data();
                            char * new_path;
                            char actualpath [PATH_MAX+1];
                            new_path = realpath(p_usr_dir, actualpath);
                            if(new_path){
                                string new_file_path;
                                new_file_path = new_path;
                                FILE* infile; //File handles for source and destination.
                                FILE* outfile;
                                char outfilename[PATH_MAX];
                                infile = fopen(new_file_path.c_str(), "r"); // Open the input and output files.
                                sprintf(outfilename, "%s/%s", nm.curr_path.c_str(), basename(new_file_path.c_str()));
                                outfile = fopen(outfilename, "w");
                            }
                            else
                                cm.curr_output = "Error! Please check the filenames again!";
                        }
                    }
                    nm.gotoDirectory();
                }
                
                // 2b - move command (only for files)
                else if (cmd_args[0] == "move"){
                    
                    string infilename; 
                    string outfileDir = cmd_args[cmd_args.size()-1];
                    string cp = nm.curr_path;
                    if (nm.pathResolution(outfileDir)) {
                        for(int i=1;i<cmd_args.size()-1;i++){
                            infilename = cmd_args[i];
                        
                            // path resolve only for file name
                            if(infilename[0] == '~')
                                infilename.replace(0,1,nm.HOME_PATH);
                            if(infilename[0] != '/')
                                infilename = cp + "/" + infilename;  
                            char * p_usr_dir = infilename.data();
                            char * new_path;
                            char actualpath [PATH_MAX+1];
                            new_path = realpath(p_usr_dir, actualpath);
                            if(new_path){
                                infilename = new_path;                                
                                FILE* infile; //File handles for source and destination.
                                FILE* outfile;
                                char outfilename[PATH_MAX];
                                infile = fopen(infilename.c_str(), "r"); // Open the input and output files.
                                sprintf(outfilename, "%s/%s", nm.curr_path.c_str(), basename(infilename.c_str()));
                                outfile = fopen(outfilename, "w");

                                char buf[256];
                                string fn = infilename.substr(infilename.rfind("/") + 1);
                                while(infilename[infilename.length()-1] != '/')
                                    infilename.pop_back();
                                if(infilename != "/")
                                    infilename.pop_back();
                                sprintf(buf, "%s/%s", infilename.c_str(), fn.data());
                                remove(buf);
                            }
                            else
                                cm.curr_output = "Error! Please check the filenames again!";
                            
                        }
                    }
                    
                    nm.gotoDirectory();
                }

                // 2c - rename command
                else if (cmd_args[0] == "rename"){
                    if(cmd_args.size() != 3)
                        cm.curr_output = "Error! Please enter -> rename <file_name> <new_file_name>";

                    else {
                        string old_name = cmd_args[1]; 
                        string new_name = cmd_args[2];

                        // path resolve for old name
                        if(old_name[0] == '~')
                            old_name.replace(0,1,nm.HOME_PATH);
                        if(old_name[0] != '/')
                            old_name = nm.curr_path + "/" + old_name;  
                        char * p_usr_dir = old_name.data();
                        char * new_path;
                        char actualpath [PATH_MAX+1];
                        new_path = realpath(p_usr_dir, actualpath);
                        if (new_path){
                            old_name = new_path;
                            nm.curr_path = old_name;
                            while(nm.curr_path[nm.curr_path.length()-1] != '/')
                                nm.curr_path.pop_back();
                            if(nm.curr_path != "/")
                                nm.curr_path.pop_back();
                            cm.curr_path = nm.curr_path;
                            int ret;
                            if (cm.curr_path == "/")
                                ret = rename(old_name.c_str(), (cm.curr_path+new_name).c_str());
                            else
                                ret = rename(old_name.c_str(), (cm.curr_path+"/"+new_name).c_str());
                        }
                        else 
                            cm.curr_output = "Error! Please check the filename again!";
                    }

                    nm.gotoDirectory();
                }

                // 5 - search command
                else if (cmd_args[0] == "search"){
                    if (cmd_args.size()!= 2)
                        cm.curr_output = "Error! Please enter -> search <file_name OR directory_name>";
                    else {
                        string name = cmd_args[1];
                        // search for file
                        bool status = nm.searchName(nm.curr_path.c_str(), name.c_str());
                        
                        if (status)
                            cm.curr_output = "File/Folder found! TRUE";
                        else
                            cm.curr_output = "File/Folder not found! FALSE";
                    }
                    nm.gotoDirectory();
                }

                // any other command
                else {
                    cm.curr_output = "Wrong command! Please try again";
                    nm.displayFilesAndDirectories();
                }
                
                nm.displayFilesAndDirectories();
            }

            // restore non-canon mode
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &termios_new);
            mode = NM_MODE;
            cout<<nm.curr_path;
            nm.displayFilesAndDirectories();
        }

        // Any other key press
        else{
            nm.displayFilesAndDirectories();
            continue;
        }
            
    }

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &termios_backup);  // restore terminal to previous settings
    return (0);  // exit
    
}