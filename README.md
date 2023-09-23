#SOPL

##Introduction
SOPL is an esoteric/weird programming language. It is of course not intended for mainstream programming. Trying to solve a program using it should rather be viewed as a kind of brain-teaser. SOPL isn't as hard as BF or even Malbolge, so it should in principle be possible to solve your problem. However, the solution will often not be as straightforward or obvious as with a mainstream language...

The main reason I invented SOPL is beauty. Since beauty lies in the eyes of the beholder, you may find ugly what I think is beautiful. The aim is to have a language that produces code more close to natural languages as is normally the case. I admit that SOPL achieves this aim only partially. Therefore I have plans for a successor language that comes more close to the aim but will also be much less intelligible for the ordinary reader.

To get acquainted with the programming style needed for SOPL programs you should look at some sample programs - see remarks below - and read the documentation (in pdf format).

The repository contains an interpreter written in C++ as well as sample programs.

##Build
You should compile the source code with C++17 or later. To avoid crashes for programs making heavy use of recursion increase heap and stack by setting the appropriate linker options. I use the following settings:
-Wl,--stack,700000000
-Wl,--heap,500000000
Note that I still prefer to include debug code and suggest you do the same when compiling the code. 
###Windows
You should have GCC (including the compiler g++) installed first. Then run the build script:

```batch
make.bat
```

Alternatively, create directories `obj`, `obj/src`, `obj/Debug`, `obj/Debug/src`, `bin` and `bin/Debug`, then execute the following commands

```batch
g++.exe -Wall -fexceptions -g -std=c++17 -Og -Iinclude -c main.cpp -o obj\Debug\main.o
g++.exe -Wall -fexceptions -g -std=c++17 -Og -Iinclude -c src\Basics.cpp -o obj\Debug\src\Basics.o
g++.exe -Wall -fexceptions -g -std=c++17 -Og -Iinclude -c src\Datetime.cpp -o obj\Debug\src\Datetime.o
g++.exe -Wall -fexceptions -g -std=c++17 -Og -Iinclude -c src\globals.cpp -o obj\Debug\src\globals.o
g++.exe -Wall -fexceptions -g -std=c++17 -Og -Iinclude -c src\Interpreter.cpp -o obj\Debug\src\Interpreter.o
g++.exe -Wall -fexceptions -g -std=c++17 -Og -Iinclude -c src\Item.cpp -o obj\Debug\src\Item.o
g++.exe -Wall -fexceptions -g -std=c++17 -Og -Iinclude -c src\Lexer.cpp -o obj\Debug\src\Lexer.o
g++.exe -Wall -fexceptions -g -std=c++17 -Og -Iinclude -c src\Log.cpp -o obj\Debug\src\Log.o
g++.exe -Wall -fexceptions -g -std=c++17 -Og -Iinclude -c src\Opcode.cpp -o obj\Debug\src\Opcode.o
g++.exe -Wall -fexceptions -g -std=c++17 -Og -Iinclude -c src\Paragraph.cpp -o obj\Debug\src\Paragraph.o
g++.exe -Wall -fexceptions -g -std=c++17 -Og -Iinclude -c src\Parser.cpp -o obj\Debug\src\Parser.o
g++.exe -Wall -fexceptions -g -std=c++17 -Og -Iinclude -c src\Preprocess.cpp -o obj\Debug\src\Preprocess.o
g++.exe -Wall -fexceptions -g -std=c++17 -Og -Iinclude -c src\Rowitem.cpp -o obj\Debug\src\Rowitem.o
g++.exe -Wall -fexceptions -g -std=c++17 -Og -Iinclude -c src\Second.cpp -o obj\Debug\src\Second.o
g++.exe -Wall -fexceptions -g -std=c++17 -Og -Iinclude -c src\Sentence.cpp -o obj\Debug\src\Sentence.o
g++.exe -Wall -fexceptions -g -std=c++17 -Og -Iinclude -c src\Third.cpp -o obj\Debug\src\Third.o
g++.exe  -o bin\Debug\SOPL.exe obj\Debug\main.o obj\Debug\src\Basics.o obj\Debug\src\Datetime.o obj\Debug\src\globals.o obj\Debug\src\Interpreter.o obj\Debug\src\Item.o obj\Debug\src\Lexer.o obj\Debug\src\Log.o obj\Debug\src\Opcode.o obj\Debug\src\Paragraph.o obj\Debug\src\Parser.o obj\Debug\src\Preprocess.o obj\Debug\src\Rowitem.o obj\Debug\src\Second.o obj\Debug\src\Sentence.o obj\Debug\src\Third.o  -Wl,--stack,700000000 -Wl,--heap,500000000  
```

After successfully compiling you should be all set. Try executing a sample program like so:

```batch
bin/Debug/SOPL.exe -x -ext samples/hello_world.sopl
```

