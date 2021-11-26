
#include "dnet_sgx_utils.h"
#include "darknet.h"
#include "trainer.h"
#include "mirroring/dnet_mirror.h"
#include "mirroring/nvdata.h"
#include "checks.h"
#include "plinius_common.h"

#define NUM_ITERATIONS 10

#define LOG_FREQ 1

comm_info *comm_in = nullptr;
NVData *pm_data = nullptr;
data train;
size_t batch_size = 0;
int count;

//define enc_key; this will be provisioned via remote attestation
unsigned char enc_key[16] = {0x76, 0x39, 0x79, 0x24, 0x42, 0x26, 0x45, 0x28, 0x48, 0x2b, 0x4d, 0x3b, 0x62, 0x51, 0x5e, 0x8f};

//global network model
network *net = nullptr;
NVModel *nv_net = nullptr;

/**
 * Peterson Yuhala
 * The network training avg accuracy should decrease
 * as the network learns
 * Batch size: the number of data samples read for one training epoch/iteration
 * If accuracy not high enough increase max batch
 */

//allocate memory for training data variable
data data_alloc(size_t batch_size)
{
    data temp;
    temp = {0};
    temp.shallow = 0;
    matrix X = make_matrix(batch_size, IMG_SIZE);
    matrix Y = make_matrix(batch_size, NUM_CLASSES);
    temp.X = X;
    temp.y = Y;
    return temp;
}
void ecall_set_data(data *data)
{
    train = *data;
}
//removes pmem net
void rm_nv_net()
{
    printf("Removing PM model\n");
    nv_net = romuluslog::RomulusLog::get_object<NVModel>(0);
    if (nv_net != nullptr)
    {
        TM_PFREE(nv_net);
        romuluslog::RomulusLog::put_object<NVModel>(0, nullptr);
    }
}
//sets pmem training data: for testing purposes with unencrypted data
void set_nv_data(data *tdata)
{
    pm_data = romuluslog::RomulusLog::get_object<NVData>(1);
    if (pm_data == nullptr)
    {
        pm_data = (NVData *)TM_PMALLOC(sizeof(struct NVData));
        romuluslog::RomulusLog::put_object<NVData>(1, pm_data);
        pm_data->alloc();
    }

    if (pm_data->data_present == 0)
    {
        pm_data->fill_pm_data(tdata);
        printf("---Copied training data to pmem---\n");
    }
    //comm training data to nv data
    //train = (data)malloc(sizeof(data));
    // pm_data->shallow_copy_data(&train);
}
void load_pm_data()
{
    pm_data = romuluslog::RomulusLog::get_object<NVData>(1);
    if (pm_data == nullptr)
    {
        printf("---Allocating PM data---\n");
        pm_data = (NVData *)TM_PMALLOC(sizeof(struct NVData));
        romuluslog::RomulusLog::put_object<NVData>(1, pm_data);
        pm_data->alloc();
    }
    if (pm_data->data_present == 0)
    {
        //ocall to copy encrypted data into enclave
        ocall_read_disk_chunk();
        printf("Copying encrypted training data in PM\n");
        pm_data->fill_pm_data(comm_in->data_chunk);
        printf("---Copied training data to PM---\n");
    }

    return;
}
void get_pm_batch()
{
    pm_data = romuluslog::RomulusLog::get_object<NVData>(1);
    if (pm_data == nullptr)
    {
        printf("No PM data\n");
        abort(); //abort training
    }

    if (count % LOG_FREQ == 0)
    {
        //print this every 10 iters
        printf("Reading and decrypting batch of: %d from PM\n", batch_size);
    }
    pm_data->deep_copy_data(&train, batch_size);
    //printf("Obtained data batch from PM\n");
}
void ecall_trainer(list *sections, data *training_data, int bsize, comm_info *info)
{
    CHECK_REF_POINTER(sections, sizeof(list));
    CHECK_REF_POINTER(training_data, sizeof(data));
    CHECK_REF_POINTER(info, sizeof(comm_info));
    /**
     * load fence after pointer checks ensures the checks are done 
     * before any assignment 
     */
    sgx_lfence();
    //fill pmem data if absent
    /* if (sections == NULL)
    {
        set_nv_data(training_data);
        return;
    } */

    comm_in = info;
    //rm_nv_net();

    train_mnist(sections, training_data, bsize);
}

/**
 * Training algorithms for different models
 */

