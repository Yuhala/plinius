# Plinius Tutorial
## Summary
- `Plinius` is a secure machine learning framework which leverages `Intel SGX` for secure training of neural network models, and `persistent memory` (PM) for fault tolerance.
- Plinius consists of two main libraries: [sgx-romulus](https://github.com/anonymous-xh/sgx-romulus) which is an Intel SGX-compatible PM library we ported from Romulus PM library, and [sgx-dnet](https://github.com/anonymous-xh/sgx-dnet) which is a port of [Darknet](http://pjreddie.com/darknet) ML framework into Intel SGX.
- NB: the below instructions are for a Linux-based system (Tested on Ubuntu 18.04).

## Prerequisites
- To build and run Plinius, you must have atleast (simulation mode only) the [Intel SGX SDK](https://github.com/intel/linux-sgx) and [Intel SGX PSW](https://github.com/intel/linux-sgx) installed. 
- If your CPU supports SGX, you can additionally install the [Intel SGX driver](). Otherwise you can run SGX in `simulation mode` (not real SGX).


## System setup
- Begin by cloning this repo to your local system. Checkout the `cuso` branch.
```
$ mkdir cuso-plinius && cd cuso-plinius
$ git clone https://github.com/Yuhala/plinius.git
$ git checkout cuso

```

### Intel SGX Installations
- To install the Intel SGX SDK, PSW and driver, copy and launch the script `sgx-install.sh` in the `cuso-plinius` folder.

### Persistent memory
- If you have a machine with real persistent memory, use the following commands to format and mount the drive with DAX enabled. We assume the PM device is `/dev/pmem0`.
```
$ sudo mkdir /mnt/pmem0
$ sudo mkfs.ext4 /dev/pmem0
$ sudo mount -t ext4 -o dax /dev/pmem0 /mnt/pmem0

```
- If you do not have real PM, you can emulate it with DRAM (ramdisk), using a temporary filesystem (i.e `tmpfs`) using the following instructions.
```
$ sudo mkdir /mnt/pmem0
$ sudo mount -t tmpfs /dev/pmem0 /mnt/pmem0

```
- All plinius data will then be stored in `/mnt/pmem0/plinius_data`. Modify this path if needed in the file: [Romulus_helper.h](App/Romulus_helper.h). 
- By default, Plinius uses a `CLFLUSH` instruction for persistent write backs. You can modify this to use an optimized cache-line flush instruction like `CLFLUSHOPT` supported by your CPU. To do this, redefine the `PWB` macro in [pfences.h](Enclave/romulus/common/pfences.h) accordingly. For example: `#define PWB_IS_CLFLUSHOPT`.

## Training and testing a model in Plinius

### Intro
- As described in the paper, training a model in Plinius is summarized in the workflow below:
![workflow](imgs/workflow.png)
- For the sake of simplicity we assume RA and SC have been done successfully and the encryption key has been provisioned to the enclave.i.e `enc_key` variable in [trainer.cpp](Enclave/dnet-in/train/trainer.cpp). This is the same key used for encrypting the mnist data set.

### Preparing training data
- We created an encrypted version of the MNIST data set, located in the `App/dnet-out/data/mnist` folder.
- The encrypted images are divided into four chunks: `img.a-e`. 
- Combine the images into one file with the command: `cat img.?? > enc_mnist_imgs.data`
- `enc_mnist_imgs.data` contains 60k encrypted mnist images and `enc_mnist_labels.data` contains 60k corresponding encrypted labels.
- The images and labels are encrypted with AES-GCM encryption algorithm, with a 16 byte MAC and 12 byte IV attached to each encrypted element (e.g. image or label).
- `t10k-images*` and `t10k-labels*` represent the images and labels in the default MNIST test dataset.
- For clearer comprehension, the encrypted image and label files have the form below:
![enc_dataset](imgs/enc_mnist.png)

### Building a neural network model
- Darknet provides network configuration files to describe the structure of the model to be built and trained. 
- For this tutorial we will use the [cuso.cfg](App/dnet-out/cfg/cuso.cfg) neural network configuration. 
- The first part of the config file contains the network's hyper-parameters and the rest contains model layers. 
- Feel free to modify the config as it suits you, but make sure to follow the correct syntax (i.e Darknet config file syntax).

### Training the model
- Once the SGX, PM, and the config files are set, its time to run the application. 
- Modify the `SGX_MODE` in the `Makefile` i.e `HW` mode if you have real SGX hardware and `SIM` otherwise.
- If working in SGX simulation mode, load SGX simulation env variables with: `source /opt/intel/sgxsdk/environment`, assuming you installed the SGX SDK in `/opt/intel` which is the default folder for that installation. Otherwise, replace that part accordingly.
- Build the project using `make` command.
- Run the program using `./plinius`
- The encrypted data will be read once into PM and training will begin. You can see the loss/average loss decreasing as training proceeds.

### Fault tolerance
- Plinius uses a `mirroring mechanism` for fault tolerance: it creates a corresponding model in PM and synchronizes it with the enclave model.
- For each iteration, the training routine reads batches of encrypted data from PM, decrypts the former in the enclave, trains the model with the batch, and then mirrors-out weights to PM.
- To test the fault tolerance capabilities, interrupt the program with a `ctrl+c` and restart it again. Upon restart, training data is already in PM and training resumes from the iteration it left off.

### Inference
- Plinius is mainly designed for model training but we can do inference too. We added the default mnist test set (10k unencrypted labeled images) just for the purpose of testing the accuracy of our trained model. 
- In a real setting a programmer who wishes to do inference with Plinius will have to encrypt his inference set and load to PM following the same idea/workflow.
- We used mnist data set as a proof of concept, the same idea can be applied with a different data set once the workflow is understood.
- After training, the program will invoke `test_mnist` to test the accuracy of the trained model.
- The model obtained with the `mnist.cfg` config yields 98.5% accuracy on the 10k test set for 1 training epoch(500 iterations). We can achieve higher accuracy by modifying the network structure and learning hyperparameters.


- Have fun !!!

### Youtube presentation
- [Watch Plinius DSN presentation on youtube](https://www.youtube.com/watch?v=RVbS-zgvlhM)
