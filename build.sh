cd asm
chmod u+x ./build.sh
./build.sh
cp hw5-asm ./../

cd ../sim/
chmod u+x ./build.sh
./build.sh
cp hw5-sim ./../
cd ..

./hw5-asm file.tk output.tko
./hw5-sim output.tko
