#include "c++test.h"
#include <iostream>

class a_class {
public:
  double a_value;
  a_class(double vvv) { a_value = vvv; };
  void a_method(double vvv) { a_value += vvv; };
};

void *new_object(double initial_value)
{
  return static_cast<void*>(new a_class(initial_value));
}

double call_method(void *the_object, double param)
{
  a_class *pc = static_cast<a_class*>(the_object);
  pc->a_method(param);
  std::cout << "Current a_value = " << pc->a_value << std::endl;
  return pc->a_value;
}
void delete_object(void *the_object)
{
  a_class *pc = static_cast<a_class*>(the_object);
  delete pc;
}