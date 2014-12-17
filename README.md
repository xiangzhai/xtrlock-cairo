xtrlock-cairo
-------------

Cairo frontend for X11 screen lock. It is based on xtrlock 
https://packages.debian.org/sid/xtrlock just cleanup source code, and use cairo 
to draw ;)


## Build && Install

```
./autogen.sh --prefix=/usr
make
sudo make install
```


## Usage

```
xtrlock-cairo
```

then just input your password for current login user.
