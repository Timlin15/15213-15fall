# Shell lab

Writing Your Own Simple version of Unix Shell With Jobs Control

##  The `tsh` Specification & Job

Jobs:

- `eval`: Main routine that parses and interprets the command line. [70 lines] 
- `builtin_cmd`: Recognizes and interprets the built-in commands: `quit`, `fg`, `bg`, and `jobs`. [25 lines] 
- `do_bgfg`: Implements the `bg` and `fg` built-in commands. [50 lines] 
- `waitfg`: Waits for a foreground job to complete. [20 lines] 
- `sigchld_handler`: Catches SIGCHILD signals. [80 lines] 
- `sigint_handler`: Catches SIGINT (ctrl-c) signals. [15 lines] 
- `sigtstp_handler`: Catches SIGTSTP(ctrl-z) signals. [15 lines]

Spec:

- The prompt should be the string “tsh> ”. 
- The command line typed by the user should consist of a name and zero or more arguments, all separated by one or more spaces. If `name` is a built-in command, then `tsh` should handle it immediately and wait for the next command line. Otherwise, `tsh` should assume that name is the path of an executable file, which it loads and runs in the context of an initial child process (In this context, the term job refers to this initial child process). 
- `tsh` need not support pipes (|) or I/O redirection (< and >). 
- Typing ctrl-c(ctrl-z)should cause a SIGINT (SIGTSTP) signal to be sent to the current fore ground job, as well as any descendent of that job (e.g., any child processes that it forked). If there is no foreground job, then the signal should have no effect. 
- If the command line ends with an ampersand &, then `tsh` should run the job in the background. Otherwise, it should run the job in the foreground. 
- Each job can be identified by either a process ID (PID) or a job ID (JID), which is a positive integer assigned by `tsh`. JIDs should be denoted on the command line by the prefix '%'. For example, “%5” denotes JID 5, and “5” denotes PID 5. (We have provided you with all of the routines you need for manipulating the job list.) 
- `tsh` should support the following built-in commands:
    -  The `quit` command terminates the shell.
    - The `jobs` command lists all background jobs. 
    - The `bg <job>` command restarts `<job>` by sending it a SIGCONT signal, and then runs it in the background. The  argument can be either a PID or a JID.
    - The `fg <job>` command restarts by sending it a SIGCONT signal, and then runs it in the foreground. The  argument can be either a PID or a JID. 
- `tsh` should reap all of its zombie children. If any job terminates because it receives a signal that it didn't catch, then `tsh` should recognize this event and print a message with the job's PID and a description of the offending signal

##  General Overview of Unix Shells

The first word in the command line is either the name of a **built-in command** or **the pathname of an executable file**. The remaining words are command-line arguments. 

If the first word is a built-in command, the shell immediately executes the command in the current process. 

