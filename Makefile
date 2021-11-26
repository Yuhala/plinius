#
# Author: xxx xxxx
# Copyright (C) 2020 xxxx. All rights reserved. 
#

######## SGX SDK Settings ########

SGX_SDK ?= /opt/intel/sgxsdk
SGX_MODE ?= SIM
SGX_ARCH ?= x64
SGX_DEBUG ?= 1
ROM_CXX ?= g++-8 

ifeq ($(shell getconf LONG_BIT), 32)
	SGX_ARCH := x86
else ifeq ($(findstring -m32, $(CXXFLAGS)), -m32)
	SGX_ARCH := x86
endif

ifeq ($(SGX_ARCH), x86)
	SGX_COMMON_FLAGS := -m32
	SGX_LIBRARY_PATH := $(SGX_SDK)/lib
	SGX_ENCLAVE_SIGNER := $(SGX_SDK)/bin/x86/sgx_sign
	SGX_EDGER8R := $(SGX_SDK)/bin/x86/sgx_edger8r
else
	SGX_COMMON_FLAGS := -m64
	SGX_LIBRARY_PATH := $(SGX_SDK)/lib64
	SGX_ENCLAVE_SIGNER := $(SGX_SDK)/bin/x64/sgx_sign
	SGX_EDGER8R := $(SGX_SDK)/bin/x64/sgx_edger8r
endif

ifeq ($(SGX_DEBUG), 1)
ifeq ($(SGX_PRERELEASE), 1)
$(error Cannot set SGX_DEBUG and SGX_PRERELEASE at the same time!!)
endif
endif

ifeq ($(SGX_DEBUG), 1)
	SGX_COMMON_FLAGS += -O0 -g
else
	SGX_COMMON_FLAGS += -O2
endif

SGX_COMMON_FLAGS += -Wall -Wextra -Winit-self -Wpointer-arith -Wreturn-type \
                    -Waddress -Wsequence-point -Wformat-security \
                    -Wmissing-include-dirs -Wfloat-equal -Wundef -Wshadow \
                    -Wcast-align -Wcast-qual -Wconversion -Wredundant-decls
SGX_COMMON_CFLAGS := $(SGX_COMMON_FLAGS) -Wjump-misses-init -Wstrict-prototypes -Wunsuffixed-float-constants
SGX_COMMON_CXXFLAGS := $(SGX_COMMON_FLAGS) -Wnon-virtual-dtor -std=c++11

######## App Settings ########

ifneq ($(SGX_MODE), HW)
	Urts_Library_Name := sgx_urts_sim
else
	Urts_Library_Name := sgx_urts
endif

