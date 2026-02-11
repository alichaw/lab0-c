#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *q = malloc(sizeof(struct list_head));

    if (!q) {
        return NULL;
    }

    q->next = q;
    q->prev = q;

    return q;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head) {
        return;
    }

    element_t *entry, *safe;

    list_for_each_entry_safe(entry, safe, head, list) {
        list_del(&entry->list);
        q_release_element(entry);
    }
    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head || !s) {
        return false;
    }
    element_t *new = malloc(sizeof(element_t));
    if (!new) {
        return false;
    }

    new->value = strdup(s);
    if (!new->value) {
        free(new);
        return false;
    }

    list_add(&new->list, head);

    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head || !s) {
        return false;
    }
    element_t *new = malloc(sizeof(element_t));
    if (!new) {
        return false;
    }
    new->value = strdup(s);
    if (!new->value) {
        free(new);
        return false;
    }
    list_add_tail(&new->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head)) {
        return NULL;
    }
    element_t *entry = list_first_entry(head, element_t, list);
    list_del(&entry->list);
    if (sp && bufsize > 0 && entry->value) {
        size_t len = strlen(entry->value);
        if (len >= bufsize)
            len = bufsize - 1;
        memcpy(sp, entry->value, len);
        sp[len] = '\0';
    }
    return entry;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head)) {
        return NULL;
    }
    element_t *entry = list_last_entry(head, element_t, list);
    list_del(&entry->list);
    if (sp && bufsize > 0 && entry->value) {
        size_t len = strlen(entry->value);
        if (len >= bufsize)
            len = bufsize - 1;
        memcpy(sp, entry->value, len);
        sp[len] = '\0';
    }
    return entry;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head) {
        return 0;
    }
    int size = 0;
    struct list_head *curr;
    list_for_each(curr, head) {
        size++;
    }
    return size;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head)) {
        return false;
    }
    struct list_head *slow = head->next;
    struct list_head *fast = head->next;
    while (fast != head && fast->next != head) {
        slow = slow->next;
        fast = fast->next->next;
    }
    list_del(slow);
    element_t *entry = list_entry(slow, element_t, list);
    q_release_element(entry);

    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head || list_empty(head))
        return false;

    struct list_head *curr = head->next;

    while (curr != head) {
        element_t *e = list_entry(curr, element_t, list);
        struct list_head *next = curr->next;
        bool dup = false;

        // 刪掉 curr 後面連續相同的（因為題目說已排序）
        while (next != head) {
            element_t *en = list_entry(next, element_t, list);
            if (strcmp(e->value, en->value) != 0)
                break;

            dup = true;
            struct list_head *tmp = next->next;
            list_del(next);
            q_release_element(en);
            next = tmp;
        }

        if (dup) {
            // 連 curr 自己也要刪
            struct list_head *tmp = curr->next; // 這時 curr->next == next
            list_del(curr);
            q_release_element(e);
            curr = tmp;
        } else {
            curr = curr->next;
        }
    }

    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_empty(head)) {
        return;
    }
    struct list_head *curr = head->next;
    while (curr != head && curr->next != head) {
        struct list_head *first = curr;
        struct list_head *second = curr->next;
        list_del(first);
        list_add(first, second);
        curr = first->next;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head)) {
        return;
    }
    struct list_head *curr = head;
    do {
        struct list_head *temp = curr->next;
        curr->next = curr->prev;
        curr->prev = temp;
        curr = temp;
    } while (curr != head);
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || list_empty(head) || k < 2)
        return;

    LIST_HEAD(result);

    while (!list_empty(head)) {
        struct list_head *end = head;
        int i;
        for (i = 0; i < k && end->next != head; i++)
            end = end->next;

        if (i < k) { // 不足 k 個
            list_splice_tail_init(head, &result);
            break;
        }

        LIST_HEAD(group);
        list_cut_position(&group, head, end);
        q_reverse(&group);
        list_splice_tail_init(&group, &result);
    }

    INIT_LIST_HEAD(head);
    list_splice_init(&result, head);
}

struct list_head *merge(struct list_head *l1,
                        struct list_head *l2,
                        bool descend)
{
    // https://leetcode.com/problems/merge-two-sorted-lists/
    struct list_head *head = NULL, **ptr = &head;
    while (l1 && l2) {
        element_t *e1 = list_entry(l1, element_t, list);
        element_t *e2 = list_entry(l2, element_t, list);
        int cmp = strcmp(e1->value, e2->value);
        bool choose_l1 = descend ? (cmp > 0) : (cmp < 0);
        if (cmp == 0)
            choose_l1 = true;
        if (choose_l1) {
            *ptr = l1;
            l1 = l1->next;
        } else {
            *ptr = l2;
            l2 = l2->next;
        }
        ptr = &(*ptr)->next;
    }
    *ptr = l1 ? l1 : l2;
    return head;
}

struct list_head *merge_sort(struct list_head *head, bool descend)
{
    if (!head || !head->next) {
        return head;
    }
    struct list_head *slow = head, *fast = head->next;
    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
    }
    struct list_head *mid = slow->next;
    slow->next = NULL;

    struct list_head *left = merge_sort(head, descend);
    struct list_head *right = merge_sort(mid, descend);
    return merge(left, right, descend);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head)) {
        return;
    }
    head->prev->next = NULL;
    head->next->prev = NULL;
    head->next = merge_sort(head->next, descend);
    struct list_head *curr = head, *next = head->next;
    while (next) {
        next->prev = curr;
        curr = next;
        next = next->next;
    }
    curr->next = head;
    head->prev = curr;
}

/* Helper for q_ascend/q_descend */
int q_remove_nodes(struct list_head *head, bool descend)
{
    if (!head || list_empty(head)) {
        return 0;
    }
    q_reverse(head);
    struct list_head *curr = head->next;
    struct list_head *next;

    element_t *max = list_entry(curr, element_t, list);
    curr = curr->next;

    while (curr != head) {
        next = curr->next;
        element_t *curr_elem = list_entry(curr, element_t, list);
        int cmp = strcmp(curr_elem->value, max->value);

        bool drop = descend ? (cmp < 0) : (cmp > 0);
        if (drop) {
            list_del(curr);
            q_release_element(curr_elem);
        } else {
            max = curr_elem;
        }
        curr = next;
    }

    q_reverse(head);
    // 修正 3: 必須回傳新的大小
    return q_size(head);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return q_remove_nodes(head, false);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return q_remove_nodes(head, true);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    if (!head || list_empty(head)) return 0;
    if (list_is_singular(head))
        return list_entry(head->next, queue_contex_t, chain)->size;

    queue_contex_t *first = list_entry(head->next, queue_contex_t, chain);
    struct list_head *merged_head = first->q;

    struct list_head *curr = head->next->next;
    while (curr != head) {
        queue_contex_t *ctx = list_entry(curr, queue_contex_t, chain);
        
        // 準備合併：斷開兩個環
        merged_head->prev->next = NULL;
        ctx->q->prev->next = NULL;

        // 進行合併
        merged_head->next = merge(merged_head->next, ctx->q->next, descend);

        // 重新連接 prev 指標並封閉環狀鏈結
        struct list_head *tmp = merged_head, *n = merged_head->next;
        while (n) {
            n->prev = tmp;
            tmp = n;
            n = n->next;
        }
        tmp->next = merged_head;
        merged_head->prev = tmp;

        // 清空被合併的佇列
        INIT_LIST_HEAD(ctx->q);
        ctx->size = 0;
        curr = curr->next;
    }
    first->size = q_size(merged_head);
    return first->size;
}