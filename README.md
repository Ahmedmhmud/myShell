# myShell

A simple command-line shell written in C using POSIX system calls.

## Features

- Execute programs found in `PATH`
- Command history with `!!` to repeat the previous command
- Concurrent execution with a trailing `&`
- Pipes with `|` (supports two commands not multiple piping)
- Input and output redirection with `<` and `>`
- Exit with the `exit` command


## Build and Run

```sh
make shell
```


### To remove the binary:

```sh
make clean
```

## Usage

Once running, the shell shows an `osh>` prompt. Enter a command and press Enter.

```bash
osh> ls -l
osh> cat file.txt | grep error
osh> echo hello > out.txt
osh> sleep 5 &
osh> !!
osh> exit
```
