## Project #1: My Powerful Shell

Please do not fork this repository!

### *** Due on 11:59pm, April 15 (Friday)***

### Goal
With the system calls learned in the class and a few additional ones, you are ready to manipulate processes in the system.
Let's build my powerful shell with those system calls.


### Background
- *Shell* is a program that gets inputs from users, interpretes the inputs, and processes them accordingly. The example includes the Command Prompt in Windows, Bourne Shell in Linux, zsh in macOSX, so forth.

- An user can input a command by writing a sentence on the shell and press the "Enter" key. Upon receiving the input, the shell parses the requests into command *tokens*, and diverges execution according to the first token.

- The shell *always* assumes that the first token is the filename of the executable file to run except for `cd`, `history`, and `!` (see belows for the descriptions of those). The shell executes the executable with the rest of the tokens as the arguments. For example, if an user inputs `ls -al /home/sce213`, the shell will execute `ls` executable with `-al` and `/home/sce213` as its arguments. The shell may execute the executable using the p-variant of the `exec()` system call family so that the executable file is automatically checked from the *`$PATH`* environment variable.


### Problem Specification
- The shell program `posh` (**PO**werful **SH**ell) awaits your command line input after printing out "$" as the prompt. When you enter a line of command, the framework tokenizes the command line with `parse_command()` function, and the framework calls `run_command()` function with the tokens. You need to implement following features starting in `run_command()`.

- Currently, the shell keeps getting input commands and processes them until the user enters `exit`. In that case, the shell program exits.


#### Execute external commands (40 pts)
- When the shell gets a command, it should **run the executable** as explained in Background above. Each command can be comprised of one exeutable followed by zero or more arguments. For example;

  ```bash
  $ /bin/ls
  list_head.h  Makefile  pa1.c  parser.c  parser.h  README.md  types.h
  $ ls
  list_head.h  Makefile  pa1.c  parser.c  parser.h  README.md  types.h
  $ pwd
  /home/jonghyeon/os/pa1
  $ cp pa1.c pa1-backup.c
  $ ls
  list_head.h  Makefile  pa1-backup.c  pa1.c  parser.c  parser.h  README.md  types.h
  $ exit
  ```
- Your task is to **execute external executables** (such as `ls`, `pwd`, and `cp`), **NOT** to implement the function of each command.

- When the specified executable cannot be executed for some reasons, print out the following message to `stderr`.

  ```C
  if (unable to execute the specified command) {
    fprintf(stderr, "Unable to execute %s\n", the first token of the command);
  }

  $ blah blahh
  Unable to execute blah
  $
  ```

- Use `toy` program which is included in the template code for your development and testing. It simply prints out the arguments it receives, so you can check whether your implementation handles input commands properly.

  ```bash
  $ ./toy arg1 arg2 arg3 arg4
  pid  = xxxxxx
  argc = 5
  argv[0] = ./toy
  argv[1] = arg1
  argv[2] = arg2
  argv[3] = arg3
  argv[4] = arg4
  done!
  ```

- Hint: `fork(), exec(), wait(), waitpid()`


#### Change working directory (20 pts)
- The shell has so-called *current working directory* on which the shell is on. All operations are processed on the current working directory by default, and you can check the current working directory with `/bin/pwd`. This is similar to what happens when you browse folders with the Explorer; when you select "New Folder", a new folder will be created on the "current" folder. You can change the current folder by selecting one of folders in the directory.

- Implement `cd`, a special command manipulating the current working directory. This command is special in that this feature is not handled by executing executables but the shell understands the command and processes itself. In this sense, this type of command is called a *built-in command*.

- Each user has one's *home directory* which is denoted by `~`. The actual path is defined in `$HOME` environment variable. Make `cd` command to understand it

  ```bash
  $ pwd
  /home/directory/of/your/account  # Assume this is the home directory of the user
  $ cd /somewhere/i/dont/know
  $ pwd
  /somewhere/i/dont/know
  $ cd ~
  $ pwd
  /home/directory/of/your/account
  $ cd /somewhere/i/know
  $ pwd
  /somewhere/i/know
  $ cd   # Equivalent to cd ~
  $ pwd
  /home/directory/of/your/account
  ```