App_Cpp_Files := $(wildcard App/*.cpp) 
App_Include_Paths := -IInclude -IApp -I$(SGX_SDK)/include

App_C_Flags := -fPIC -Wno-attributes $(App_Include_Paths)

# Three configuration modes - Debug, prerelease, release
#   Debug - Macro DEBUG enabled.
#   Prerelease - Macro NDEBUG and EDEBUG enabled.
#   Release - Macro NDEBUG enabled.
ifeq ($(SGX_DEBUG), 1)
        App_C_Flags += -DDEBUG -UNDEBUG -UEDEBUG
else ifeq ($(SGX_PRERELEASE), 1)
        App_C_Flags += -DNDEBUG -DEDEBUG -UDEBUG
else
        App_C_Flags += -DNDEBUG -UEDEBUG -UDEBUG
endif

App_Cpp_Flags := $(App_C_Flags) $(SGX_COMMON_CXXFLAGS)
App_C_Flags += $(SGX_COMMON_CFLAGS)
App_Link_Flags := -L$(SGX_LIBRARY_PATH) -l$(Urts_Library_Name) -lpthread -lm

ifneq ($(SGX_MODE), HW)
	App_Link_Flags += -lsgx_uae_service_sim
else
	App_Link_Flags += -lsgx_uae_service
endif

App_Cpp_Objects := $(App_Cpp_Files:.cpp=.o)

App_Name := plinius



######## Darknet lib Settings ######
#--------------------------------------------------------
# dnet object directories
DNET_IN_BASE := Enclave/dnet-in
DNET_OUT_BASE := App/dnet-out
DNET_INC_OUT := -IInclude/ -I$(DNET_OUT_BASE)/src/
DNET_INC_IN := -IInclude/ -I$(DNET_IN_BASE)/src/

DNET_TRAINER_BASE := $(DNET_IN_BASE)/train

# dnet objects
OBJ_IN := activation_layer.o activations.o avgpool_layer.o batchnorm_layer.o blas.o box.o col2im.o \
		  connected_layer.o convolutional_layer.o cost_layer.o crnn_layer.o crop_layer.o deconvolutional_layer.o \
		  detection_layer.o dropout_layer.o gemm.o gru_layer.o im2col.o iseg_layer.o l2norm_layer.o layer.o \
		  list.o local_layer.o logistic_layer.o lstm_layer.o matrix.o maxpool_layer.o normalization_layer.o \
		  region_layer.o reorg_layer.o rnn_layer.o route_layer.o shortcut_layer.o softmax_layer.o tree.o \
		  upsample_layer.o utils.o image.o yolo_layer.o network.o data.o option_list.o parser.o


OBJ_OUT := blas.o box.o data.o dnet_ocalls.o image.o list.o matrix.o option_list.o parser.o tree.o utils.o data_mnist.o


DNET_OBJS_IN := $(addprefix $(DNET_IN_BASE)/obj/, $(OBJ_IN)) 

TRAINER_OBJ_IN := $(DNET_TRAINER_BASE)/trainer.o 

DNET_DEPS_IN := $(wildcard $(DNET_IN_BASE)/src/*.h) $(wildcard Include/*.h) 



DNET_OBJS_OUT := $(addprefix $(DNET_OUT_BASE)/obj/, $(OBJ_OUT))

DNET_DEPS_OUT := $(wildcard $(DNET_OUT_BASE)/src/*.h) $(wildcard Include/*.h)



#---------------------------------------------------------
######## Enclave Settings ########

ifneq ($(SGX_MODE), HW)
	Trts_Library_Name := sgx_trts_sim
	Service_Library_Name := sgx_tservice_sim
else
	Trts_Library_Name := sgx_trts
	Service_Library_Name := sgx_tservice
endif
Crypto_Library_Name := sgx_tcrypto

#---------------------------------------------------------
Rom_Folder :=  Enclave/romulus
Rom_Include_Paths := -I$(Rom_Folder)
Romulus_Cpp_Flags :=  $(Rom_Include_Paths) #-DPWB_IS_CLFLUSH
Rom_Cpp_Files:= $(Rom_Folder)/romuluslog/RomulusLogSGX.cpp $(Rom_Folder)/romuluslog/malloc.cpp $(Rom_Folder)/common/ThreadRegistry.cpp
#Rom_Cpp_Objects := $(Rom_Cpp_Files:.cpp=.o)
Rom_Cpp_Objects := RomulusLogSGX.o malloc.o ThreadRegistry.o


#-------------------------------------------------------
Enclave_Cpp_Files := Enclave/Enclave.cpp $(wildcard Enclave/mirroring/*.cpp) $(wildcard Enclave/crypto/*.cpp) 
Enclave_Include_Paths := -IInclude -IEnclave -IEnclave/romulus -IEnclave/romulus/romuluslog -I$(SGX_SDK)/include -I$(SGX_SDK)/include/tlibc -I$(SGX_SDK)/include/libcxx 

Enclave_C_Flags := -nostdinc -fvisibility=hidden -fpie -fstack-protector $(Enclave_Include_Paths)
Enclave_Cpp_Flags := $(Enclave_C_Flags) $(SGX_COMMON_CXXFLAGS) -nostdinc++
Enclave_C_Flags += $(SGX_COMMON_CFLAGS)

# Enable the security flags
Enclave_Security_Link_Flags := -Wl,-z,relro,-z,now,-z,noexecstack

# To generate a proper enclave, it is recommended to follow below guideline to link the trusted libraries:
#    1. Link sgx_trts with the `--whole-archive' and `--no-whole-archive' options,
#       so that the whole content of trts is included in the enclave.
#    2. For other libraries, you just need to pull the required symbols.
#       Use `--start-group' and `--end-group' to link these libraries.
# Do NOT move the libraries linked with `--start-group' and `--end-group' within `--whole-archive' and `--no-whole-archive' options.
# Otherwise, you may get some undesirable errors.
#
# Notes on linking sgx_switchless:
# libsgx_tswitchless.a contains some strong symbols that must override the 
# corresponding weak symbols defined in sgx_tlibc.a and sgx_trts.a. For this reason,
# -lsgx_tswitchless option must be put between -Wl,--whole-archive and 
#  -Wl,--no-whole-archive with -l$(TRTS_LIBRARY) and before the linker options 
#  for other libraries (e.g., -lsgx_tstdc).
Enclave_Link_Flags := $(Enclave_Security_Link_Flags) \
    -Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles -L$(SGX_LIBRARY_PATH) \
	-Wl,--whole-archive  -l$(Trts_Library_Name) -Wl,--no-whole-archive \
	-Wl,--start-group -lsgx_tstdc -lsgx_tcxx -l$(Crypto_Library_Name) -l$(Service_Library_Name) -Wl,--end-group \
	-Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined \
	-Wl,-pie,-eenclave_entry -Wl,--export-dynamic  \
	-Wl,--defsym,__ImageBase=0 \
	-Wl,--version-script=Enclave/Enclave.lds

Enclave_Cpp_Objects := $(Enclave_Cpp_Files:.cpp=.o)

Enclave_Name := enclave.so
Signed_Enclave_Name := enclave.signed.so
Enclave_Config_File := Enclave/Enclave.config.xml

ifeq ($(SGX_MODE), HW)
ifneq ($(SGX_DEBUG), 1)
ifneq ($(SGX_PRERELEASE), 1)
Build_Mode = HW_RELEASE
endif
endif
endif

romulus := $(Rom_Cpp_Objects)

.PHONY: all run obj

ifeq ($(Build_Mode), HW_RELEASE)
all: obj $(App_Name) $(romulus) $(Enclave_Name)
	@echo "The project has been built in release hardware mode."
	@echo "Please sign the $(Enclave_Name) first with your signing key before you run the $(App_Name) to launch and access the enclave."
	@echo "To sign the enclave use the command:"
	@echo "   $(SGX_ENCLAVE_SIGNER) sign -key <your key> -enclave $(Enclave_Name) -out <$(Signed_Enclave_Name)> -config $(Enclave_Config_File)"
	@echo "You can also sign the enclave using an external signing tool."
	@echo "To build the project in simulation mode set SGX_MODE=SIM. To build the project in prerelease mode set SGX_PRERELEASE=1 and SGX_MODE=HW."
else
all: obj $(App_Name) $(romulus) $(Signed_Enclave_Name)
endif

run: all
ifneq ($(Build_Mode), HW_RELEASE)
	@$(CURDIR)/$(App_Name)
	@echo "RUN  =>  $(App_Name) [$(SGX_MODE)|$(SGX_ARCH), OK]"
endif

######## Directory Creation ######
obj:
	mkdir -p $(DNET_IN_BASE)/obj $(DNET_OUT_BASE)/obj

######## App Objects ########


App/Enclave_u.h: $(SGX_EDGER8R) Enclave/Enclave.edl	
	@cd App && $(SGX_EDGER8R) --untrusted ../Enclave/Enclave.edl --search-path ../Enclave --search-path $(SGX_SDK)/include
	@echo "GEN  =>  $@"

App/Enclave_u.c: App/Enclave_u.h

App/Enclave_u.o: App/Enclave_u.c
	@$(CC) $(App_C_Flags) -c $< -o $@
	@echo "CC   <=  $<"
	
$(DNET_OUT_BASE)/obj/%.o: $(DNET_OUT_BASE)/src/%.c $(DNET_DEPS_OUT)
	@echo "Creating darknet c objects out.."
	@$(CC) $(DNET_INC_OUT) $(App_C_Flags) -c $< -o $@

$(DNET_OUT_BASE)/obj/%.o: $(DNET_OUT_BASE)/src/%.cpp $(DNET_DEPS_OUT)
	@echo "Creating darknet cpp objects out.."
	@$(CXX) $(DNET_INC_OUT) $(App_Cpp_Flags) -c $< -o $@



					

App/%.o: App/%.cpp App/Enclave_u.h
	@$(CXX) $(App_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

$(App_Name): App/Enclave_u.o $(App_Cpp_Objects) $(DNET_OBJS_OUT)
	@$(CXX) $^ -o $@ $(App_Link_Flags)
	@echo "LINK =>  $@"

######## Romulus Objects ########

## TODO: use wildcards once this works



######## Enclave Objects ########

Enclave/Enclave_t.h: $(SGX_EDGER8R) Enclave/Enclave.edl
	@cd Enclave && $(SGX_EDGER8R) --trusted ../Enclave/Enclave.edl --search-path ../Enclave --search-path $(SGX_SDK)/include
	@echo "GEN  =>  $@"

Enclave/Enclave_t.c: Enclave/Enclave_t.h

Enclave/Enclave_t.o: Enclave/Enclave_t.c
	@$(CC) $(Enclave_C_Flags) -c $< -o $@
	@echo "CC   <=  $<"

Enclave/%.o: Enclave/%.cpp Enclave/Enclave_t.h
	@$(CXX) $(Enclave_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

$(DNET_IN_BASE)/obj/%.o : $(DNET_IN_BASE)/src/%.c $(DNET_DEPS_IN)
	@echo "Creating darknet objects in enclave.."
	@$(CC) $(DNET_INC_IN) $(Enclave_C_Flags) -c $< -o $@


$(DNET_TRAINER_BASE)/%.o: $(DNET_TRAINER_BASE)/%.cpp $(DNET_DEPS_IN)
	@echo "Creating trainer object in enclave.."
	@$(CXX) $(DNET_INC_IN) $(Enclave_Cpp_Flags) -c $< -o $@


$(Enclave_Name): Enclave/Enclave_t.o $(Enclave_Cpp_Objects) $(Rom_Cpp_Objects) $(DNET_OBJS_IN) $(TRAINER_OBJ_IN)
	@$(CXX) $^ -o $@ $(Enclave_Link_Flags)
	@echo "LINK =>  $@"


$(romulus): $(Rom_Cpp_Files)
	$(CXX) $(Romulus_Cpp_Flags) $(Enclave_Cpp_Flags) -c $^ 


$(Signed_Enclave_Name): $(Enclave_Name)
	@$(SGX_ENCLAVE_SIGNER) sign -key Enclave/Enclave_private.pem -enclave $(Enclave_Name) -out $@ -config $(Enclave_Config_File)
	@echo "SIGN =>  $@"

.PHONY: clean 

clean:
	@rm -f $(App_Name) $(Enclave_Name) $(Signed_Enclave_Name) $(App_Cpp_Objects) App/Enclave_u.* $(Enclave_Cpp_Objects) Enclave/Enclave_t.* \
	$(Rom_Cpp_Objects) $(DNET_IN_BASE)/obj/* $(DNET_OUT_BASE)/obj/* $(DNET_TRAINER_BASE)/*.o 
