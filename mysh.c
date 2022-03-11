#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

int command(char** args_command,int r, char* output_filename){

    int pid = fork();
    
    if (pid < 0){
        exit(1);
    }
    else if (pid == 0){
        //redirect output
        if(r == 1){
            FILE* output_filestream = fopen(output_filename, "w");
            int fd = open(output_filename, O_WRONLY, O_CREAT, O_TRUNC, 0644);
            dup2(fd, fileno(stdout));
            close(fd);
            fclose(output_filestream);
            
        }
        execv(args_command[0],args_command);
        exit(1);
    }
    else if (pid > 0){
        int status;
        waitpid(pid, &status, 0);
    }
    return 0;
}


int main (int argc, char* argv[]){
    FILE *input_stream = malloc(256);
    int mode = 0;
    //check numbers of arguments
    if (argc > 2){
        write(STDERR_FILENO, "Usage: mysh [batch-file]\n", 25);
        return 1;
    }
    //Set input stream depending on batch or interactive
    if(argc>1){
        input_stream = fopen(argv[1],"r");
        if(input_stream == NULL){
            char* errmess = malloc(256);
            strcpy(errmess, "Error: Cannon open file ");
            strcat(strcat(errmess,argv[1]),".\n");
            write(STDERR_FILENO, errmess, strlen(errmess));
            return 1;
        }
    }
    else if(argc==1){
        input_stream = stdin;
        mode = 1;
    }
    while(1){
        if(feof(input_stream)){
            break;
        }
        if(mode){
            write(1,"mysh> ",6);
        }
        char* line = malloc(512);
        fgets(line, 512, input_stream);
        char* line_copy = malloc(strlen(line));
        strcpy(line_copy, line);
        //check if redirection has been inputted
        
        if (mode==0){
            write(1,line,strlen(line));
        }

        if(strcmp(line,"exit\n") == 0 || strcmp(line, "exit") == 0){
            break;
        }

        char* output_file = "";
        int r = 0;
        if (strstr(line_copy,">")!= NULL){
            line = strtok(line_copy, ">");
            output_file = strtok(NULL, " \n\t");
            r = 1;
        }
        
        //separate command
        char* token = strtok(line, " \n\t");
        int count = 0;
        char* command_args[100];
        while(token != NULL){
            command_args[count++] = token;
            token = strtok(NULL, " \n\t");
        }
        //write(1, line_copy, strlen(line_copy));
        command_args[count] = NULL;
        
        
        //write(1,output_file, strlen(output_file));
        if(r == 1){
            command(command_args,1,output_file);
        }
        else{
            command(command_args,0,output_file);
        }
    }
    exit(0);
}
