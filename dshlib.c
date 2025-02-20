#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "dshlib.h"
#include <errno.h>

/*
 * Implement your exec_local_cmd_loop function by building a loop that prompts the 
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.
 * 
 *      while(1){
 *        printf("%s", SH_PROMPT);
 *        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
 *           printf("\n");
 *           break;
 *        }
 *        //remove the trailing \n from cmd_buff
 *        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
 * 
 *        //IMPLEMENT THE REST OF THE REQUIREMENTS
 *      }
 * 
 *   Also, use the constants in the dshlib.h in this code.  
 *      SH_CMD_MAX              maximum buffer size for user input
 *      EXIT_CMD                constant that terminates the dsh program
 *      SH_PROMPT               the shell prompt
 *      OK                      the command was parsed properly
 *      WARN_NO_CMDS            the user command was empty
 *      ERR_TOO_MANY_COMMANDS   too many pipes used
 *      ERR_MEMORY              dynamic memory management failure
 * 
 *   errors returned
 *      OK                     No error
 *      ERR_MEMORY             Dynamic memory management failure
 *      WARN_NO_CMDS           No commands parsed
 *      ERR_TOO_MANY_COMMANDS  too many pipes used
 *   
 *   console messages
 *      CMD_WARN_NO_CMD        print on WARN_NO_CMDS
 *      CMD_ERR_PIPE_LIMIT     print on ERR_TOO_MANY_COMMANDS
 *      CMD_ERR_EXECUTE        print on execution failure of external command
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 1+)
 *      malloc(), free(), strlen(), fgets(), strcspn(), printf()
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 2+)
 *      fork(), execvp(), exit(), chdir()
 */

static int last_return_code = 0;

int alloc_cmd_buff(cmd_buff_t *cmd_buff) {
    if (cmd_buff == NULL) {
        return ERR_MEMORY;
    }

    cmd_buff->_cmd_buffer = malloc(SH_CMD_MAX);
    if (cmd_buff->_cmd_buffer == NULL) {
        return ERR_MEMORY;
    }

    for (int i = 0; i < CMD_ARGV_MAX; i++) {
        cmd_buff->argv[i] = NULL;
    }

    cmd_buff->argc = 0;
    return OK;
}

int free_cmd_buff(cmd_buff_t *cmd_buff) {
    if (cmd_buff == NULL) {
        return ERR_MEMORY;
    }

    if (cmd_buff->_cmd_buffer != NULL) {
        free(cmd_buff->_cmd_buffer);
        cmd_buff->_cmd_buffer = NULL;
    }

    for (int i = 0; i < CMD_ARGV_MAX; i++) {
        cmd_buff->argv[i] = NULL;
    }
    cmd_buff->argc = 0;

    return OK;
}

int clear_cmd_buff(cmd_buff_t *cmd_buff) {
    if (cmd_buff == NULL || cmd_buff->_cmd_buffer == NULL) {
        return ERR_MEMORY;
    }

    cmd_buff->_cmd_buffer[0] = '\0';
    
    for (int i = 0; i < CMD_ARGV_MAX; i++) {
        cmd_buff->argv[i] = NULL;
    }
    cmd_buff->argc = 0;

    return OK;
}

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
    if (cmd_line == NULL || cmd_buff == NULL || cmd_buff->_cmd_buffer == NULL) {
        return ERR_MEMORY;
    }

    clear_cmd_buff(cmd_buff);

    strncpy(cmd_buff->_cmd_buffer, cmd_line, SH_CMD_MAX - 1);
    cmd_buff->_cmd_buffer[SH_CMD_MAX - 1] = '\0';

    char *curr = cmd_buff->_cmd_buffer;
    while (*curr && isspace(*curr)) {
        curr++;
    }

    if (*curr == '\0') {
        return WARN_NO_CMDS;
    }

    cmd_buff->argc = 0;
    bool in_quotes = false;
    char *token_start = curr;
    char *write_pos = curr; 
    
    while (*curr) {
        if (*curr == '"') {
            in_quotes = !in_quotes;
            curr++;
        } else if (isspace(*curr) && !in_quotes) {
            *write_pos = '\0';
            write_pos++;
            
            if (token_start[0] != '\0') {
                if (cmd_buff->argc >= CMD_MAX) {
                    return ERR_TOO_MANY_COMMANDS;
                }
                cmd_buff->argv[cmd_buff->argc++] = token_start;
            }
            
            while (*(curr+1) && isspace(*(curr+1))) {
                curr++;
            }
            
            curr++;
            token_start = write_pos;
        } else {
            *write_pos = *curr;
            write_pos++;
            curr++;
        }
    }
    
    if (token_start < write_pos) {
        *write_pos = '\0';
        if (cmd_buff->argc >= CMD_MAX) {
            return ERR_TOO_MANY_COMMANDS;
        }
        cmd_buff->argv[cmd_buff->argc++] = token_start;
    }
    
    cmd_buff->argv[cmd_buff->argc] = NULL;
    
    return OK;
}