Otherwise, the word is assumed to be the pathname of an executable program. In this case, the shell forks a child process, then loads and runs the program in the context of the child. The child processes created as a result of interpreting a single command line are known collectively as a job. In general, a job can consist of multiple child processes connected by [Unix pipes]([pipe - How piping mechanism works in UNIX - Stack Overflow](https://stackoverflow.com/questions/77334496/how-piping-mechanism-works-in-unix)).

If the command line ends with an ampersand "&", then the job runs in the background, which means that the shell does not wait for the job to terminate before printing the prompt and awaiting the next command line. Otherwise, the job runs in the foreground, which means that the shell waits for the job to terminate before awaiting the next command line. 

## Concepts

### Procedure

<img src="https://wdxtub.com/images/csapp/14614185201475.jpg" alt="img"  />

在一个程序中编写`sigint_handler`等信号处理函数可以决定该程序作为进程运行的时候对该信号的反应，而使用`fork()`可以令`kernel`创建一个子程序，不需要我们编写并行逻辑。

在父进程中使用`wait(NULL)`即可等待至子进程结束再进行。

```
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();

    if (pid > 0) {
        wait(NULL); // 等待子进程结束
        printf("我是父进程，子进程结束了\n");
    } else if (pid == 0) {
        printf("我是子进程，我的 PID: %d\n", getpid());
    } else {
        printf("fork 失败！\n");
    }

    return 0;
}
```

| 发生僵尸进程的情况             | 解决方法                              |
| ------------------------------ | ------------------------------------- |
| 父进程不调用 `wait()`          | 使用 `wait()` 或 `waitpid()`          |
| 父进程长时间运行但不回收子进程 | 使用 `wait()` 或 `SIGCHLD` 处理       |
| 父进程退出但子进程已经结束     | `init` 进程会自动回收（一般无需处理） |

🔹 **最佳实践**：

- **显式调用 `wait()`**（适用于单个子进程）
- **使用 `SIGCHLD` 信号处理回收子进程**（适用于多个子进程）
- **守护进程或 `init` 进程最终会清理僵尸进程**，但不应依赖它

为了避免僵尸进程，可以**利用 `SIGCHLD` 信号**，让父进程**在子进程退出时**自动执行 `wait()`，回收子进程。

### Job

在 Unix/Linux **Shell 和终端环境**中，**Job（作业）\**是指\**在同一个 Shell 会话中运行的进程组**。一个 Job 可以是**前台**或**后台**运行的一个或多个进程的组合。

**1. Job 的基本类型**

在 Unix/Linux 终端中，作业通常有两种状态：

**1.1 前台作业（Foreground Job）**

- **默认情况下**，当你运行一个命令时，它会在前台执行，占据终端的控制权。
- 特点：
    - 需要等命令执行完成才能输入新的命令
    - 可以使用 `Ctrl + C` 终止
    - 可以用 `Ctrl + Z` 挂起

**示例**

```bash
$ sleep 60  # 这个命令会阻塞终端 60 秒
```

- 终端会被 `sleep 60` 占用，直到它执行完毕。

**1.2 后台作业（Background Job）**

- 后台作业不会占据终端的控制权，而是**在后台运行**。

- 通过 

    ```
    &
    ```

     符号启动后台作业：

    ```bash
    $ sleep 60 &  
    [1] 12345  # [作业编号] 进程 PID
    ```

- 特点：

    - 不会阻塞终端，用户可以继续输入其他命令
    - 仍然属于当前 Shell 会话
    - 终端关闭时，默认会被终止（除非使用 `nohup` 或 `disown`）

**2. Job 控制**

Unix 提供了一些命令用于**管理和控制作业**。

**2.1 `jobs`：查看当前作业**

```bash
$ jobs
[1]   Running                 sleep 60 &
[2]   Stopped                 nano test.txt
```

- `[1]` 和 `[2]` 是作业编号
- `Running` 表示后台运行
- `Stopped` 表示暂停的作业（`Ctrl + Z` 挂起）

**2.2 `bg`：让挂起的作业在后台继续运行**

如果某个作业被 `Ctrl + Z` 挂起，它会进入 `Stopped` 状态，可以用 `bg` 让它继续运行：

```bash
$ bg %1  # 让作业编号 1 继续在后台运行
```

等同于：

```bash
$ bg
```

如果只有一个挂起作业，直接输入 `bg` 即可。

**2.3 `fg`：让后台作业回到前台**

如果你想让一个后台作业恢复到前台执行，可以使用：

```bash
$ fg %1  # 将作业编号 1 切换到前台
```

**2.4 `kill`：终止作业**

```bash
$ kill %1  # 终止作业编号 1
```

或者直接使用进程 ID：

```bash
$ kill 12345  # 终止 PID 为 12345 的进程
```

**3. 进阶：守护作业**

**默认情况下，作业（Job）会在 Shell 关闭时被终止**。如果希望 Shell 关闭后仍然保持运行，可以使用：

**3.1 `nohup`（不挂起）**

```bash
$ nohup sleep 600 &
```

- `nohup` 使得进程不受 `SIGHUP` 信号影响（即终端关闭后仍然运行）。
- 输出默认重定向到 `nohup.out`，可以用 `> /dev/null 2>&1` 丢弃输出。

**3.2 `disown`（脱离 Shell）**

如果你已经启动了一个作业，并希望它不再受 Shell 影响：

```bash
$ disown -h %1
```

这样作业不会因为 Shell 退出而被终止。

**4. 总结**

| 操作                   | 命令                            | 说明                       |
| ---------------------- | ------------------------------- | -------------------------- |
| **后台运行**           | `command &`                     | 让进程在后台运行           |
| **查看作业**           | `jobs`                          | 显示当前作业               |
| **挂起作业**           | `Ctrl + Z`                      | 暂停当前作业               |
| **继续后台运行**       | `bg %n`                         | 让挂起的作业继续在后台执行 |
| **切换到前台**         | `fg %n`                         | 让后台作业恢复到前台       |
| **终止作业**           | `kill %n` / `kill PID`          | 终止作业                   |
| **防止终端关闭后结束** | `nohup command &` / `disown %n` | 让作业脱离 Shell           |

**作业（Job）只是 Shell 会话中的进程管理方式，而进程（Process）是操作系统层面的实体。**

## Design

Jobs:

- `eval`: Main routine that parses and interprets the command line. [70 lines] 
- `builtin_cmd`: Recognizes and interprets the built-in commands: `quit`, `fg`, `bg`, and `jobs`. [25 lines] 
- `do_bgfg`: Implements the `bg` and `fg` built-in commands. [50 lines] 
- `waitfg`: Waits for a foreground job to complete. [20 lines] 
- `sigchld_handler`: Catches SIGCHILD signals. [80 lines] 
- `sigint_handler`: Catches SIGINT (ctrl-c) signals. [15 lines] 
- `sigtstp_handler`: Catches SIGTSTP(ctrl-z) signals. [15 lines]

### Overall