void train_mnist(list *sections, data *training_data, int pmem)
{

    PLINIUS_INFO("------Training mnist in enclave..----------\n");

    srand(12345);
    float avg_loss = 0;
    float loss = 0;
    int classes = 10;
    int N = 60000; //number of training images
    int cur_batch = 0;
    float progress = 0;
    count = 0;
    int chunk_counter = 0;

    unsigned int num_params;
    //allocate enclave model
    net = create_net_in(sections);

    //mirror in if PM net exists
    nv_net = romuluslog::RomulusLog::get_object<NVModel>(0);
    if (nv_net != nullptr)
    {
        //mirror in and resume training
        nv_net->mirror_in(net, &avg_loss);
    }

    int epoch = (*net->seen) / N;
    count = 0;
    num_params = get_param_size(net);
    comm_in->model_size = (double)(num_params * 4) / (1024 * 1024);

    PLINIUS_INFO("Max batches: %d\n", net->max_batches);
    PLINIUS_INFO("Net batch size: %d\n", net->batch);
    PLINIUS_INFO("Number of params: %d  Model size: %f MB \n", num_params, comm_in->model_size);

    //set batch size
    batch_size = net->batch;
    //allocate training data
    train = data_alloc(batch_size);
    //load data from disk to PM
    load_pm_data();
    //you can reduce the number of iters to a smaller num just for testing purposes
    //net->max_batches = 10;

    //allocate nvmodel here
    if (nv_net == nullptr) //mirror model absent
    {
        nv_net = (NVModel *)TM_PMALLOC(sizeof(struct NVModel));
        romuluslog::RomulusLog::put_object<NVModel>(0, nv_net);
        nv_net->allocator(net);
        avg_loss = -1; //we are training from 0
    }

    //training iterations
    while ((cur_batch < net->max_batches || net->max_batches == 0))
    {
        count++;
        cur_batch = get_current_batch(net);

        /* Get and decrypt batch of pm data */
        get_pm_batch();

        //one training iteration
        loss = train_network_sgd(net, train, 1);

        if (avg_loss < 0)
        {
            avg_loss = loss;
        }

        avg_loss = avg_loss * .9 + loss * .1;
        epoch = (*net->seen) / N;

        progress = ((double)cur_batch / net->max_batches) * 100;
        if (cur_batch % LOG_FREQ == 0)
        { //print benchmark progress every LOG_FREQ iters
            PLINIUS_INFO("Batch num: %ld, Avg loss: %f avg, L. rate: %f, Progress: %.2f%% \n",
                         cur_batch, avg_loss, get_current_rate(net), progress);
        }

        //mirror model out to PM
        nv_net->mirror_out(net, &avg_loss);
    }

    PLINIUS_INFO("Done training mnist network..\n");
    free_network(net);
}

void ecall_tester(list *sections, data *test_data, int pmem)
{
    CHECK_REF_POINTER(sections, sizeof(list));
    CHECK_REF_POINTER(test_data, sizeof(data));
    /**
     * load fence after pointer checks ensures the checks are done 
     * before any assignment 
     */
    sgx_lfence();
    test_mnist(sections, test_data, pmem);
}

void ecall_classify(list *sections, list *labels, image *im)
{
    CHECK_REF_POINTER(sections, sizeof(list));
    CHECK_REF_POINTER(labels, sizeof(list));
    CHECK_REF_POINTER(im, sizeof(image));
    /**
     * load fence after pointer checks ensures the checks are done 
     * before any assignment 
     */
    sgx_lfence();
    //classify_tiny(sections, labels, im, 5);
}

/**
 * Test trained mnist model
 */
void test_mnist(list *sections, data *test_data, int pmem)
{

    if (pmem)
    {
        //dummy variable
    }

    srand(12345);
    float avg_loss = 0;
    network *net = create_net_in(sections);

    //instantiate nvmodel
    nv_net = romuluslog::RomulusLog::get_object<NVModel>(0);
    if (nv_net != nullptr)
    {
        nv_net->mirror_in(net, &avg_loss);
        PLINIUS_INFO("Mirrored net in for testing\n");
    }

    if (net == NULL)
    {
        PLINIUS_INFO("No neural network in enclave..\n");
        return;
    }
    srand(12345);

    PLINIUS_INFO("-----Beginning mnist testing----\n");
    float avg_acc = 0;
    data test = *test_data;
    float *acc = network_accuracies(net, test, 2);
    avg_acc += acc[0];

    printf("Accuracy: %f%%, %d images\n", avg_acc * 100, test.X.rows);
    free_network(net);

    /**
     * Test mnist multi
     *
    float avg_acc = 0;
    data test = *test_data;
    image im;

    for (int i = 0; i < test.X.rows; ++i)
    {
         im = float_to_image(28, 28, 1, test.X.vals[i]);

        float pred[10] = {0};

        float *p = network_predict(net, im.data);
        axpy_cpu(10, 1, p, 1, pred, 1);
        flip_image(im);
        p = network_predict(net, im.data);
        axpy_cpu(10, 1, p, 1, pred, 1);

        int index = max_index(pred, 10);
        int class = max_index(test.y.vals[i], 10);
        if (index == class)
            avg_acc += 1;
        
       printf("%4d: %.2f%%\n", i, 100. * avg_acc / (i + 1)); //un/comment to see/hide accuracy progress
    }
    printf("Overall prediction accuracy: %2f%%\n", 100. * avg_acc / test.X.rows);
    free_network(net);    
    */
}
