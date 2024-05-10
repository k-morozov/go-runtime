# python3 -m venv ./env/
# pip3 install --user meson
# sudo bash -c "$(wget -O - https://apt.llvm.org/llvm.sh)"
# clang-format -style=google -dump-config > .clang-format

# pip install conan
# https://docs.conan.io/2/reference/config_files/profiles.html
# conan install . --output-folder=buildDir --build=missing --profile:build=config/myprofile --profile:host=myprofile
# meson setup --native-file buildDir/conan_meson_native.ini  buildDir .
# meson compile -C buildDir
# meson test -C buildDir

import subprocess

commands = [
    "sudo apt-get install python3 python3-pip python3-setuptools python3-wheel ninja-build",
    "python3 -m venv ./env/",
    "pip3 install --user meson",
    "sudo bash -c \"$(wget -O - https://apt.llvm.org/llvm.sh)\"",
    "echo compiler version",
    "ls -l /usr/bin | grep clang",
    "clang++ --version",
    "clang++-18 --version",
    "/usr/bin/c++ --version",
    "sudo rm /usr/bin/c++",
    "sudo ln -s /usr/bin/clang++-18 /usr/bin/c++",
    "sudo rm /usr/bin/clang++",
    "sudo ln -s /usr/bin/clang++-18 /usr/bin/clang++",
    "pip install conan",
    "ls -l",
    "conan install . --output-folder=buildDir --build=missing --profile:build=./config/debug_profile --profile:host=./config/debug_profile",
]

for cmd in commands:
    print(cmd)
    subprocess.run(cmd, shell=True, check=True)
