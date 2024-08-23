# smplog

## 3rd 引用

### 中间件
| 模块 | 名称 | 版本 | 链接  |
| ------ | ------ | ------ | ------ |
| sqlite3  | 8.1.0  | [sqlite3]() |
| SQLiteCpp| 3.3.2 | [SQLiteCpp](https://github.com/SRombauts/SQLiteCpp.git) |


#### CMake and tests
A CMake configuration file is also provided for multi-platform support and testing.

Typical generic build for MS Visual Studio under Windows (from [build.bat](build.bat)):

```Batchfile
mkdir build
cd build

cmake ..        # cmake .. -G "Visual Studio 16 2019"    # for Visual Studio 2019
@REM Generate a Visual Studio solution for latest version found
cmake -DBUILD_EXAMPLE=ON ..

@REM Build default configuration (ie 'Debug')
cmake --build .

@REM Build and run tests
ctest --output-on-failure
```

Generating the Linux Makefile, building in Debug and executing the tests (from [build.sh](build.sh)):

```Shell
mkdir Debug
cd Debug

# Generate a Makefile for GCC (or Clang, depanding on CC/CXX envvar)
cmake -DSQLITECPP_BUILD_EXAMPLES=ON -DSQLITECPP_BUILD_TESTS=ON ..

# Build (ie 'make')
cmake --build .

# Build and run unit-tests (ie 'make test')
ctest --output-on-failure
```

#### Building with meson

You can build SQLiteCpp with [meson](https://mesonbuild.com/) using the provided meson project.

you can install meson using pip: `pip install meson` however you may need to install ninja and other dependencies depending on your platform as an compiler toolchain

Arch Linux:

```sh
# install clang (compiler toolchain) and ninja (recommended build system)
sudo pacman -Syu clang ninja
# install python and pip (required for meson)
sudo pacman -Syu python python-pip
# install meson 
pip install meson
```

Ubuntu:

```sh
# install gcc(compiler toolchain) and ninja (recommended build system)
sudo apt install build-essential ninja-build
# install python and pip (required for meson)
sudo apt install python3 python3-pip
# install meson
pip install meson
```

