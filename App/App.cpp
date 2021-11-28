
#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <thread>

#include <sgx_urts.h>
#include "App.h"
#include "Enclave_u.h"
#include "ErrorSupport.h"
#include "Romulus_helper.h"

/* For romulus */
#define MAX_PATH FILENAME_MAX

#define PLINIUS_TEST_SIZE 1000

/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid = 0;

/* Romulus objects */
extern PersistentHeader *per_out;
extern uint8_t *base_addr;
extern uint8_t *real_base_addr;

/* Darknet variables */
data training_data, test_data;
size_t chunk_size;

/* Benchmarking */
#include "benchtools.h"
#include <time.h>
struct timespec start, stop;
double diff;

comm_info *comm_out;

//---------------------------------------------------------------------------------
/**
 * Config files
 */
#define IMG_SIZE 784

/* #define CONFIG_FILE "./App/dnet-out/cfg/mnist.cfg"
#define MNIST_TRAIN_IMAGES "./App/dnet-out/data/mnist/train-images-idx3-ubyte" 
#define MNIST_TRAIN_LABELS "./App/dnet-out/data/mnist/train-labels-idx1-ubyte" */

#define MNIST_TEST_IMAGES "./App/dnet-out/data/mnist/t10k-images-idx3-ubyte"
#define MNIST_TEST_LABELS "./App/dnet-out/data/mnist/t10k-labels-idx1-ubyte"

#define MNIST_CFG "./App/dnet-out/cfg/mnist.cfg"
#define MNIST_TRAIN_IMAGES "./App/dnet-out/data/mnist/enc_mnist_imgs.data"
#define MNIST_TRAIN_LABELS "./App/dnet-out/data/mnist/enc_mnist_labels.data"

/* For benchmarking */
/**
 * We can't measure time in the enclave runtime so we measure it with an ocall.
 * The transition time is very small(~5ns) compared to the times we measure (ms,s,mins etc),
 * so the values are accurate enough for our use cases
 */
void ocall_start_clock()
{
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
}
void ocall_stop_clock()
{
    clock_gettime(CLOCK_MONOTONIC_RAW, &stop);
}

void ocall_add_loss()
{
    //you can use this routine to register the loss and iteration during training in a file
    //update the loss in the enclave in comm_in->loss and the iteration in comm_in->epoch
}

//alloc space for mnist training data variable
void data_malloc(size_t chunk)
{
    //removed
}

/**
 * This ocall reads encrypted mnist data from disk to DRAM
 */
void ocall_read_disk_chunk()
{

    printf("Reading initial training data from disk\n");
    if (&training_data != NULL)
    {
        free_data(training_data);
    }
    std::string img_path = MNIST_TRAIN_IMAGES;
    std::string label_path = MNIST_TRAIN_LABELS;
    //load encrypted data into volatile matrix
    training_data = load_enc_mnist_images(img_path, NUM_IMGS_MNIST);
    training_data.y = load_enc_mnist_labels(label_path, NUM_IMGS_MNIST);
    comm_out->data_chunk = &training_data;
    printf("Done reading disk data\n");
    //ecall_set_data(global_eid, &training_data);
}

/**
 * Trains mnist in the enclave
 */
void train_mnist(char *cfgfile)
{

    list *config_sections = read_cfg(cfgfile);
    comm_out = (comm_info *)malloc(sizeof(comm_info));
    comm_out->config = config_sections;
    ecall_trainer(global_eid, comm_out->config, &training_data, chunk_size, comm_out);
    printf("Mnist training complete..\n");
    free_data(training_data);
}

void read_all_mnist_data()
{
}

/**
 * Test a trained mnist model
 * 
 */
void test_mnist(char *cfgfile)
{

    std::string img_path = MNIST_TEST_IMAGES;
    std::string label_path = MNIST_TEST_LABELS;
    data test = load_mnist_images(img_path, PLINIUS_TEST_SIZE);
    test.y = load_mnist_labels(label_path, PLINIUS_TEST_SIZE);
    list *config_sections = read_cfg(cfgfile);

    ecall_tester(global_eid, config_sections, &test, 0);
    printf("Mnist testing complete..\n");
    free_data(test);
}

//--------------------------------------------------------------------------------------------------------------

/* Do munmap and close file */
void my_ocall_close()
{
    close_file();
}

/* Initialize the enclave:
 * Call sgx_create_enclave to initialize an enclave instance
 */
int initialize_enclave(void)
{
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;

    /* Call sgx_create_enclave to initialize an enclave instance */
    /* Debug Support: set 2nd parameter to 1 */
    ret = sgx_create_enclave(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, NULL, NULL, &global_eid, NULL);
    if (ret != SGX_SUCCESS)
    {
        print_error_message(ret);
        return -1;
    }

    return 0;
}

/* Application entry */
int SGX_CDECL main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    sgx_status_t ret;

    /* Initialize the enclave */
    if (initialize_enclave() < 0)
    {
        printf("Enter a character before exit ...\n");
        getchar();
        return -1;
    }
    //Initialize sgx-rom
    rom_init();
    printf("Base addr is : %p\n", base_addr);
    ecall_init(global_eid, (void *)per_out, base_addr);

    //mnist model config file
    char cfg[128] = MNIST_CFG;

    //train a model on mnist via the Plinius workflow
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    train_mnist(cfg);
    clock_gettime(CLOCK_MONOTONIC_RAW, &stop);
    printf("Total training time: %f mins\n", time_diff(&start, &stop, SEC) / 60);

    //test the accuracy of the trained model
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    test_mnist(cfg);
    clock_gettime(CLOCK_MONOTONIC_RAW, &stop);
    printf("Total inference time: %f mins\n", time_diff(&start, &stop, SEC) / 60);

    //Destroy enclave
    sgx_destroy_enclave(global_eid);
    return 0;
}
