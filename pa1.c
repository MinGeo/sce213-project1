/**********************************************************************
 * Copyright (c) 2021
 *  Sang-Hoon Kim <sanghoonkim@ajou.ac.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTIABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>

#include "types.h"
#include "list_head.h"
#include "parser.h"
LIST_HEAD(history);	//위에 선언해 주지 않으면 run_command에서 history list를 인식하지 못한다.
struct entry {	//history 실행에 필요
	struct list_head list;
	char *command;
	int index;
};
static int __process_command(char *command);	//이걸 위에다가 선언 안해주면 run_command에서 인식을 하지 못해서 수행이 안된다
int indexplus = 0;		//append_history에서 사용될 변수
/***********************************************************************
 * run_command()
 *
 * DESCRIPTION
 *   Implement the specified shell features here using the parsed
 *   command tokens.
 *
 * RETURN VALUE
 *   Return 1 on successful command execution
 *   Return 0 when user inputs "exit"
 *   Return <0 on error
 */
static int run_command(int nr_tokens, char *tokens[])
{
	
	int pipet = 0;				//pipe command인지를 찾기 위해 파이프인지 아닌지 체크한다
	int pipeindex = 0;			//|가 위치한 곳을 저장하기 위한 변수

	for(int i = 0; i < nr_tokens; i++)
	{
		if(strcmp(tokens[i] ,"|") == 0)	//파이프 기호를 찾으면, 임시변수인 pipet에 1을 저장, pipeindex에 pipe 기호 위치 저장하고 반복문 종료
		{
			pipet = 1;
			pipeindex = i;
			break;
		}
	}
	
	if (strcmp(tokens[0], "exit") == 0) return 0;	//exit을 입력하면 프로그램 종료됨
	
	else if (strcmp(tokens[0], "cd") == 0)	//cd를 입력할때, change working directory를 수행
	{
		if (nr_tokens == 1)	//cd만 입력했을때를 체크하기 위해서, cd만 입력하면 home directory로 이동
		{
			chdir(getenv("HOME"));	//cd만 입력했을때는 홈으로 간다.
			
		}
		else {
			if(strcmp(tokens[1], "~") == 0)	//~을 입력하면 홈으로 이동
			{
				chdir(getenv("HOME"));
			}
			else {
				chdir(tokens[1]);	//입력한 디렉토리가 있다면, 입력한 디렉토리로 이동한다.
			}
		}
		return 1;
		
	}
	else if (strcmp(tokens[0], "history") == 0)//keep the command history 구현 부분
	{		//지난번 pa0의 dump_stack을 참고하였다.
		struct entry *item;	//지난번 pa0과제 처럼 하기 위해 위에 entry를 선언하고 구조체 포인터를 선언
		list_for_each_entry(item, &history, list)
		{
			fprintf(stderr, "%2d: %s", item->index, item->command);	//순서대로 먼저 넣은 순대로 출력
		}
		return 1;
	}
	else if (strcmp(tokens[0], "!") == 0)	//! command 구현 부분
	{
		struct entry *item;	//위와 같이 history에 접근해야하므로 구조체 포인터 선언
		char *temp = malloc(sizeof(char)*MAX_COMMAND_LEN+1);//구조체 포인터에 최대 길이 만큼 메모리 할당
		list_for_each_entry(item, &history, list)	//history 맨 처음부터 탐색
		{
			if(item->index == atoi(tokens[1]))	//command는 char이므로 atoi 함수를 이용해 int로 바꿔주고 index 비교
			{

				strcpy(temp, item->command);	//그냥 넣으니까 \n이 추가가 안되어 자동으로 \n을 추가해주는 strcpy함수 활용
				__process_command(temp);// 그냥 item->command를 넣을때 줄바꿈 없이 출력되는 오류가 있었다.
				
			}
		}
		return 1;
		
	}
	else {	//pipe and external command 수행하는 part
		pid_t pid;	
		int fd[2];	//file descriptor table
		int status;	//자식 프로세스가 종료될 때 상태 정보 저장
		int tempindex = 0;
		char *first_tokens[MAX_NR_TOKENS] = {NULL,};
		char *last_tokens[MAX_NR_TOKENS] = {NULL,};
		if(pipet == 1)
		{		//pipe라면, 위에서 구했던 파이프 기호의 위치를 이용해 왼쪽 command와 오른쪽 command를 쪼개준다.
			
			for(int j = 0; j < pipeindex; j++)
			{
				first_tokens[tempindex] = tokens[j];
				tempindex++;
			}
			tempindex = 0;
			for(int k = pipeindex+1; k<nr_tokens; k++)
			{
				last_tokens[tempindex] = tokens[k];
				tempindex++;
			}
			
			pipe(fd);	//pipe함수가 파이프 생성, 준비한 테이블에 저장
			
			if((pid = fork())== 0)		//fork가 성공적으로 실행되었으며 child process일때 
			{
				close(fd[0]);	//child process는 파이프로부터 읽지 않기 때문에 파이프의 읽는 쪽인 fd[0]을 닫고
				dup2(fd[1], 1);	//stdout을 pipe의 write쪽으로 연결
				execvp(*first_tokens, first_tokens);//실행
				exit(1);
			}
			else {
				//parent process
				if((pid = fork())== 0)
				{	
					close(fd[1]); //쓰는 쪽 닫아줌
					dup2(fd[0], 0);	//stdin을 pipe의 read쪽으로 연결
					execvp(*last_tokens, last_tokens);//실행
					exit(1);
				}
				else {
					close(fd[1]);	//쓰는 쪽을 닫아줌
					pid = waitpid(pid, &status, 0);	//child process가 끝날때 까지 기다린다.
				}
				
			}
		
		}
		else {				//pipe가 아닐때
			if((pid = fork()) == 0)//	fork 성공, child process
			{
				execvp(*tokens, tokens); //실행
				fprintf(stderr, "Unable to execute %s\n", *tokens);  //여기 if문에는 pipe가 아니고, 조건문으로 달아놓은 명령이 아닌 외부명령이 오기때문에, 실행 불가능한 명령이 들어올수 있다. 그 경우 실행한다
				exit(1);
			}
			else{	//parent process
				pid = waitpid(pid,&status, 0);	//child 끝날 때 까지 기다림

			}
		}
		
		return 1;	
		
	}
	
	
	
	return -EINVAL;
}


