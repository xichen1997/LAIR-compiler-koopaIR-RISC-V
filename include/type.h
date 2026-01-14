struct Type {
    enum Kind { Int, Pointer, Array } kind;
    Type* elem = nullptr; // Pointer / Array 
    int   len  = -1;      // Array 

    static Type* IntTy() {
        static Type t{Int, nullptr, -1};
        return &t;
    }

    static Type* PointerTy(Type* elem) {
        return new Type{Pointer, elem, -1};
    }

    static Type* ArrayTy(Type* elem, int len) {
        return new Type{Array, elem, len};
    }
};