REM https://docs.velopack.io/reference/cli/content/vpk-windows
REM https://docs.velopack.io/packaging/bootstrapping
chcp 65001
vpk pack --packId velopack_sample ^
--packVersion 1.0.2 ^
--channel release ^
--packDir .\x64\Release ^
--mainExe velopack_sample.exe ^
--outputDir "\\192.168.1.11\rex_SW1팀공용\002.무인단속\publish" ^
--framework vcredist142-x64 ^
--exclude ".*\.lib|.*\.pdb|.*\.exp" ^
--icon .\icon_sample.ico ^
--splashImage .\splash_sample.bmp


REM --outputDir .\outputPackage ^