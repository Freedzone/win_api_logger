# Windows APIs logger
API calls logger tool that I implemented as part of my course project. The purpose was to practice function hooking.
Can log API name (without parameters) and call timestamp.
## How does it work?
Most of Windows API calls wrappers have common 5 bytes prologue. This is very convenient as it can be replaced with 5 bytes FAR JMP to any part of the code (aka **Splicing**, please refer to [Wiki](https://en.wikipedia.org/wiki/Hooking#API/Function_Hooking/Interception_Using_JMP_Instruction_aka_splicing) for more details).

Hook handlers (a part of the code to which function flow will be redirected after hooking) are generated dynamically inside memory of target process. The only difference between handlers is function names that should be logged.

Now, after target function will be called, its name will be logged into the file.

It is possible to create more unique handlers to output function parameters, e.g. read stack to extract parameters. But it is TOO MUCH work. Just use API monitor (freeware, but closed source though).

Other possible solutions: use **Win Debugging API** (ltrace in Linux) or replace import table entries for target process with your own handlers (but it won't help if functions are called through `GetProcAdress()`)
## Implementation details
Algorithm:
* Injector starts a target application in suspended state and injects API_Logger.dll.
* DLL hooks API wrappers inside the target process.
  * Read API name from API files.
  * `GetProcAddress` of API.
  * Generate handler.
  * Replace first 5 bytes with JMP instruction to the handler.
* Process is unsuspended by the DLL.
## Limitations
Even though on XP, Win 7,8,10 most API wrappers have same size prologue (registers can vary), there are certain Windows libraries differences, e.g. no KERNELBASE in XP, KERNELBASE is calling kernel32(kernel32 is lower level wrapper that actually calls ntdll) in Win 7 and vice versa in Win 10. If overwritten instruction set is not atomic crash will occur. So every API wrapper requires analysis whether automatic (just scan first bytes of function) or manual. To sum up, this approach of api monitoring is platform sensitive. However, can be useful for analysis of debug protected applications and also can monitor calls through `GetProcAddress()`.
## How to use?
Keep injector and dll in the same folder. DLL name is hardcoded to **API_Logger.dll**. Copy APIS.txt of required libraries to the directory with executables. APIs txt file name should equal to library name (KERNELBASE, kernel32 etc.). 1 API per line. You can place `>` before API name to ignore it.
