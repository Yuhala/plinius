
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

/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid = 0;
static int stack_val = 10;

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
std::vector<double> sup_times;
std::vector<double> chkpt_times;
std::vector<double> loss;
std::vector<int> epoch;

//---------------------------------------------------------------------------------
/**
 * Config files
 */
#define IMG_SIZE 784

#define CONFIG_BASE "./App/dnet-out/cfg/batch/"

#define CIFAR_CFG_FILE "./App/dnet-out/cfg/"
#define CIFAR_TEST_DATA "./App/dnet-out/data/cifar/cifar-10-batches-bin/test_batch.bin"
#define TINY_IMAGE "./App/dnet-out/data/person.jpg"
#define TINY_CFG "./App/dnet-out/cfg/tiny.cfg"
#define DATA_CFG "./App/dnet-out/data/tiny.data"
#define MNIST_TRAIN_IMAGES "./App/dnet-out/data/mnist/train-images-idx3-ubyte"
#define MNIST_TRAIN_LABELS "./App/dnet-out/data/mnist/train-labels-idx1-ubyte"
#define MNIST_TEST_IMAGES "./App/dnet-out/data/mnist/t10k-images-idx3-ubyte"
#define MNIST_TEST_LABELS "./App/dnet-out/data/mnist/t10k-labels-idx1-ubyte"
#define MNIST_CFG "./App/dnet-out/cfg/mnist.cfg"
#define TEST_CFG "./App/dnet-out/cfg/cfg.cfg"
#define MNIST_WEIGHTS "./App/dnet-out/backup/mnist.weights"
#define NV_MODEL "/dev/shm/romuluslog_shared"
#define SAVE "./results/save.csv"
#define RESTORE "./results/restore.csv"
#define CRASH "./results/crash.csv"
#define NOCRASH "./results/nocrash.csv"
#define BATCH "./results/batch.csv"
#define CRASH_TESTx
std::ofstream file;

/* For benchmarking */
void ocall_start_clock()
{
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
}
void ocall_stop_clock()
{
    clock_gettime(CLOCK_MONOTONIC_RAW, &stop);
    //printf("Ocall time: %f\n", time_diff(&start, &stop, MILLI));
    if (comm_out->chkpt == 1)
    {
        chkpt_times.push_back(time_diff(&start, &stop, MILLI));
    }
    else
    {
        sup_times.push_back(time_diff(&start, &stop, MILLI));
    }
}

void ocall_add_loss()
{
    //TODO
}

//alloc space for mnist training data variable
void data_malloc(size_t chunk)
{
    training_data = {0};
    training_data.shallow = 0;
    matrix X = make_matrix(chunk, IMG_SIZE);
    matrix Y = make_matrix(chunk, NUM_CLASSES);
    training_data.X = X;
    training_data.y = Y;
}

void run_full()
{

    comm_out = (comm_info *)malloc(sizeof(comm_info));
    //std::string img_path = MNIST_TRAIN_IMAGES;
    //std::string label_path = MNIST_TRAIN_LABELS;
    //training_data = load_mnist_images(img_path, NUM_IMGS_MNIST);
    //training_data.y = load_mnist_labels(label_path, NUM_IMGS_MNIST);
    //Results files
    std::ofstream file;

    //write headers of results files
    file.open(BATCH, std::ios::app);

    file << "batch_size,iter_time(ms)\n";
    file.flush();
    int counter = 1;
    for (int i = 8; i <= 1024; i = i * 2)
    {
        chunk_size = i; //double the chunk size for each full training operation
        char base[] = CONFIG_BASE;
        char cfg[128];
        //int num = 0;
        snprintf(cfg, sizeof(cfg), "%scfg%d.cfg", base, counter);
        counter++;
        //printf("cfg: %s\n", cfg);

        //part b: full training with mirroring and data remains in pmem
        comm_out->chkpt = 0;
        comm_out->sup = 1;
        comm_out->model_size = 0;
        data_malloc(NUM_IMGS_MNIST);

        printf("Pmem based training: \n");
        clock_gettime(CLOCK_MONOTONIC_RAW, &start);
        train_mnist(cfg);
        //test_mnist(cfg);
        clock_gettime(CLOCK_MONOTONIC_RAW, &stop);
        double sup_time = time_diff(&start, &stop, SEC);
        //sup_time /= 60000; //time in minutes
        printf("Iteration time: %f\n", sup_time);
        free_data(training_data);
        file << i << "," << sup_time << "\n";
        file.flush();
    }
    //free_data(training_data);
    file.close();
}
void ocall_read_disk_chunk()
{ //Here our chunk is all the 60k mnist training images
    printf("Reading initial training data from disk\n");
    if (&training_data != NULL)
    {
        free_data(training_data);
    }
    std::string img_path = MNIST_TRAIN_IMAGES;
    std::string label_path = MNIST_TRAIN_LABELS;
    training_data = load_mnist_images(img_path, NUM_IMGS_MNIST);
    training_data.y = load_mnist_labels(label_path, NUM_IMGS_MNIST);
    comm_out->data_chunk = &training_data;
    printf("Done reading disk data\n");
    //ecall_set_data(global_eid, &training_data);
}

//------------------------------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------------------
/**
 * Train mnist classifier inside the enclave
 * mnist: digit classification
 */
void train_mnist(char *cfgfile)
{

    list *sections = read_cfg(cfgfile);
    //comm = (comm_info *)malloc(sizeof(comm_info)); //
    ecall_trainer(global_eid, sections, &training_data, chunk_size, comm_out);
    printf("Mnist training complete..\n");
    //free_data(training_data); //
}

void read_all_mnist_data()
{
    std::string img_path = MNIST_TRAIN_IMAGES;
    std::string label_path = MNIST_TRAIN_LABELS;
    training_data = load_mnist_images(img_path, NUM_IMGS_MNIST);
    training_data.y = load_mnist_labels(label_path, NUM_IMGS_MNIST);
    ecall_trainer(global_eid, NULL, &training_data, 0, NULL);
    free_data(training_data);
}

/**
 * Test a trained mnist model
 * Define path to weighfile in trainer.c
 */
void test_mnist(char *cfgfile)
{

    std::string img_path = MNIST_TEST_IMAGES;
    std::string label_path = MNIST_TEST_LABELS;
    data test = load_mnist_images(img_path, 10000);
    test.y = load_mnist_labels(label_path, 10000);
    list *sections = read_cfg(cfgfile);

    ecall_tester(global_eid, sections, &test, 0);
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

    //Initialize rom persistent mem
    rom_init();
    printf("Base addr is : %p\n", base_addr);
    ecall_init(global_eid, (void *)per_out, base_addr);

    /* Benchmarking stuff */
    run_full();
    //read_all_mnist_data();

    //Destroy enclave
    sgx_destroy_enclave(global_eid);
    return 0;
}
