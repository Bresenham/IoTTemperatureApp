## ESP8266 Installation & Setup

### Install Python 2.7 (required for flashing over serial port)

- `$ sudo apt-get install python`
- `$curl https://bootstrap.pypa.io/pip/2.7/get-pip.py --output get-pip.py`
- `$ sudo python get-pip.py`
-- If this doesn't work due to `zipimport.ZipImportError: can't decompress data; zlib not available`: `$ sudo apt install zlib1g-dev zlibc`

### Download ESP8266 SDK (https://github.com/espressif/ESP8266_RTOS_SDK)

- `$ git clone https://github.com/espressif/ESP8266_RTOS_SDK.git`
- `$ wget https://dl.espressif.com/dl/xtensa-lx106-elf-gcc8_4_0-esp-2020r3-linux-amd64.tar.gz`
- `$ tar -zxvf xtensa-lx106-elf-gcc8_4_0-esp-2020r3-linux-amd64.tar.gz`
- `$ cd ~/
- `$ nano .profile` -> add `export IDF_PATH=<CurrentPath>/ESP8266_RTOS_SDK` and `export PATH=<CurrentPath>/xtensa-lx106-elf/bin/:$PATH

### Install Python requirements

- `$ cd ESP8266_RTOS_SDK/` -> `python -m pip install -r requirements.txt`
