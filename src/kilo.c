#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>

#pragma region defines

#define CTRL_KEY(k) ((k)& 0x1f)
#pragma endregion

#pragma region data

struct editorConfig {
    
    struct termios ogState;
}; 

struct editorConfig E;
#pragma endregion

#pragma region Output

void editorDrawRows(){
    
    int y; 
    for (y = 0; y < 24; y++)
    {
        write(STDOUT_FILENO, "~/r/n,", 3);
    }
    
}

void editorRefershScreen(){
    
    // \1xb is a escape sequence to the terminal (1 byte) the other 3 are [2J which is a escape command.
    write(STDOUT_FILENO, "\x1b[2J", 4);
    // [H is for Cursor positioning. Takes 2 arugments rows and columns. By default they are both set to 1.
    write(STDOUT_FILENO, "\x1b[H", 3);
    
    editorDrawRows(); // draw rows
    write(STDOUT_FILENO, "\x1b[H", 3); // reset coursor
}
#pragma endregion

#pragma region Terminal

char editorReadKey(){
    int nRead;
    char c; 
    
    /*error handler*/
    while((nRead = read(STDIN_FILENO, &c, 1)) != 1 ){ 
        if(nRead == -1 && errno != EAGAIN) die("read"); 
    }
    return c;
}

void die(const char *s){ // prints out a error message when called
    
    //clear screen 
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    
    // print error
    perror(s);
    exit(1);
}

void disableRaw(){
    if(tcsetattr(STDIN_FILENO,TCSAFLUSH,&E.ogState) == -1)
        die("tcsetattr");
}

void enableRawMode(){

    if(tcgetattr(STDIN_FILENO, &E.ogState) == -1) die("tcgetattr");
    atexit(disableRaw);
    
    struct termios raw = E.ogState;
    
    // Input flags turned off
    raw.c_iflag &= ~(ISTRIP // causes the 8th bit of each character to be turned off
    | IXON // Turns off clrt s and q. s stops input reading until q is hit and plays catch up
    | ICRNL // Turns off clrt m to return 13 instead of 10 same with enter. This is because both perform carrgie return (returning input)
    | BRKINT // turns off breaks like crlt c fir 
    | INPCK); // ables parily checking (dont need to worry about this)
    
    raw.c_cflag |= (CS8); // Character size(CS) is a bit mask sets the character sizes to 8 bits per byte. Standard for IDEs

    //out put flag turned off
    raw.c_oflag &= ~(OPOST); // Turns off post processing. Only makes a new line instead of returing

    // local flags turned off
    raw.c_lflag &= ~( 
        ECHO | // turns off the ability to see text
        ICANON | // Reads every byte the users puts in
        ISIG | // Turns off crlt z and c commands
        IEXTEN); // Turns off clrt v and o commmands

    //Control characters (They are arrays of bytes)
    raw.c_cc[VMIN] = 0; // min chars to read before before reads return
    raw.c_cc[VTIME] = 1; // the time before timeout

    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw)== -1) die("tcsetattr");

}

int getWindowSize (int *row, int *cols){

    struct winsize ws;

    //icotl is input output termial control and TIOCGEWINSZ is Termial input out control get window size
    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0){
        return -1;
    }else{
        *cols = ws.ws_col;
        *row = ws.ws_row;
        return 0;
    }
}
#pragma endregion

#pragma region Inputs

void editorProcessKeypresses(){
    
    char c = editorReadKey();
    
    /*quick keys*/
    switch(c){
        case CTRL_KEY('q'):
        write(STDOUT_FILENO, "\x1b[2J", 4);
        write(STDOUT_FILENO, "\x1b[H", 3);
        exit(0);
        break;
    }
}
#pragma endregion

#pragma region init
int main(){
    enableRawMode();

    
    while(1){
        editorRefershScreen();
        editorProcessKeypresses();
    }
    
    
    return 0;
}
#pragma endregion