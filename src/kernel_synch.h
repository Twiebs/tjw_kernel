

typedef struct {
  volatile int lock;
} Spin_Lock;

static inline void spinlock_acquire(Spin_Lock *lock){
  while(!__sync_bool_compare_and_swap(&lock->lock, 0, 1));
  __sync_synchronize();
}

static inline void spinlock_release(Spin_Lock *lock){
  __sync_synchronize(); 
  lock->lock = 0;
}