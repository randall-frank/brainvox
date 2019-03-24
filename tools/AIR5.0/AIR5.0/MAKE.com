echo cd src
cd src

# Now compile all the subroutines
for VAL in *.c
do
	echo cc -c ${VAL} -O
	cc -c ${VAL} -O
	echo ar rv libRPW.a `basename ${VAL} .c`.o
	ar rv libRPW.a `basename ${VAL} .c`.o
	echo rm `basename ${VAL} .c`.o
	rm `basename ${VAL} .c`.o
done
echo ranlib libRPW.a
ranlib libRPW.a

# Move back into main directory

echo cd ../
cd ../

# State where files will be stored
if [ $# -gt 0 ]
then
	if [ -d $1 ]
	then
		echo Programs will be stored in directory $1
	else
		echo $1 is not a directory. Programs will be stored in AIR main directory
	fi
fi

# Compile the main programs

for VAL in *.c
do
	echo cc ${VAL} -o `basename ${VAL} .c` -Lsrc -lRPW -lm -O
	cc ${VAL} -o `basename ${VAL} .c` -Lsrc -lRPW -lm -O
	
	# Move program to installation directory if specified
	if [ $# -gt 0 ]
	then
		if [ -d $1 ]
		then
			echo mv `basename ${VAL} .c` $1
			mv `basename ${VAL} .c` $1
		fi
	fi
done
