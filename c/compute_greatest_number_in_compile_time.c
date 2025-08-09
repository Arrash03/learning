union value_set {
#define V(name, val) char name##_[val];
    V(A, 56)
    V(B, 76)
    V(C, 14)
#undef V
};

enum { GREATEST = sizeof(union value_set) };

