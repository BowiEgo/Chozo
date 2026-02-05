# Chozo

![20260206010122_64_2.png](https://s3.bmp.ovh/2026/02/06/540pZlli.png)
![20260206012048_65_2.jpg](https://s3.bmp.ovh/2026/02/06/TtwHpCpM.jpg)

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
