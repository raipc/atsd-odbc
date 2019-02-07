cd ..
md build
cd build
cmake .. -G "Visual Studio 15 2017 Win64" && cmake --build . -- /m || exit
::TODO: copy driver\Debug\*.dll  "C:\Program Files\ClickHouse ODBC"
ctest -V -C Debug
cd ..

md build32
cd build32
cmake .. -G "Visual Studio 15 2017" && cmake --build . -- /m || exit
::TODO: copy driver\Debug\*.dll "C:\Program Files (x86)\ClickHouse ODBC"
ctest -V -C Debug
cd ..
