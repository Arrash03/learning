#include <cassert>

class A {
  int i = 42;
};
template <int A::*PtrValue> struct private_access {
  friend int A::*get() { return PtrValue; }
};
int A::*get();
template struct private_access<&A::i>;
void usage() {
  A a;
  int A::*ip = get();
  int &i = a.*ip;
  assert(i == 42);
}

int main()
{
    ACCESS
    usage();
}
