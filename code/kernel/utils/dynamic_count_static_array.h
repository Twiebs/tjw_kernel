#pragma once

#define DEFINE_DYNAMIC_COUNT_STATIC_ARRAY(NAME, Name, Element, Count) \
  typedef struct {                                                    \
    Element elements[Count];                                          \
    size_t count;                                                     \
  } NAME;                                                             \
                                                                      \
  Element *Name##_add(NAME *array) {                                  \
    if (array->count >= Count) return NULL;                           \
    Element *result = &array->elements[array->count++];               \
    return result;                                                    \
  }

//END DEFINE_DYNAMIC_COUNT_STATIC_ARRAY