#include <stdio.h>
#include "c++test.h"

int main()
{
  void *the_object;
  the_object = new_object(0.0);
  call_method(the_object, 1.0);
  call_method(the_object, 1.0);
  delete_object(the_object);
  return 0;
}