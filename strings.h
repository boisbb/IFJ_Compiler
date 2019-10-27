


typedef struct string String;
struct string{
  char *contents;           /// string
  int size;                 /// size of strings
  int asize;                /// actual size of string
};

int str_init(String *str);

int str_add(String *str, char new_char);
