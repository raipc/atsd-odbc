
:: installer64\bin\Debug\atsd_odbc_x64.msi /quiet

copy Debug\*.dll "C:\Program Files (x86)\ATSD ODBC"
copy x64\Debug\*.dll "C:\Program Files\ATSD ODBC"
