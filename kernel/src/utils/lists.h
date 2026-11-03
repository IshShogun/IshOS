#pragma once
#include <panic.h>
#include <stddef.h>
#include <stdio.h>


#define offsetof(type, member) ((size_t)&(((type *)0)->member))

#define container_of(ptr, type, member) \
    ((type *)((char *)ptr - offsetof(type, member)))

#define list_entry(ptr, type, member) container_of(ptr, type, member)

typedef struct list_head {
	struct list_head *prev;
	struct list_head *next;
} list_head_t;

static inline void list_head_init(list_head_t *head){
  assert(head != NULL, "Variable head is not NULL.");
	// log_debug("I BREAK AFTER HERE");
	head->prev = head->next = head;
	// log_debug("I BREAK BEFORE HERE");
}

static inline void list_head_validate(const list_head_t *list)
{
    assert(list != NULL, "List is NULL.");
    assert(list->prev != NULL, "List->prev is NULL.");
    assert(list->next != NULL, "List->next is NULL.");
}


static inline void list_head_insert_after(list_head_t *entry, list_head_t *location){
	assert(entry != NULL, "Variable entry is NULL.");
	list_head_validate(location);

	list_head_t *old_next = location->next;
	location->next = entry;
	entry->prev = location;
	entry->next = old_next;
	old_next->prev = entry;
}

static inline int list_head_empty(const list_head_t *head)
{
    list_head_validate(head);
    return head->next == head;
}

static inline void list_head_remove(list_head_t *entry)
{
    if (!list_head_empty(entry)) {            // is it actually linked into a list?
        entry->prev->next = entry->next;      // left neighbour skips over entry
        entry->next->prev = entry->prev;      // right neighbour skips back over entry
        list_head_init(entry);                // entry->next = entry->prev = entry
    }
}

