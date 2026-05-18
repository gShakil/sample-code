REM https://docs.velopack.io/reference/cli/content/vpk-windows
REM https://docs.velopack.io/packaging/bootstrapping
chcp 65001
vpk pack --packId velopack_sample ^
--packVersion 1.0.5 ^
--packDir .\x64\Release ^
--mainExe velopack_sample.exe ^
--outputDir ".\outputPackage" ^
--framework vcredist142-x64 ^
--exclude ".*\.lib|.*\.pdb|.*\.exp" ^
--channel release ^
--icon .\icon_sample.ico ^
--splashImage .\splash_sample.bmp

vpk upload local --outputDir ".\outputPackage" ^
--path "\\192.168.1.11\rex_SW1팀공용\002.무인단속\publish" ^
--keepMaxReleases 10 ^
--channel release


REM --outputDir .\outputPackage ^