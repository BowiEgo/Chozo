# Chozo

![](http://youke.xn--y7xa690gmna.cn/s1/2026/01/17/696b90b473735.webp)

## Install & Build

```console
$ git clone https://github.com/BowiEgo/Chozo
$ cd Chozo
$ git submodule update --init --recursive
$ ./tools/vcpkg/bootstrap-vcpkg.sh
$ ./tools/vcpkg/vcpkg install
```

```console
$ mkdir build
$ cmake -S . -B build
$ cmake --build build
```

or just use the shell sript contains above commands.

```console
$ git clone https://github.com/BowiEgo/Chozo
$ cd Chozo
$ chmod +x install.sh
$ ./install.sh
```

## Run

```console
$ ./bin/ChozoEditor
```
