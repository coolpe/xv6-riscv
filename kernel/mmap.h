struct vma {
    uint64 address;
    uint64 length;
    int prot;
    int ref; // reference
    struct file *f;
};

