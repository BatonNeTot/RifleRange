//
// Created by KILLdon on 27.05.2019.
//

#ifndef RIFLERANGE_OBJECT_POOL_H
#define RIFLERANGE_OBJECT_POOL_H

#include <set>
#include <functional>

template <typename T, unsigned int poolSize = 128>
class ObjectPool {
public:
    ObjectPool() = default;
    ~ObjectPool();

    template<typename... Args>
    T *create(Args &&... args);
    bool free(T *object);

    void foreachUsed(std::function<void (T &)> action);

private:

    std::set<void *> pools;
    std::set<T *> freeObjects;
    std::set<T *> usedObjects;
};

template <typename T, unsigned int poolSize>
ObjectPool<T, poolSize>::~ObjectPool() {
    for (auto ptr : usedObjects) {
        ptr->~T();
    }
    for (auto ptr : pools) {
        ::operator delete(ptr);
    }
}

template <typename T, unsigned int poolSize>
template <typename... Args>
T *ObjectPool<T, poolSize>::create(Args &&... args) {
    auto iter = freeObjects.begin();

    if (iter == freeObjects.end()) {
        T *newPool = (T *)::operator new(poolSize * sizeof(T));

        pools.insert(newPool);
        T *end = newPool + poolSize;

        iter = freeObjects.insert(newPool++).first;
        auto iterNew = iter;
        while (newPool != end) {
            iterNew = freeObjects.insert(iterNew, newPool++);
        }
    }

    T *ptr = *iter;
    freeObjects.erase(iter);

    new (ptr) T(std::forward<Args>(args)...);

    usedObjects.insert(ptr);

    return ptr;
}

template <typename T, unsigned int poolSize>
bool ObjectPool<T, poolSize>::free(T *ptr) {
    auto iter = usedObjects.find(ptr);
    if (iter == usedObjects.end()) {
        return false;
    }

    usedObjects.erase(iter);

    ptr->~T();

    freeObjects.insert(ptr);


    return true;
}


template <typename T, unsigned int poolSize>
void ObjectPool<T, poolSize>::foreachUsed(std::function<void (T &)> action) {
    for (auto ptr : usedObjects) {
        action(*ptr);
    }
}

#endif //RIFLERANGE_OBJECT_POOL_H
