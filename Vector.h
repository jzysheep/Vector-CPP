//
// Created by Ziyang Jiang on 1/31/16.
//
// Vector.h -- header file for Vector data structure project

#pragma once
#ifndef _Vector_h
#define _Vector_h

#define MIN_CAPACITY 8


#include <new>
#include <cstdint>
#include <stdexcept>
#include <assert.h>
#include <iostream>

namespace epl{
    
    template <typename T>
    class Vector {
    private:
        int64_t start;
        int64_t finish;
        uint64_t capacity;
        T* buffer;
        
        void copy(const Vector<T>&);
        void destroy(void);
    public:
        Vector(void);
        explicit Vector(uint64_t);
        Vector(const Vector<T>&);
        uint64_t size(void) const;
        //        Vector(Vector<T>&&) noexcept;
        Vector<T>& operator=(const Vector<T>&);
        //        Vector<T>& operator=(Vector<T>&&) noexcept;
        ~Vector(void);
        T& operator[](uint64_t);
        const T& operator[](uint64_t) const;
        void push_back(const T&);
        //        void push_back(T&&) noexcept;
        void push_front(const T&);
        //        void push_front(T&&) noexcept ;
        void pop_back(void);
        void pop_front(void);
    };
    
    template <typename T>
    Vector<T>::Vector(void) {
        capacity = MIN_CAPACITY;
        start = -1;
        finish = -1;
        buffer = (T*) operator new(sizeof(T) * capacity);
    }
    
    template <typename T>
    Vector<T>::Vector(uint64_t n) {
        assert(n >= 0);
        if (n == 0) {
            start = -1;
            finish = -1;
            capacity = MIN_CAPACITY;
            buffer = (T*) operator new(MIN_CAPACITY * sizeof(T));
            
        } else {
            start = 0;
            finish =  n - 1;
            capacity = n;
            buffer = (T*) operator new(n * sizeof(T));
            for (int i = 0; i < n; i++) {
                new (buffer + i) T{};
            }
        }
    }
    
    template <typename T>
    void Vector<T>::copy(const Vector<T>& that) {
        start = that.start;
        finish = that.finish;
        capacity = that.capacity;
        
        if (capacity != 0) {
            buffer = (T*) operator new(capacity * sizeof(T));
            for (uint64_t i = 0; i < size(); i++) {
                new (buffer + (start + i) % capacity) T{that[i]};
            }
        } else {
            buffer = nullptr;
        }
    }
    
    template <typename T>
    void Vector<T>::destroy(void) {
        for (int i = 0; i < size(); i++) {
            (operator [](i)).~T();
        }
        
        operator delete(buffer);
    }
    
    template <typename T>
    Vector<T>::Vector(const Vector<T> &that) {
        copy(that);
    }
    
    template <typename T>
    uint64_t Vector<T>::size(void) const {
        if (start == -1) {
            return 0;
        }
        
        if (finish >= start) {
            return finish - start + 1;
        } else {
            return finish + 1 + capacity - start;
        }
    }
    
    //    template <typename T>
    //    Vector<T>::Vector(Vector<T>&& tmp) noexcept {
    //        start = tmp.start;
    //        finish = tmp.finish;
    //        capacity = tmp.capacity;
    //        buffer = tmp.buffer;
    //        tmp.buffer = nullptr;
    //        tmp.start = -1;
    //        tmp.finish = -1;
    //        tmp.capacity = 0;
    //    }
    
    template <typename T>
    Vector<T>& Vector<T>::operator=(const Vector<T> &rhs) {
        if (this != &rhs) {
            for (int i = 0; i < size(); i++) {
                (operator [](i)).~T();
            }
            
            if (rhs.capacity > capacity) {
                operator delete(buffer);
                if (rhs.capacity != 0) {
                    buffer = (T*) operator new(rhs.capacity * sizeof(T));
                } else {
                    buffer = nullptr;
                }
                capacity = rhs.capacity;
            }
            
            uint64_t rhs_size = rhs.size();
            if (rhs_size != 0) {
                start = 0;
                finish = rhs_size - 1;
                for(int i = 0; i < rhs_size; i++) {
                    new (buffer + i) T{rhs[i]};
                }
            } else {
                start = -1;
                finish = -1;
            }
            
        }
        return *this;
        
    }
    //
    //    template <typename T>
    //    Vector<T>& Vector<T>::operator=(Vector<T>&& tmp) noexcept {
    //        std::swap(start, tmp.start);
    //        std::swap(finish, tmp.finish);
    //        std::swap(capacity, tmp.capacity);
    //        std::swap(buffer, tmp.buffer);  // type mismatch?
    //        return *this;
    //    }
    
    template <typename T>
    Vector<T>::~Vector(void) {
        destroy();
    }
    
