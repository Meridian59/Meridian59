cd\allart\palett~1\textures
echo Start > palcomp.txt
for %%i in (*.bmp) do palcomp %%i >> palcomp.txt
edit palcomp.txt