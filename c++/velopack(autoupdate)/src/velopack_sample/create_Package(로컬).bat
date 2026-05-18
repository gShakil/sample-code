REM https://docs.velopack.io/reference/cli/content/vpk-windows
REM https://docs.velopack.io/packaging/bootstrapping
chcp 65001
vpk pack --packId velopack_sample ^
--packVersion 1.0.1 ^
--packDir .\x64\Release ^
--mainExe velopack_sample.exe ^
--outputDir .\outputPackage ^
--delta BestSize ^
--framework vcredist142-x64 ^
--exclude ".*\.lib|.*\.pdb|.*\.exp" ^
--icon .\icon_sample.ico ^
--splashImage .\splash_sample.bmp