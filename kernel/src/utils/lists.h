#pragma once
#include <panic.h>
#include <stddef.h>

typedef struct list_head {
	struct list_head *prev;
	struct list_head *next;
} list_head_t;

static inline void list_head_init(list_head_t *head){
  assert(head == NULL, "Variable head is NULL.");
	head->prev = head->next = head;
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
