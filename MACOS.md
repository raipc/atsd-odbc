# Install

Install brew:
```brew --help &>/dev/null || /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"```

Download and prepare:

```bash
brew install git cmake
git clone --recursive https://github.com/axibase/atsd-odbc
cd atsd-odbc
```

Before build with standard libiodbc:

```bash
brew install libiodbc
```

Or for build with unixodbc:

```bash
brew install unixodbc
```

Build:

```bash
mkdir -p build; cd build && cmake .. && make -j $(nproc || sysctl -n hw.ncpu || echo 4)
```

edit ~/.odbc.ini:

```(ini)
[ATSD]
Driver = /Users/YOUR_USER_NAME/atsd-odbc/build/driver/libatsdodbcw.so
# Optional settings:
#Description = ATSD driver
#url=https://atsd.example.org:8443/odbc
#username = john.doe
#password = secret
#sslmode = require
```
