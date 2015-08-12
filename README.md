xtrlock-cairo
-------------

Cairo frontend for X11 screen lock. It is based on xtrlock 
https://packages.debian.org/sid/xtrlock just cleanup source code, and use cairo 
to draw ;)


## Build && Install

```
./autogen.sh --prefix=/usr  \
    --enable-debug=yes
make
sudo make install
```


## Usage

```
xtrlock-cairo
```

![ScreenShot](https://raw.github.com/xiangzhai/xtrlock-cairo/master/doc/xtrlock-cairo.jpg)

then just input your password for current login user.


## dontkillme


slock http://tools.suckless.org/slock/ there is dontkillme function, but how to 
implement?
