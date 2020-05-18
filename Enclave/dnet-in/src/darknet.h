/**
 * Author: xxx xxxx
 */

/*
 * Created on Fri Feb 14 2020
 *
 * Copyright (c) 2020 xxx xxxx, xxxx
 */

#ifndef DARKNET_API_IN
#define DARKNET_API_IN

#include "dnet_sgx_utils.h"
#include "dnet_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

    extern int gpu_index;

    metadata get_metadata(char *file);
    tree *read_tree(char *filename);
    void free_layer(layer);
    //calculate num of params in network
    unsigned int get_param_size(network *net);
    //create enclave net with parsed sections
    network *create_net_in(list *sections);

    //modify this api
    void load_network(network *net, char *weights, int clear);

    load_args get_base_args(network *net);

    void free_data(data d);
    //pthread_t load_data(load_args args);
    //list *read_data_cfg(char *filename);
    //list *read_cfg(char *filename);
    //unsigned char *read_file(char *filename);
    data resize_data(data orig, int w, int h);
    data *tile_data(data orig, int divs, int size);
    data select_data(data *orig, int *inds);

    void forward_network(network *net);
    void backward_network(network *net);
    void update_network(network *net);

    float dot_cpu(int N, float *X, int INCX, float *Y, int INCY);
    void axpy_cpu(int N, float ALPHA, float *X, int INCX, float *Y, int INCY);
    void copy_cpu(int N, float *X, int INCX, float *Y, int INCY);
    void scal_cpu(int N, float ALPHA, float *X, int INCX);
    void fill_cpu(int N, float ALPHA, float *X, int INCX);
    void normalize_cpu(float *x, float *mean, float *variance, int batch, int filters, int spatial);
    void softmax(float *input, int n, float temp, int stride, float *output);

    int best_3d_shift_r(image a, image b, int min, int max);

    image get_label(image **characters, char *string, int size);
    void draw_label(image a, int r, int c, image label, const float *rgb);
    //void save_image(image im, const char *name);
    //void save_image_options(image im, const char *name, IMTYPE f, int quality);
    void get_next_batch(data d, int n, int offset, float *X, float *y);
    void grayscale_image_3c(image im);
    void normalize_image(image p);
    //void matrix_to_csv(matrix m);
    float train_network_sgd(network *net, data d, int n);
    void rgbgr_image(image im);
    data copy_data(data d);
    data concat_data(data d1, data d2);
    //data load_cifar10_data(char *filename);
    void scale_data_rows(data d, float s);
    float matrix_topk_accuracy(matrix truth, matrix guess, int k);
    void matrix_add_matrix(matrix from, matrix to);
    void scale_matrix(matrix m, float scale);
    void transpose_matrix(float *a, int rows, int cols);
    //matrix csv_to_matrix(char *filename);
    float *network_accuracies(network *net, data d, int n);
    float train_network_datum(network *net);
    image make_random_image(int w, int h, int c);

    void denormalize_connected_layer(layer l);
    void denormalize_convolutional_layer(layer l);
    void statistics_connected_layer(layer l);
    void rescale_weights(layer l, float scale, float trans);
    void rgbgr_weights(layer l);
    image *get_weights(layer l);

    //void demo(char *cfgfile, char *weightfile, float thresh, int cam_index, const char *filename, char **names, int classes, int frame_skip, char *prefix, int avg, float hier_thresh, int w, int h, int fps, int fullscreen);
    void get_detection_detections(layer l, int w, int h, float thresh, detection *dets);

    char *option_find_str(list *l, char *key, char *def);
    int option_find_int(list *l, char *key, int def);
    int option_find_int_quiet(list *l, char *key, int def);

    //network *parse_network_cfg(char *filename);
    void save_weights(network *net, char *filename);
    void load_weights(network *net, char *filename);
    void save_weights_upto(network *net, char *filename, int cutoff);
    void load_weights_upto(network *net, char *filename, int start, int cutoff);

    void zero_objectness(layer l);
    void get_region_detections(layer l, int w, int h, int netw, int neth, float thresh, int *map, float tree_thresh, int relative, detection *dets);
    int get_yolo_detections(layer l, int w, int h, int netw, int neth, float thresh, int *map, int relative, detection *dets);
    void free_network(network *net);
    void set_batch_network(network *net, int b);
    void set_temp_network(network *net, float t);
    //image load_image(char *filename, int w, int h, int c);
    //image load_image_color(char *filename, int w, int h);
    image make_image(int w, int h, int c);
    image resize_image(image im, int w, int h);
    void censor_image(image im, int dx, int dy, int w, int h);
    image letterbox_image(image im, int w, int h);
    image crop_image(image im, int dx, int dy, int w, int h);
    image center_crop_image(image im, int w, int h);
    image resize_min(image im, int min);
    image resize_max(image im, int max);
    image threshold_image(image im, float thresh);
    image mask_to_rgb(image mask);
    int resize_network(network *net, int w, int h);
    void free_matrix(matrix m);
    void test_resize(char *filename);
    int show_image(image p, const char *name, int ms);
    image copy_image(image p);
    void draw_box_width(image a, int x1, int y1, int x2, int y2, int w, float r, float g, float b);
    float get_current_rate(network *net);
    void composite_3d(char *f1, char *f2, char *out, int delta);
    //data load_data_old(char **paths, int n, int m, char **labels, int k, int w, int h);
    size_t get_current_batch(network *net);
    void constrain_image(image im);
    image get_network_image_layer(network *net, int i);
    layer get_network_output_layer(network *net);
    void top_predictions(network *net, int n, int *index);
    void flip_image(image a);
    image float_to_image(int w, int h, int c, float *data);
    void ghost_image(image source, image dest, int dx, int dy);
    float network_accuracy(network *net, data d);
    void random_distort_image(image im, float hue, float saturation, float exposure);
    void fill_image(image m, float s);
    image grayscale_image(image im);
    void rotate_image_cw(image im, int times);
    //double what_time_is_it_now();
    image rotate_image(image m, float rad);
    void visualize_network(network *net);
    float box_iou(box a, box b);
    data load_all_cifar10();
    box_label *read_boxes(char *filename, int *n);
    box float_to_box(float *f, int stride);
    void draw_detections(image im, detection *dets, int num, float thresh, char **names, image **alphabet, int classes);

    matrix network_predict_data(network *net, data test);
    image **load_alphabet();
    image get_network_image(network *net);
    float *network_predict(network *net, float *input);

    int network_width(network *net);
    int network_height(network *net);
    float *network_predict_image(network *net, image im);
    void network_detect(network *net, image im, float thresh, float hier_thresh, float nms, detection *dets);
    detection *get_network_boxes(network *net, int w, int h, float thresh, float hier, int *map, int relative, int *num);
    void free_detections(detection *dets, int n);

    void reset_network_state(network *net, int b);

    //char **get_labels(char *filename);
    void do_nms_obj(detection *dets, int total, int classes, float thresh);
    void do_nms_sort(detection *dets, int total, int classes, float thresh);

    matrix make_matrix(int rows, int cols);

    void free_image(image m);
    float train_network(network *net, data d);
    //pthread_t load_data_in_thread(load_args args);
    //void load_data_blocking(load_args args);
    //list *get_paths(char *filename);
    void hierarchy_predictions(float *predictions, int n, tree *hier, int only_leaves, int stride);
    void change_leaves(tree *t, char *leaf_list);

    int find_int_arg(int argc, char **argv, char *arg, int def);
    float find_float_arg(int argc, char **argv, char *arg, float def);
    int find_arg(int argc, char *argv[], char *arg);
    char *find_char_arg(int argc, char **argv, char *arg, char *def);
    //char *basecfg(char *cfgfile);
    void find_replace(char *str, char *orig, char *rep, char *output);
    void free_ptrs(void **ptrs, int n);
    //char *fgetl(FILE *fp);
    void strip(char *s);
    //float sec(clock_t clocks);
    void **list_to_array(list *l);
    void top_k(float *a, int n, int k, int *index);
    int *read_map(char *filename);
    void error(const char *s);
    int max_index(float *a, int n);
    int max_int_index(int *a, int n);
    int sample_array(float *a, int n);
    int *random_index_order(int min, int max);
    void free_list(list *l);
    float mse_array(float *a, int n);
    float variance_array(float *a, int n);
    float mag_array(float *a, int n);
    void scale_array(float *a, int n, float s);
    float mean_array(float *a, int n);
    float sum_array(float *a, int n);
    void normalize_array(float *a, int n);
    int *read_intlist(char *s, int *n, int d);
    size_t rand_size_t();
    float rand_normal();
    float rand_uniform(float min, float max);

#ifdef __cplusplus
}
#endif
#endif
