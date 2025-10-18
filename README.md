Приложение для анализа стрелковых мишеней с помощью компьютерного зрения.

## Установка

1. Установите vcpkg
```cmd
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

2. Установите OpenCV
```cmd
.\vcpkg install opencv

3. Соберите проект
```cmd
git clone https://github.com/AndreevArt/TargetLock.git
cd TargetLock
mkdir build 
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake ..
cmake --build .

После сборки
.\build\Debug\TargetAnalyzerFinal.exe
Тут же должны лежать мишени, пока что называется target.jpg(их скину в тг вам)

