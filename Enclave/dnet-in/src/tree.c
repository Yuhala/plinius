#include "dnet_sgx_utils.h"
#include "tree.h"
#include "utils.h"
#include "data.h"

/* void change_leaves(tree *t, char *leaf_list)
{
    list *llist = get_paths(leaf_list);
    char **leaves = (char **)list_to_array(llist);
    int n = llist->size;
    int i, j;
    int found = 0;
    for (i = 0; i < t->n; ++i)
    {
        t->leaf[i] = 0;
        for (j = 0; j < n; ++j)
        {
            if (0 == strcmp(t->name[i], leaves[j]))
            {
                t->leaf[i] = 1;
                ++found;
                break;
            }
        }
    }
#ifdef DNET_SGX_DEBUG
    printf("Found %d leaves.\n", found);
#endif
} */

float get_hierarchy_probability(float *x, tree *hier, int c, int stride)
{
    float p = 1;
    while (c >= 0)
    {
        p = p * x[c * stride];
        c = hier->parent[c];
    }
    return p;
}

void hierarchy_predictions(float *predictions, int n, tree *hier, int only_leaves, int stride)
{
    int j;
    for (j = 0; j < n; ++j)
    {
        int parent = hier->parent[j];
        if (parent >= 0)
        {
            predictions[j * stride] *= predictions[parent * stride];
        }
    }
    if (only_leaves)
    {
        for (j = 0; j < n; ++j)
        {
            if (!hier->leaf[j])
                predictions[j * stride] = 0;
        }
    }
}

int hierarchy_top_prediction(float *predictions, tree *hier, float thresh, int stride)
{
    float p = 1;
    int group = 0;
    int i;
    while (1)
    {
        float max = 0;
        int max_i = 0;

        for (i = 0; i < hier->group_size[group]; ++i)
        {
            int index = i + hier->group_offset[group];
            float val = predictions[(i + hier->group_offset[group]) * stride];
            if (val > max)
            {
                max_i = index;
                max = val;
            }
        }
        if (p * max > thresh)
        {
            p = p * max;
            group = hier->child[max_i];
            if (hier->child[max_i] < 0)
                return max_i;
        }
        else if (group == 0)
        {
            return max_i;
        }
        else
        {
            return hier->parent[hier->group_offset[group]];
        }
    }
    return 0;
}

