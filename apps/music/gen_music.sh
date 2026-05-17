
echo "#include \"notas.h\" "
echo "void melody_note(unsigned int frequency, double duration);"
echo "void cancion() {"
cat starwars_main.h | grep NOTE | while read linea ; do
	NOTA=$(echo $linea | cut -f1 -d\ )
	DUR=$(echo $linea | cut -f2 -d\ )
	echo "melody_note(${NOTA} , 2400 / ${DUR} );"
	# echo "melody_note(${NOTA} , 60000*4/100 / ${DUR} );"
done
echo "}"
