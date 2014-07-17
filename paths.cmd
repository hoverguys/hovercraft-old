@echo off
echo Setting paths...
SET PATH=%KATANA_ROOT%\Utl\Dev\Make;%KATANA_ROOT%\Utl\Dev\Hitachi
SET SHC_LIB=%KATANA_ROOT%\Utl\Dev\Hitachi
SET SHC_INC=%KATANA_ROOT%\Include\Shc
SET SHCPP_LIB=%SHC_LIB%
SET SHCPP_INC=%SHC_INC%
SET SHC_TMP=.
SET SHCPP_TMP=%SHC_TMP%
SET HLNK_LIBRARY1=%KATANA_ROOT%\Lib\sh4nlfzn.lib
echo Done!