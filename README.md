# MyShell

A simple command-line shell implemented in C for Linux, demonstrating key OS concepts like process management, system calls, and command parsing. This project is designed to enhance an SDE CV by showcasing low-level system programming skills.

## Features

- **Basic Shell Functionality**: Reads user input, parses commands, and executes them.
- **Built-in Commands**: `cd`, `exit`, `help`.
- **External Commands**: Executes system commands using `fork` and `execvp`.
- **Command History**: Supports recalling previous commands with arrow keys (up/down) using the readline library.
- **Piping**: Chains commands with `|` (e.g., `ls | grep txt`).
- **I/O Redirection**: Redirects input/output with `>` and `<` (e.g., `echo hello > file.txt`).
- **Background Processes**: Runs commands in the background with `&` (e.g., `sleep 10 &`).
- **Scripting Support**: Runs shell scripts by passing a script file as an argument (e.g., `./myshell script.sh`).

## OS Concepts Demonstrated

- **Process Management**: Uses `fork` to create child processes and `waitpid` to synchronize parent-child execution.
- **System Calls**: Employs `execvp` for executing programs, `dup2` for file descriptor manipulation, and `pipe` for inter-process communication.
- **Parsing**: Tokenizes input using `strtok` to separate commands and arguments.
- **Error Handling**: Gracefully handles errors like command not found or fork failures with appropriate messages.
- **Memory Management**: Allocates and frees memory for parsed arguments to avoid leaks.

## Prerequisites

- Linux environment (tested on Fedora).
- GCC compiler.
- Readline library for command history (`libreadline-dev` on Debian/Ubuntu, `readline-devel` on Fedora).

### Installation on Fedora

```bash
sudo dnf install gcc readline-devel
```

## Compilation

Use the provided Makefile:

```bash
make
```

This compiles `myshell.c` into the executable `myshell` with warnings enabled.

To clean up:

```bash
make clean
```

## Usage

### Running the Shell

```bash
./myshell
```

You'll see the prompt `> `. Type commands and press Enter.

### Examples

- Basic commands:

  ```
  > ls
  > pwd
  > echo Hello World
  ```

- Built-ins:

  ```
  > cd /home
  > help
  > exit
  ```

- Piping:

  ```
  > ls | grep .txt
  ```

- Redirection:

  ```
  > echo "Hello" > hello.txt
  > cat < hello.txt
  ```

- Background processes:

  ```
  > sleep 5 &
  [Background process started with PID 1234]
  ```

- Command history: Use up/down arrow keys to navigate previous commands.

### Running Scripts

Create a script file, e.g., `script.sh`:

```bash
echo "Starting script"
ls
echo "Script done"
```

Run it:

```bash
./myshell script.sh
```

## Project Structure

- `myshell.c`: Main source file containing all the shell logic.
- `Makefile`: Build script for compilation.
- `README.md`: This documentation.

## Code Overview

The code is modular with separate functions for:

- `read_input()`: Reads user input with readline.
- `parse_input()`: Parses input into arguments.
- `execute_builtin()`: Handles built-in commands.
- `execute_external()`: Executes external commands with fork/exec.
- `handle_piping()`: Manages command piping.
- `handle_redirection()`: Handles I/O redirection.
- `add_to_history()`: Manages command history.
- `run_script()`: Executes commands from a file.

## Testing

Test the shell with various commands:

- Basic execution: `ls`, `pwd`.
- Built-ins: `cd /tmp`, `help`.
- Piping: `ls | wc -l`.
- Redirection: `echo test > out.txt` then `cat out.txt`.
- Background: `ping google.com &`.
- History: Type a few commands, then use arrows.
- Scripting: Create a simple script and run it.

## Future Extensions

- **Signal Handling**: Add support for Ctrl+C to interrupt processes.
- **Environment Variables**: Expand environment variable support beyond PATH.
- **Job Control**: Implement job management for background processes.
- **Advanced Parsing**: Add support for quotes, escaping, and complex expressions.
- **Multi-user Support**: Add user authentication or session management.
- **Scripting Enhancements**: Support for variables, loops, and conditionals in scripts.
