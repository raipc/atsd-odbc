## Install :

Install brew:
```brew --help &>/dev/null || /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"```


Download and prepare:
```bash
brew install git cmake
git clone --recursive https://github.com/axibase/atsd-odbc
cd atsd-odbc
```

Before build with standard libiodbc:
```
brew install libiodbc
```
Or for build with unixodbc:
```
brew install unixodbc
```

Build:
```
mkdir -p build; cd build && cmake .. && make -j $(nproc || sysctl -n hw.ncpu || echo 4)
```

edit ~/.odbc.ini:

```(ini)
[ATSD]
Driver = /Users/YOUR_USER_NAME/atsd-odbc/build/driver/libatsdodbcw.so
# Optional settings:
#Description = ATSD driver
#url=https://your-atsd-server.com:443/odbc
#username = some_user
#password = 123456
#sslmode = require
```
