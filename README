# Parabix JSON Formatter

An experimental JSON formatter built using [Parabix](https://cs-git-research.cs.surrey.sfu.ca/cameron/parabix-devel) technology developed at Simon Fraser University.

# Usage

```
json-fmt [options] [file]
```
The formatter can accept **syntactically correct** JSON input from either a file or from stdin. This is not a parser so trying to format invalid JSON won't produce any errors, just some weird output.

## Example

```
$ echo '{"a":"b"}' | json-fmt
{
  "a":"b"
}
```

# Build Instructions

The Parabix framework is required to build this program, it can be found [here](https://cs-git-research.cs.surrey.sfu.ca/cameron/parabix-devel). Along with this, a relatively recent version of LLVM (anything 4.0.0 or greater should do) and Boost are also required.

> ### Building Parabix
>
> In order to ensure that the program works correctly, Parabix should be built with `ARTIFACT_MODE=Single` and `ARTIFACT_TYPE=Static`.
>
> Example Parabix CMake configuration:
>
> ```
> cmake -DCMAKE_BUILD_TYPE=Release -DARTIFACT_MODE=Single -DARTIFACT_TYPE=Static ..
> ```

Once Parabix has been built and LLVM and Boost have been installed, the following commands can be used to build this project:

```bash
# In parabix-json-fmt repository directory
mkdir build
cd build

# Tell CMake where to find the Parabix include directory and libparabix.a library file
cmake -DPARABIX_INCLUDE=<path to parabix include dir> -DPARABIX_LIB=<path to location of libparabix.a> ..

# Build the project
make
```

* `PARABIX_INCLUDE` should be the path to wherever the `parabix-devel/include` directory is on your machine
* `PARABIX_LIB` should be the path to the directory that contains the `libparabix.a` library (this would be the directory that you built Parbix in)