- Hints: `chdir(), getenv("HOME")`


#### Keep the command history (40 pts)
- It would be awesome if the shell remembers all your past commands and allows to run some easily, isn't it? So, the instructor initially designed the framework to support the nice feature, but could not finish by the deadline. Your task is to complete this feature.

- When the framework receives a line of command, it invokes `append_history()` before processing the command. Complete the function to maintain the command history. You must use `struct list_head history` to keep the history. Do not access `prev` and `next` of `struct list_head` directly as of PA0.

- Your implementation should be able to hold unlimited history entries. Also, each command can be up to *some* size limit, which should be considered during allocating the buffer for command strings.

- When the user enters `history` in the prompt, print out the history of commands in the following format. Note that `history` will be a built-in command since the shell needs to process this command by itself.

  ```
  fprintf(stderr, "%2d: %s", index, command);
  ```

- Allow the user to execute n-th command in the history with `! <number>`. See below example

  ```bash
  $ ls
  list_head.h  Makefile  pa1.c  pa1.o  parser.c  parser.h  parser.o  posh  README.md  subdir  testcases  toy  toy.c  toy.o  types.h
  $ cp pa1.c temp.c
  $ ls -al
  ...
  $ cat types.h parser.h
  $ history
   0: ls
   1: cp pa1.c temp.c
   2: ls -al
   3: cat types.h parser.h
   4: history
  $ ! 0   # Rerun 0-th command in the history, which is "ls".
  list_head.h  Makefile  pa1-backup.c  pa1.c  pa1.o  parser.c  parser.h  parser.o  posh  README.md  subdir  temp.c  testcases  toy  toy.c  toy.o	types.h
  $ history
   0: ls
   1: cp pa1.c temp.c
   2: ls -al
   3: cat types.h parser.h
   4: history
   5: ! 0
   6: history
  $ ! 4   # 4-th command in the history is "history"
   0: ls
   1: cp pa1.c temp.c
   2: ls -al
   3: cat types.h parser.h
   4: history
   5: ! 0
   6: history
   7: ! 4
  $ ! 7  # 7-th entry is "! 4", which is "history"
   0: ls
   1: cp pa1.c temp.c
   2: ls -al
   3: cat types.h parser.h
   4: history
   5: ! 0
   6: history
   7: ! 4
   8: ! 7
  $
  ```

- Hint: Recall the exercise done as PA0.


#### Connect two processes with a pipe (100 pts)
- As we discussed in the class, we can connect two processes with an ordinary pipe. Implement this feature.

- The user may enter two commands with the pipe symbol `|` in between. All output of the first command should be carried to the input of the second command.

  ```bash
  $ cat pa1.c | sort -n
  $ echo hello_my_cruel_world | cut -c2-5
  ```

- Note that the shell should be *sane* after processing the pipe.

- Hints
  - Use `pipe()` and `dup2()`.
  - Implement incrementally. First check whether the pipe symbol exists in the tokens. If not, just do execute the command. If exists, split the tokens into two pars and feed them to **two** different processes which are connected through a pipe.



### Restriction and hints
- For your coding practice, the compiler is set to halt on some (important) warnings. Write your code to fully comply the C99 standard.
- DO NOT USE `system()` system call. You will get 0 pts if you use it.
- DO NOT implement external programs' features by yourself (e.g., printing out a message to handle `echo` command, listing the current directory to handle `ls` command, etc). You will not get any point in this case.
- It is advised to test your code on your computer first and to implement incrementally. Some sample inputs are included under `testcase` directory. Try to input each line or just run `./posh < [input file]`.


### Submission / Grading
- 200 pts in total
- Source: ***pa1.c*** (200 pts in total)
  - Points will be prorated by testcase results.
- Free to make a question through AjouBb. However, **YOU MIGHT NOT GET AN ANSWER IF THE ISSUE/TOPIC IS ALREADY DISCUSSED ON THIS HANDOUT**.
- **QUESTIONS OVER EMAIL WILL BE IGNORED unless it concerns your privacy**.
