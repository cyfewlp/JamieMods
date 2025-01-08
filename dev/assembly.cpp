
class A {
    int a1;

public:
    virtual int A_virt1();
    virtual int A_virt2();
    static void A_static1();
    void A_simple1();
};

class B {
    int b1;
    int b2;

public:
    virtual int B_virt1();
    virtual int B_virt2();
};

class C : public A, public B {
    int c1;

public:
    virtual int A_virt2();
    virtual int B_virt2();
};

int __fastcall foo(int a, int b) { return a + b; }

int main() { 
    auto* pC = new C();
    pC->A_virt2();
    pC->B_virt1();
    
    return foo(1, 2); }