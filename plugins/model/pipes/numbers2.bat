..\..\..\bin\x64\vc140\xmltrain.exe -out numbers2 numbers2-tmp
..\..\..\bin\x64\vc140\xmltrain.exe -eval 0 -kfolds 2 numbers2-tmp
..\..\..\bin\x64\vc140\xmltrain.exe -eval 3 -tset data\numbers_test numbers2-tmp
