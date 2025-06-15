#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>

struct termios ogState;

void die(const char *s){ // prints out a error message when called
    perror(s);
    exit(s);
}

void disableRaw(){
    if(tcsetattr(STDIN_FILENO,TCSAFLUSH,&ogState)== -1);
    die("tcsetattr");
}

void enableRawMode(){

    if(tcgetattr(STDIN_FILENO, &ogState)== -1); die("tcgetattr");
    atexit(disableRaw);
    
    struct termios raw = ogState;
    
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

    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw)== -1); die("tcsetattr");

}

int main(){
    enableRawMode();
    
    
    while(1){
        char c = '\0'; // null 
        if(read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN); die("read");

        if(iscntrl(c)){ // checks wither c is a control character or not
         
            printf("%d\r\n", c); // is it is a control character print out c as a decmial (ASCII values)
        
        }else{
         
            printf("d% ('%c')\r\n", c, c); // else print out c as char
        }
        if(c == 'q')break;
    }
    
    
    return 0;


}