### macOS/Linux
This requires `clang++` to be installed. Run the build script:

```shell
./make.sh
```

Alternatively, create directories `obj`, `obj/src`, `obj/Debug`, `obj/Debug/src`, `bin` and `bin/Debug`, then execute the following commands

```shell
clang++ -Wall -fexceptions -g -std=c++17 -Og -Iinclude -c main.cpp -o obj/Debug/main.o
clang++ -Wall -fexceptions -g -std=c++17 -Og -Iinclude -c src/Basics.cpp -o obj/Debug/src/Basics.o
clang++ -Wall -fexceptions -g -std=c++17 -Og -Iinclude -c src/Datetime.cpp -o obj/Debug/src/Datetime.o
clang++ -Wall -fexceptions -g -std=c++17 -Og -Iinclude -c src/globals.cpp -o obj/Debug/src/globals.o
clang++ -Wall -fexceptions -g -std=c++17 -Og -Iinclude -c src/Interpreter.cpp -o obj/Debug/src/Interpreter.o
clang++ -Wall -fexceptions -g -std=c++17 -Og -Iinclude -c src/Item.cpp -o obj/Debug/src/Item.o
clang++ -Wall -fexceptions -g -std=c++17 -Og -Iinclude -c src/Lexer.cpp -o obj/Debug/src/Lexer.o
clang++ -Wall -fexceptions -g -std=c++17 -Og -Iinclude -c src/Log.cpp -o obj/Debug/src/Log.o
clang++ -Wall -fexceptions -g -std=c++17 -Og -Iinclude -c src/Opcode.cpp -o obj/Debug/src/Opcode.o
clang++ -Wall -fexceptions -g -std=c++17 -Og -Iinclude -c src/Paragraph.cpp -o obj/Debug/src/Paragraph.o
clang++ -Wall -fexceptions -g -std=c++17 -Og -Iinclude -c src/Parser.cpp -o obj/Debug/src/Parser.o
clang++ -Wall -fexceptions -g -std=c++17 -Og -Iinclude -c src/Preprocess.cpp -o obj/Debug/src/Preprocess.o
clang++ -Wall -fexceptions -g -std=c++17 -Og -Iinclude -c src/Rowitem.cpp -o obj/Debug/src/Rowitem.o
clang++ -Wall -fexceptions -g -std=c++17 -Og -Iinclude -c src/Second.cpp -o obj/Debug/src/Second.o
clang++ -Wall -fexceptions -g -std=c++17 -Og -Iinclude -c src/Sentence.cpp -o obj/Debug/src/Sentence.o
clang++ -Wall -fexceptions -g -std=c++17 -Og -Iinclude -c src/Third.cpp -o obj/Debug/src/Third.o
clang++  -o bin/Debug/sopl obj/Debug/main.o obj/Debug/src/Basics.o obj/Debug/src/Datetime.o obj/Debug/src/globals.o obj/Debug/src/Interpreter.o obj/Debug/src/Item.o obj/Debug/src/Lexer.o obj/Debug/src/Log.o obj/Debug/src/Opcode.o obj/Debug/src/Paragraph.o obj/Debug/src/Parser.o obj/Debug/src/Preprocess.o obj/Debug/src/Rowitem.o obj/Debug/src/Second.o obj/Debug/src/Sentence.o obj/Debug/src/Third.o  -Wl,-stack_size -Wl,0x30000000 -Wl,-stack_size -Wl,0x20000000  
```

After successfully compiling you should be all set. Try executing a sample program like so:

```shell
bin/Debug/sopl -x -ext samples/hello_world.sopl
```

##Structure
The basic structure of an SOPL program is as follows:
- It starts with a header, basically consisting of a paragraph call and ending with an empty line.
- Then a series of paragraphs follows, each paragraph is introduced with its name followed by a colon on a separate line and a subsequent series of sentences. Paragraphs are terminated by an empty line.
- Sentences are series of words where the last word is immediately followed by a dot. The last word of a sentence is considered a verb and equivalent to a function name while the preceding words serve as parameters.
- Essentially each sentence is regarded as an expression and has the value of the evaluated expression.
- You may reference values of preceding sentences (inside the same paragraph) by special reference words like 'this', 'that' etc.

For a thorough description of the language please see the PDF documentation in the docs subdirectory.

##Design
The only design principle I am trying to follow is KISS: Keep it simple and stupid. Sure the code isn't always as stupid as it could be but I keep trying...

##Samples
The 'samples' directory lists the solutions to a few 'standard' tasks. Might be extended in the future... 

There is also a 'features' directory which contains small programs that mostly do not have a meaning in themselves but simply demonstrate the features of the language. You should make yourself familar with the examples listed there and try to understand the results by running them to get a better understanding of the language.

##Issues
If you find that something is wrong or should be changed for whatever reason please submit an issue. I will usually try to react in a week or so but if I don't that may probably be due to private reasons.