Built_In_Cmds match_command(const char *input) {
    if (input == NULL) {
        return BI_NOT_BI;
    }

    if (strcmp(input, EXIT_CMD) == 0) {
        return BI_CMD_EXIT;
    } else if (strcmp(input, "dragon") == 0) {
        return BI_CMD_DRAGON;
    } else if (strcmp(input, "cd") == 0) {
        return BI_CMD_CD;
    } else if (strcmp(input, "rc") == 0) {
        return BI_RC;
    }

    return BI_NOT_BI;
}

extern void print_dragon();

Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {
    if (cmd == NULL || cmd->argc == 0 || cmd->argv[0] == NULL) {
        return BI_NOT_BI;
    }

    Built_In_Cmds cmd_type = match_command(cmd->argv[0]);

    switch (cmd_type) {
        case BI_CMD_EXIT:
            exit(OK_EXIT);
            break;

        case BI_CMD_DRAGON:
            print_dragon();
            return BI_EXECUTED;

        case BI_CMD_CD:
            if (cmd->argc > 1) {
                if (chdir(cmd->argv[1]) != 0) {
                    perror("cd");
                    last_return_code = errno;
                } else {
                    last_return_code = 0;
                }
            }
            return BI_EXECUTED;

        case BI_RC:
            printf("%d\n", last_return_code);
            return BI_EXECUTED;

        default:
            return BI_NOT_BI;
    }

    return BI_NOT_BI;
}

int exec_cmd(cmd_buff_t *cmd) {
    if (cmd == NULL || cmd->argc == 0) {
        return WARN_NO_CMDS;
    }

    Built_In_Cmds result = exec_built_in_cmd(cmd);
    if (result == BI_EXECUTED) {
        return OK;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        last_return_code = errno;
        return ERR_EXEC_CMD;
    } else if (pid == 0) {
        execvp(cmd->argv[0], cmd->argv);
        exit(errno);
    } else {
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            last_return_code = WEXITSTATUS(status);
            
            if (last_return_code != 0) {
                switch (last_return_code) {
                    case ENOENT:
                        printf("Command not found in PATH\n");
                        break;
                    case EACCES:
                        printf("Permission denied\n");
                        break;
                    case ENOEXEC:
                        printf("Exec format error\n");
                        break;
                    default:
                        printf("Command failed with code %d\n", last_return_code);
                        break;
                }
                return ERR_EXEC_CMD;
            }
        } else {
            last_return_code = -1;
            printf("Command execution failed\n");
            return ERR_EXEC_CMD;
        }
    }

    return OK;
}

int exec_local_cmd_loop() {
    char cmd_line[SH_CMD_MAX];
    int rc = OK;
    cmd_buff_t cmd;

    rc = alloc_cmd_buff(&cmd);
    if (rc != OK) {
        printf("Memory allocation failed\n");
        return ERR_MEMORY;
    }

    while (1) {
        printf("%s", SH_PROMPT);
        
        if (fgets(cmd_line, SH_CMD_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }
        
        cmd_line[strcspn(cmd_line, "\n")] = '\0';
        
        rc = build_cmd_buff(cmd_line, &cmd);
        
        if (rc == WARN_NO_CMDS) {
            continue;
        } else if (rc == ERR_TOO_MANY_COMMANDS) {
            printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
            continue;
        } else if (rc != OK) {
            printf("Error parsing command\n");
            continue;
        }
        
        rc = exec_cmd(&cmd);
    }

    free_cmd_buff(&cmd);
    return OK;
}
