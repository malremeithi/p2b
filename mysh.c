#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
struct clone *init;

struct clone
{
    char *name;
    char *content;
    struct clone *next;
    struct clone *prev;
};
struct clone * inAlias(char * name_key){
        struct clone * iter = init;
        while(iter!=NULL){
        if(strcmp(iter->name, name_key)==0){
                return iter;
        }
        else
                iter=iter->next;
        }
        return NULL;
}

int command(char** args_command,int r, char* output_filename){

    int pid = fork();

    if (pid < 0){
        exit(1);
    }
    else if (pid == 0){
        //redirect output
        if(r != 0 ){
            FILE* output_filestream = fopen(output_filename, "w+");
            if( r==-1 || output_filestream==NULL){
            write(STDERR_FILENO, "Redirection misformatted.\n", 26);
            exit(1);
            }
            int fd = open(output_filename, O_WRONLY, O_CREAT, O_TRUNC, 0644);
            dup2(fd, fileno(stdout));
            close(fd);
            fclose(output_filestream);

        }
        execv(args_command[0],args_command);
         write(STDERR_FILENO, args_command[0], strlen(args_command[0])); //print command name
            write(STDERR_FILENO, ": Command not found.\n", 21); //print error message
  //          exit(1);
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
    int n_clones =0;
    init = NULL;
    struct clone * cur = NULL;


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
            strcpy(errmess, "Error: Cannot open file ");
            strcat(errmess,argv[1]);
            strcat(errmess,".\n");
            write(STDERR_FILENO, errmess, strlen(errmess));
            free(errmess);
            exit(1);
        }
    }
    else if(argc==1){
        input_stream = stdin;
        mode = 1;
    }
    while(1){

        //if(getc(input_stream)==EOF){
          //  break;
        //}
        if(mode){
            write(1,"mysh> ",6);
        }
        char* line = malloc(512);
//        if(fgets(line, 512, input_stream)==NULL && line!=NULL && strcmp(line, "\n")!=0)
//                      break;
//
        if(fgets(line, 512, input_stream)==NULL)
                break;
        char* line_copy = malloc(strlen(line));
        strcpy(line_copy, line);
        //check if redirection has been inputted

        if (mode==0){
            write(1,line,strlen(line));
        }

        if(strcmp(line,"exit\n") == 0 || strcmp(line, "exit") == 0 || feof(input_stream)){
            free(line);
            free(line_copy);
            break;
        }


        char* output_file = "";
        int r = 0;
        if (strstr(line_copy,">")!= NULL){
            if (line_copy[0] == '>') {
                write(STDERR_FILENO, "Redirection misformatted.\n",26);
                free(line_copy);
                free(line);
                continue;
            }
            line = strtok(line_copy, ">");
            output_file = strtok(NULL, " \n\t");
            r = 1;
            char* next_tok;
            next_tok = strtok(NULL, " \n\t");
            if(next_tok!=NULL){
                    r=-1;
            }
        }

        //separate command
        char* token = strtok(line, " \n\t");
        int count = 0;
        int al=0;
        char* command_args[100];
        if(token == NULL){
            continue;
        }
        while(token != NULL){
            command_args[count++] = token;
            token = strtok(NULL, " \n\t");
        }
        command_args[count] = NULL;
        char * al_command_args[100];
        struct clone *check = (struct clone*)malloc(sizeof(struct clone));
        check = inAlias(command_args[0]);
        if(check!=NULL){
                al =1;
                char * token_al;
                token_al = strtok(check->content, " \n\t");
                int count_al=0;
                while(token_al != NULL){
                 al_command_args[count_al] = strdup(token_al);
                 count_al++;
                 token_al = strtok(NULL, " \n\t");
                }
                for(int y=0; y<count-1; y++){
                        al_command_args[count_al+y] = strdup(command_args[y]);

                }
                al_command_args[count_al+count-1]=NULL;




        }


        //aliases 
        if(strcmp(command_args[0], "alias")==0 && count!=1 && count!=2){

                         struct clone *tmp = (struct clone *)malloc(sizeof(struct clone));
         tmp->name = (char *)malloc(256 * sizeof(char));
         strcpy(tmp->name, command_args[1]);
        tmp->content = (char *)malloc(256*sizeof(char));
        for(int y=2; y<count; y++){
        strcat(tmp->content, command_args[y]);
        if(y!=count-1)
                strcat(tmp->content, " ");
        }
        struct clone* clone_tmp = inAlias(tmp->name);
        if(clone_tmp!=NULL){
                clone_tmp->content = tmp-> content;
                continue;
        }
         tmp->next = NULL;
         tmp->prev = cur;
         if (n_clones == 0)
             init = tmp;
         else
             cur->next = tmp;
         cur = tmp;
         n_clones+=1;
                continue;
        }
        else if(strcmp(command_args[0], "alias")==0 && count==1){
        struct clone *tmp = init;
        while(tmp!=NULL){

                write(1, tmp->name, strlen(tmp->name));
                write(1, " ", 1);
                write(1, tmp->content, strlen(tmp->content));
                write(1, "\n", 1);
                tmp = tmp->next;
        }
        continue;
        }
        else if(strcmp(command_args[0], "unalias")==0 ){
                if(count!=2){
                        write(1, "unalias: Incorrect number of arguments.\n", 40);
                        continue;
                }
                struct clone *tmp = init;
                while(tmp!=NULL){
                if(strcmp(tmp->name, command_args[1])==0){
                        tmp->prev->next = tmp->next;
                        tmp->next->prev = tmp->prev;
                        n_clones--;
                        break;
                }

                }
        }
        else if(strcmp(command_args[0], "alias")==0 && count==2)
        {
                struct clone *check;
                check = inAlias(command_args[1]);
                if(check!=NULL){
                write(1, check->name, strlen(check->name));
                write(1, " ", 1);
                write(1, check->content, strlen(check->content));
                write(1, "\n", 1);
                }
                continue;
        }

        if(!al)
            command(command_args,r,output_file);
        else
            command(al_command_args,0,output_file);
        free(line);
    }
    if (mode == 0){
        fclose(input_stream);
    }
    exit(0);
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  
                 
