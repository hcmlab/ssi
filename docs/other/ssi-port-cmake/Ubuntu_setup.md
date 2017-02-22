# SSI on Ubuntu
* Tested with Xubuntu 16.04 x64 (daily)
* GCC 5.3.1

* Tested on Feb. 18 2016 by Simon and Andreas

* See "Intro.pdf" for general introduction

## Quickstart

### Install required packets

#### General build tools
``` batch
sudo apt-get install build-essential subversion cmake
```

#### Libraries for plugins
``` batch
sudo apt-get install libcairo-dev libsdl2-dev portaudio19-dev libavcodec-dev libavformat-dev libswscale-dev openjdk-8-jdk
```

### SVN checkout
``` batch
cd ~
mkdir code
cd code
mkdir SSI
cd SSI

svn checkout https://hcm-lab.de/svn/Johannes/openssi/trunk --username svnusername

```

### Build

``` batch
mkdir ssi-build
cd ssi-build
cmake ../trunk/

mkdir ../trunk/bin_cmake/Linux
make -j 8 install
```

### Run test (example)
``` batch
cd ../trunk/bin_cmake/Linux
./ssigraphic_test
```

### Run test with debugger (example)
``` batch
cd ../trunk/bin_cmake/Linux
gdb ssigraphic_test
(gdb) run
<CTRL+C>
(gdb) quit
```