    template <typename T>
    T& Vector<T>::operator[](uint64_t index) {
        if (index >= size()) {
            throw std::out_of_range("subscript out of range");
        }
        return buffer[(start + index) % capacity];
    }
    
    template <typename T>
    const T& Vector<T>::operator[](uint64_t index) const {
        if (index >= size()) {
            throw std::out_of_range("subscript out of range");
        }
        return buffer[(start + index) % capacity];
    }
    
    template <typename T>
    void Vector<T>::push_back(const T& element) {
        if (start != (finish + 1) % capacity) {
            finish = (finish + 1) % capacity;
            new (buffer + finish) T(element);
            
            if (start == -1) {
                start = 0;
            }
        } else {
            T* new_address = (T*) operator new(capacity * 2 * sizeof(T));
            new (new_address + capacity) T(element);
            for (int i=0; i < size(); ++i) {
                new (new_address + i) T{std::move(operator [](i))};
            }
            
            for (int i=0; i < size(); ++i) {
                (buffer + i) -> ~T();
            }
            operator delete(buffer);
            
            start = 0;
            finish = capacity;
            capacity *= 2;
            buffer = new_address;
        }
    }
    //
    //    template <typename T>
    //    void Vector<T>::push_back(T&& element) noexcept {
    //        if (start != (finish + 1) % capacity) {
    //            finish = (finish + 1) % capacity;
    //            new(buffer + finish) T(std::move(element)); // difference
    //
    //            if (start == -1) {
    //                start = 0;
    //            }
    //        } else {
    //            T* new_address = (T*) operator new(capacity * 2 * sizeof(T));
    //            new(new_address + capacity) T(std::move(element)); // difference
    //            for (int i = 0; i < size(); ++i) {
    //                new(new_address + i) T{std::move(operator[](i))};
    //            }
    //
    //            for (int i = 0; i < size(); ++i) {
    //                (buffer + i)->~T();
    //            }
    //            operator delete(buffer);
    //
    //            start = 0;
    //            finish = capacity;
    //            capacity *= 2;
    //            buffer = new_address;
    //        }
    //    }
    
    template <typename T>
    void Vector<T>::push_front(const T& element) {
        if (start != (finish + 1) % capacity) {
            if (start == -1) {
                start = 0;
                finish = 0;
            } else {
                if ( start == 0 ) {
                    start = capacity;
                }
                start -= 1;
            }
            new (buffer + start) T(element);
        } else {
            T* new_address = (T*) operator new(capacity * 2 * sizeof(T));
            new(new_address + 2 * capacity - 1) T(element); // order?
            
            for (int i = 0; i < size(); i++) {
                new(new_address + i) T{std::move(operator [](i))};
            }
            
            for (int i = 0; i < size(); i++) {
                (buffer + i) -> ~T();
            }
            operator delete(buffer);
            
            finish = capacity - 1;
            capacity *= 2;
            start = capacity - 1;
            buffer = new_address;
        }
    }
    //
    //    template <typename T>
    //    void Vector<T>::push_front(T&& element) noexcept {
    //        if (start != (finish + 1) % capacity){
    //            if (start == -1) {
    //                start = 0;
    //                finish = 0;
    //            } else {
    //                if ( start == 0 ) {
    //                    start = capacity;
    //                }
    //                start -= 1;
    //            }
    //            new(buffer + start) T(std::move(element)); // difference
    //        } else {
    //            T* new_address = (T*) operator new(capacity * 2 * sizeof(T));
    //            for (int i = 0; i < size(); i++) {
    //                new(new_address + i) T{std::move(operator [](i))};
    //            }
    //            new(new_address + 2 * capacity - 1) T(std::move(element)); // difference
    //            for (int i = 0; i < size(); i++) {
    //                (buffer + i) -> ~T();
    //            }
    //            operator delete(buffer);
    //            finish = capacity - 1;
    //            capacity *= 2;
    //            start = capacity - 1;
    //            buffer = new_address;
    //        }
    //    }
    
    template <typename T>
    void Vector<T>::pop_back(void) {
        if (start == -1) {
            throw std::out_of_range("Vector is empty. No element to pop");
        }
        
        (buffer + finish) -> ~T();
        if (finish == 0) {
            finish = capacity;
        }
        finish -= 1;
        
        if (start == (finish + 1) % capacity) {
            start = -1;
            finish = -1;
        }
    }
    
    template <typename T>
    void Vector<T>::pop_front(void) {
        if (start == -1) {
            throw std::out_of_range("Vector is empty. No element to pop");
        }
        
        (buffer + start) -> ~T();
        start = (start + 1) % capacity;
        if (start == (finish + 1) % capacity) {
            start = -1;
            finish = -1;
        }
        
    }
} //namespace epl

#endif /* _Vector_h */

