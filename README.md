# InjectCrash
Crashes a target process on Windows by injecting a crashy DLL into it

URL: https://github.com/MrEricSir/InjectCrash/

## Compilation

The software can be built using Visual Studio 2013, though it could be ported to other versions of Visual Studio or even mingw with a bit of time and effort. 

The debug versions of the executable and DLLs are dynamically linked against the runtime (what Microsoft confusingly calls "Multithreaded Debug DLL") and the release versions are statically linked (again, "Multithreaded".)  Depending on your use case you may need to adjust these.

## Usage

From the command line, run

`injectcrash.exe [pid]`

Where [pid] is the process identifier. You can find the process identifier on the Details tab of Task Manager under the "PID" header. The target program will crash.

For more information and other parameters, run `injectcrash.exe` without the pid parameter.

## FAQ

Q: Why would I want this?

A: There's a few use cases that might be of interest:

* You're writing a crash reporter tool and want a convenient way to test it.
* You're concerned about crash reports leaking personal data, such as your credit card number on a shopping website or financial data in tax software.
* You have an interest in DLL injection and want to look at the source code.
* You simply want to watch the world burn.

Q: What are the limitations?

A: There are several:

* For the moment it only supports two crash types, null pointer exceptions and debug breaks. More may be added in the future. 
* InjectCrash makes no effort to determine if the target process is 32 or 64 bit. It's up to the end user to compile the executable and DLLs with the desired architecture.
* The error messages can be cryptic. See the Microsoft's [System Error Codes](https://msdn.microsoft.com/en-us/library/windows/desktop/ms681381(v=vs.85).aspx) for a reference. 
* You may need to run the process as an administrator if UAC is enabled on Windows Vista or newer to crash privileged processes.

Q: Can I mess around with the source code, fork it, etc?

A: Absolutely! See the LICENSE file for details. You're also free to file a ticket on Github, see the URL at the top of this file for a link to the project. Keep in mind this is a program I wrote in my spare time for my own amusement, so if you want a significant feature request you may have to build it yourself.
