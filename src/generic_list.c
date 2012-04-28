#include <stdlib.h>
#include <assert.h>
#include "common.h"

lnode_t* list_node_new(void)
{
        lnode_t         *ptr;

        ptr = malloc(sizeof (lnode_t));
        assert (ptr != NULL);

        ptr->data = NULL;
        ptr->next = NULL;

        return ptr;
}


lnode_t* list_link(lnode_t *head, lnode_t *node)
{
        lnode_t         *ptr;

        if (head == NULL)
                return node;

        for (ptr = head; ptr->next != NULL; ptr = ptr->next) ;

        ptr->next = node;

        return head;
}


lnode_t* list_unlink(lnode_t *head, lnode_t *node)
{
        lnode_t         *ptr, *ret = head, *prev = NULL;

        for (ptr = head; ptr != NULL; ptr = ptr->next) {
                if (ptr == node) {
                        if (prev == NULL && ptr->next != NULL)
                                ret = ptr->next;
                        else if (prev == NULL && ptr->next == NULL)
                                ret = NULL;
                        else if (prev != NULL && ptr->next != NULL)
                                prev->next = ptr->next;
                        else if (prev != NULL && ptr->next == NULL)
                                prev->next = NULL;

                        //free(ptr);
                        break;
                }
                prev = ptr;
        }

        return ret;
}

lnode_t* list_unlink_by_number(lnode_t *head, size_t number)
{
        lnode_t         *ptr;
        size_t          num = 0;

        for (ptr = head; ptr != NULL; ptr = ptr->next) {
                if (num++ == number) {
                        return list_unlink(head, ptr);
                }
        }

        return head;
}


lnode_t* list_node_to_front(lnode_t *head, lnode_t *node)
{
        lnode_t         *ptr, *phead, *ret;


        ptr = node;
        phead = list_unlink(head, node);

        ptr->next = NULL;

        ret = list_link(ptr, phead);

        return ret;
}

lnode_t* list_node_to_back(lnode_t *head, lnode_t *node)
{
        lnode_t         *ptr, *phead, *ret;


        ptr = node;
        phead = list_unlink(head, node);

        ptr->next = NULL;

        ret = list_link(phead, ptr);

        return ret;
}

lnode_t* list_get_node_by_number(lnode_t *head, size_t number)
{
        lnode_t         *ptr;
        size_t          num = 0;

        for (ptr = head; ptr != NULL; ptr = ptr->next) {
                if (num++ == number) {
                        return ptr;
                }
        }

        return NULL;
}

lnode_t* list_destroy(lnode_t *head)
{
        lnode_t         *ptr, *prev = NULL;

        ptr = head;
        for (;;) {
                if (ptr == NULL)
                        break;
                prev = ptr;
                ptr = ptr->next;
                if (prev->data) {
                        free(prev->data);
                }
                free(prev);
        }

        return NULL;
}
