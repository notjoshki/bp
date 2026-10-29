# BP

BP is a no-nonsense build tool I wrote out of frustration with the state of overly complicated build systems like Make and CMake.

> [!IMPORTANT]
> BP is a WIP project, it will be lacking features and support.

## Example

Compile all C files in a directory:

```sh
cc = gcc
cflags = -Wall -Wextra -g
out = example
srcs = find(*.c)

1: $cc $cflags -o $out $srcs
```

See the [example](./example/) directory.

## Installation

Simply run the following commands that clone the repository and run the build script:

```console
$ git clone https://github.com/notjoshki/bp.git
$ cd bp
$ ./build.sh
```

You may then copy it into your ``/usr/local/bin`` directory if you wish.

### Dependencies

All you need is a C compiler and a shell to build the executable.

## Usage

```
bp [options...] [file]
```

The default file path is ```bp.bp```.

### Options

| Option | Description |
| --- | --- |
| ```--help``` | Show help information |
| ```-q``` | Hide rule commands |

## Language Reference

The language consists of three structures: [constants](#constants), [rules](#rules) and [macros](#macros).

### Constants

Constants are values that are defined once and cannot be changed. Create constants with the equals operator and everything until the new line is stored into it.

```sh
cc = gcc
cflags = -Wall -Wextra -g
```

Reference the value of a constant by preceding it with the dollar sign operator.

```sh
message = hi
other_message = hello $message
```

### Rules

Rules are commands that are executed in order regardless of placement in the script. The rule order begins at 0, they can only be integers and the rule limit in a single script is 32. Create a rule by defining the rule order followed by a colon. Rules end at the new line.

```sh
name = josh

1: echo im
2: echo $name
0: echo hi
```

### Macros

Macros are builtin intrinsics that perform specific functions, take arguments and return values. Below is a list of all the macros.

| Macro | Description |
| --- | --- |
| ```find(path)``` | Get all files in the provided path |

## License

BP is distributed under the [BSD 3-Clause](./LICENSE) license.