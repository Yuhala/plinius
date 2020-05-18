
#ifndef TRAINER_IN_H
#define TRAINER_IN_H

#define KILO 1024        //1 << 10
#define MEGA KILO * 1024 //1 << 20

void train_cifar(list *sections, data *training_data, int pmem);
void train_mnist(list *sections, data *training_data, int pmem);
void test_cfiar(list *sections, data *test_data, int pmem);
void test_mnist(list *sections, data *test_data, int pmem);
void classify_tiny(list *sections, list *labels, image *im, int top);

#if defined(__cplusplus)
extern "C"
{
#endif

#if defined(__cplusplus)
}
#endif

#endif