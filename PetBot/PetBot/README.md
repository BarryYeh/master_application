# PetBot

PetBot is a pet-like robot that can be a good companion and bring joys to people’s daily lives.

When the petbot owner do some specific gestures, petbot can perform corresponding actions or behaviors, such as spinning around, tracking and stopping.

### Start
Run the following commands in terminal to setup the environments in Windows.
```
git clone https://github.com/Florrie111/PetBot.git
cd PetBot
pip install -r requirements.txt
```
The python version  in Windows environments should be at least 3.8.0.

### Run Vision Program
Please check the TODO part in ```vision.py``` and replace the txt file path.
Before running the program, plase make sure a RealSense camera is connected to your computer.
```
python3 vision.py
```

### Run Robot Program
Move ```Robot.zip``` into your Ubuntu16.04 environment.
Before you run the following commands in terminal, make sure Pioneer P3-DX is connected to your computer.
```
c++ -fPIC -g -Wall -I../include demo.cpp -o dumb -L../lib -lAria -lpthread -ldl -lrt
sudo chmod 777 /dev/ttyUSB0
./demo1 -robotPort /dev/ttyUSB0
```

