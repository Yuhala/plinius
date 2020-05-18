
#include "dnet_sgx_utils.h"
#include "option_list.h"
#include "utils.h"

/* metadata get_metadata(char *file)
{
    metadata m = {0};
    list *options = read_data_cfg(file);

    char *name_list = option_find_str(options, "names", 0);
    if(!name_list) name_list = option_find_str(options, "labels", 0);
    if(!name_list) {
        fprintf(stderr, "No names or labels found\n");
    } else {
        m.names = get_labels(name_list);
    }
    m.classes = option_find_int(options, "classes", 2);
    free_list(options);
    return m;
} */

int read_option(char *s, list *options)
{
    size_t i;
    size_t len = strlen(s);
    char *val = 0;
    for (i = 0; i < len; ++i)
    {
        if (s[i] == '=')
        {
            s[i] = '\0';
            val = s + i + 1;
            break;
        }
    }
    if (i == len - 1)
        return 0;
    char *key = s;
    option_insert(options, key, val);
    return 1;
}

void option_insert(list *l, char *key, char *val)
{
    kvp *p = malloc(sizeof(kvp));
    p->key = key;
    p->val = val;
    p->used = 0;
    list_insert(l, p);
}

void option_unused(list *l)
{
    node *n = l->front;
    while (n)
    {
        kvp *p = (kvp *)n->val;
        if (!p->used)
        {
#ifdef DNET_SGX_DEBUG
            printf("Unused field: '%s = %s'\n", p->key, p->val);
#endif
        }
        n = n->next;
    }
}

char *option_find(list *l, char *key)
{
    node *n = l->front;
    while (n)
    {
        kvp *p = (kvp *)n->val;
        if (strcmp(p->key, key) == 0)
        {
            p->used = 1;
            return p->val;
        }
        n = n->next;
    }
    return 0;
}
char *option_find_str(list *l, char *key, char *def)
{
    char *v = option_find(l, key);
    if (v)
        return v;
    if (def)
    {
#ifdef DNET_SGX_DEBUG
        printf("%s: Using default '%s'\n", key, def);
#endif
    }

    return def;
}

int option_find_int(list *l, char *key, int def)
{
    char *v = option_find(l, key);
    if (v)
        return atoi(v);
#ifdef DNET_SGX_DEBUG
    printf("%s: Using default '%d'\n", key, def);
#endif
    return def;
}

int option_find_int_quiet(list *l, char *key, int def)
{
    char *v = option_find(l, key);
    if (v)
        return atoi(v);
    return def;
}

float option_find_float_quiet(list *l, char *key, float def)
{
    char *v = option_find(l, key);
    if (v)
        return atof(v);
    return def;
}

float option_find_float(list *l, char *key, float def)
{
    char *v = option_find(l, key);
    if (v)
        return atof(v);
#ifdef DNET_SGX_DEBUG
    printf("%s: Using default '%lf'\n", key, def);
#endif
    return def;
}