/***********************************************************************
 * struct list_head history
 *
 * DESCRIPTION
 *   Use this list_head to store unlimited command history.
 */



/***********************************************************************
 * append_history()
 *
 * DESCRIPTION
 *   Append @command into the history. The appended command can be later
 *   recalled with "!" built-in command
 */
static void append_history(char * const command)
{		//지난번 pa0의 push_stack을 참고하였다
	struct entry *item;
	item = malloc(sizeof(struct entry));	//구조체 포인터에 구조체 크기만큼 메모리 할당
	item->command = malloc(sizeof(char)*strlen(command)+1);	//커맨드의 길이 + 1 만큼 할당
	item->index = indexplus;	//가장 위에 선언해둔 indexplus 변수, 지금 history가 몇번째인지를 저장해줌
	strcpy(item->command, command);	//history에 현재 커맨드 저장
	INIT_LIST_HEAD(&item->list);
	list_add_tail(&item->list, &history);
	indexplus += 1;		//index변경 
	

}


/***********************************************************************
 * initialize()
 *
 * DESCRIPTION
 *   Call-back function for your own initialization code. It is OK to
 *   leave blank if you don't need any initialization.
 *
 * RETURN VALUE
 *   Return 0 on successful initialization.
 *   Return other value on error, which leads the program to exit.
 */
static int initialize(int argc, char * const argv[])
{
	return 0;
}


/***********************************************************************
 * finalize()
 *
 * DESCRIPTION
 *   Callback function for finalizing your code. Like @initialize(),
 *   you may leave this function blank.
 */
static void finalize(int argc, char * const argv[])
{

}


/*====================================================================*/
/*          ****** DO NOT MODIFY ANYTHING BELOW THIS LINE ******      */
/*          ****** BUT YOU MAY CALL SOME IF YOU WANT TO.. ******      */
static int __process_command(char * command)
{
	char *tokens[MAX_NR_TOKENS] = { NULL };
	int nr_tokens = 0;

	if (parse_command(command, &nr_tokens, tokens) == 0)
		return 1;

	return run_command(nr_tokens, tokens);
}

static bool __verbose = true;
static const char *__color_start = "[0;31;40m";
static const char *__color_end = "[0m";

static void __print_prompt(void)
{
	char *prompt = "$";
	if (!__verbose) return;

	fprintf(stderr, "%s%s%s ", __color_start, prompt, __color_end);
}

/***********************************************************************
 * main() of this program.
 */
int main(int argc, char * const argv[])
{
	char command[MAX_COMMAND_LEN] = { '\0' };
	int ret = 0;
	int opt;

	while ((opt = getopt(argc, argv, "qm")) != -1) {
		switch (opt) {
		case 'q':
			__verbose = false;
			break;
		case 'm':
			__color_start = __color_end = "\0";
			break;
		}
	}

	if ((ret = initialize(argc, argv))) return EXIT_FAILURE;

	/**
	 * Make stdin unbuffered to prevent ghost (buffered) inputs during
	 * abnormal exit after fork()
	 */
	setvbuf(stdin, NULL, _IONBF, 0);

	while (true) {
		__print_prompt();

		if (!fgets(command, sizeof(command), stdin)) break;

		append_history(command);
		ret = __process_command(command);

		if (!ret) break;
	}

	finalize(argc, argv);

	return EXIT_SUCCESS;
}
