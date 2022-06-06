#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "stat.h"
#include "spinlock.h"
#include "proc.h"
#include "fs.h"
#include "sleeplock.h"
#include "file.h"
#include "fcntl.h"
#include "mmap.h"

struct vma vma[NVMA];
struct {
    struct spinlock lock;
    struct vma vma[NVMA];
} vtable;

void
vmainit(void) {
  initlock(&vtable.lock, "vtable");
}

static struct vma *
vmaalloc(void) {
  struct vma *v;

  acquire(&vtable.lock);
  for (v = vtable.vma; v < vtable.vma + NVMA; v++) {
    if (v->ref == 0) {
      v->ref = 1;
      release(&vtable.lock);
      return v;
    }
  }
  release(&vtable.lock);
  return 0;
}

static uint64
walk(pagetable_t pagetable, uint64 length) {
  uint64 va = 0, size = 0;

  for (uint64 i = 0; i < MAXVA; i += PGSIZE) {
    if (walkaddr(pagetable, i) !=0) {
      va = i + PGSIZE;
      size = 0;
      continue;
    }
    size += PGSIZE;
    if (size == length)
      break;
  }

  if (size != length || size == 0)
    panic("no more unused region");
  printf("mmap va %p\n", va);
  mappages(pagetable, va, length, 0, 0);
  return va;
}

uint64 mmap(void *addr, uint64 length, int prot, int flags,
            struct file *f, uint64 offset) {
  struct vma *v;
  uint64 va;
  int vd;
  struct proc *p = myproc();
  if ((v = vmaalloc()) == 0)
    panic("no vma");

  va = walk(p->pagetable, length);
  v->length = length;
  v->prot = prot;
  f->ref++;
  v->f = f;
  v->address = va;


  for (vd = 0; vd < NVMA; vd++) {
    if (p->mvma[vd] == 0) {
      p->mvma[vd] = v;
      return va;
    }
  }
  return 0;
}

int munmap(void *addr, uint64 length) {
  return 0;
}

