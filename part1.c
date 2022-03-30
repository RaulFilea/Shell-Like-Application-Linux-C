#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<readline/readline.h>
#include<readline/history.h>

#define MAXCHAR 5000 // max nr of letters
#define MAXCMD 500 // max nr of commands

// gcc part1.c -lreadline
// ./a.out

int readLine(char* input){
    char* buffer;

    buffer = readline("\n>_ ");
    if(strlen(buffer) != 0) {
        add_history(buffer);
        strcpy(input, buffer);
        return 0;
    } else {
        return 1;
    }
}

//function for simple commands
void execSimpleCmd(char** parsed){
    pid_t pid = fork();

    if(pid == -1) {
        printf("\nFailed forking child\n");
        return;
    } else if (pid == 0) {
        if(execvp(parsed[0], parsed) < 0) {
            printf("\nCommand could not be executed\n");
        }
        exit(0);
    } else {
        //wait for child
        wait(NULL);
        return;
    }
}

//function for piped commands
void execPipedCmd(char** parsed, char** parsedPipe){

    int pipefd[2];
    pid_t p1, p2;
    if(pipe(pipefd) < 0) {
        printf("\nFailed initializing pipe\n");
        return;
    }
    p1 = fork();
    if(p1 < 0) {
        printf("\nCould not fork\n");
        return;
    }

    if(p1 == 0) {
        // child 1 executing
        close(pipefd[0]);                     //close unused write end (for child)
        dup2(pipefd[1], STDIN_FILENO);      //destination file descriptor will point to the same file as pipefd[1]
        close(pipefd[1]);

        if(execvp(parsed[0], parsed) < 0) {
            printf("\nCould not execute first command\n");
            exit(0);
        }
    } else { //parent executing
        p2 = fork();
        if (p2 < 0) {
            printf("\nCould not fork\n");
            return;
        }

        // child 2 executing
        if (p2 == 0) {
            close(pipefd[1]);                     //close unused read end (for parent)
            dup2(pipefd[0], STDOUT_FILENO);     //copy pipefd[0] to stdin_fileno
            close(pipefd[0]);
            if (execvp(parsedPipe[0], parsedPipe) < 0) {
                printf("\nCould not execute command 2..");
                exit(0);
            }
        } else {        //parent executing, waiting for 2 children
            wait(NULL);
            wait(NULL);
        }
    }
}

// Function to execute builtin commands
int myCommands (char** parsed){
    int nrCmd = 4;      //number of commands
    int i, switchOwnArg = 0;
    char* listCmd[nrCmd];

    listCmd[0] = "exit";
    listCmd[1] = "cat";
    listCmd[2] = "head";
    listCmd[3] = "env";

    for (i = 0; i < nrCmd; i++) {
        if(strcmp(parsed[0], listCmd[i]) == 0) {
            switchOwnArg = i + 1;
            break;
        }
    }

    switch (switchOwnArg) {
    case 1:
        printf("\nAlmost exiting...\n");
        exit(0);
    case 2:
        printf("\ncat not implemented\n");
        return 1;
    case 3:
        printf("\nhead not implemented\n");
        return 1;
    case 4:
        printf("\nenv not implemented\n");
        return 1;
    default:
        break;
    }

    return 0;
}

// function for finding pipe
int pipeParsing(char* str, char** pipedStr){
    int i;
    for (i=0;i<2;i++) {
        pipedStr[i] = strsep(&str, "|");    //slice up the string, look for |
        if (pipedStr[i] == NULL)
            break;
    }

    if (pipedStr[1] == NULL)
        return 0; //no pipe found
    else return 1;
}
//function for breaking the words in a piped command
void parseSpace(char* str, char** parsedCommand){
    int i;
    for (i=0; i<MAXCMD; i++) {
        parsedCommand[i] = strsep(&str, " ");
        if (parsedCommand[i] == NULL)
            break;
        if (strlen(parsedCommand[i]) == 0)
            i--;
    }
}

int processString(char* str, char** parsed, char** parsedPipe){
    char* strPiped[2];
    int piped = 0;
    piped = pipeParsing(str, strPiped);     //piped=1 if it is a piped command

    if (piped) {        //if there is a pipe, piped!=0
        parseSpace(strPiped[0], parsed);
        parseSpace(strPiped[1], parsedPipe);

    } else parseSpace(str, parsed);

    if (myCommands(parsed)!=0)
        return 0;
    else return 1 + piped;
    // returns 1 if not piped or 2 if piped
}

int main()
{
    char inputStr[MAXCHAR], *parsedCmd[MAXCMD];
    char* parsedCmdPiped[MAXCMD];
    int execType = 0;

    while (1) {

        if (readLine(inputStr))
            continue;
        execType = processString(inputStr, parsedCmd, parsedCmdPiped);      //check the type of the command
        // returns 0 for my commands: exit, ls, tac, dirname
        if (execType == 1)
            execSimpleCmd(parsedCmd);

        if (execType == 2)
            execPipedCmd(parsedCmd, parsedCmdPiped);
    }
    return 0;
}
