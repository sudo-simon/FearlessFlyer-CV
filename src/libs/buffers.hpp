/*
 *  Copyright (C) 2023 Simone Palmieri <github dot com/sudo-simon>
 *  All rights reserved.
 *
 *  This file is part of a project released under the GNU GENERAL PUBLIC LICENSE
 * Version 3.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *  *  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  *  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#pragma once

#include <cstddef>
#include <iostream>
#include <vector>




/*
  FIFO buffer class
*/
template <typename T> class FIFOBuffer {

private:
  long size;
  long capacity;

  long push_i;
  long prev_push_i;

  long pop_i;
  std::vector<long> next_pop;

  std::vector<T> buffer;

public:
  FIFOBuffer();
  FIFOBuffer(long initial_capacity);
  ~FIFOBuffer();

  long getSize();
  long getCapacity();
  T at(long index);

  void push(T elem);
  void pop(T *ret_pointer);
};




/*
  LIFO buffer class
*/
template <typename T> class LIFOBuffer {

private:
  long size;
  long capacity;

  std::vector<T> buffer;

public:
  LIFOBuffer();
  LIFOBuffer(long initial_capacity);
  ~LIFOBuffer();

  long getSize();
  long getCapacity();

  void push(T elem);
  void pop(T *ret_pointer);
};





//? Start of FIFO buffer implementations -------------------------------------------------------------------------

/*
  Empty constructor that initializes capacity to 1
*/
template <typename T> FIFOBuffer<T>::FIFOBuffer() {
  this->size = 0;
  this->capacity = 1;
  this->push_i = -1;
  this->prev_push_i = -1;
  this->pop_i = -1;

  this->next_pop = std::vector<long>(1);

  this->buffer = std::vector<T>(1);
}

template <typename T> FIFOBuffer<T>::FIFOBuffer(long initial_capacity) {
  if (initial_capacity <= 0) {
    std::cerr << "FIFOBuffer ERROR: initial_capacity was not a positive long int" << std::endl;
    std::exit(1);
  }

  this->size = 0;
  this->capacity = initial_capacity;
  this->push_i = -1;
  this->prev_push_i = -1;
  this->pop_i = -1;

  this->next_pop = std::vector<long>(initial_capacity);

  this->buffer = std::vector<T>(initial_capacity);
}

template <typename T> FIFOBuffer<T>::~FIFOBuffer() {
  this->next_pop.clear();
  this->buffer.clear();
}




template <typename T> long FIFOBuffer<T>::getSize() { return this->size; }

template <typename T> long FIFOBuffer<T>::getCapacity() { return this->capacity; }




template <typename T> void FIFOBuffer<T>::push(T elem) {

  if (this->size == 0) { //? No elements in buffer, we can start from the beginning
    this->prev_push_i = 0;
    this->push_i = 0;
    this->pop_i = 0;
    goto PUSH;
  }

  if (this->push_i == this->pop_i) { //? Buffer is full, risk of overwriting
    this->push_i = this->capacity;
    this->next_pop.resize(this->capacity * 2);
    this->buffer.resize(this->capacity * 2);
    this->capacity *= 2;
  }

PUSH:
  this->buffer[this->push_i] = elem;                //? Core push operation on correct push_i
  this->next_pop[this->prev_push_i] = this->push_i; //? next_pop_map updated
  this->prev_push_i = this->push_i++;               //? New prev_push_i is set and push_i increased
  this->push_i %= this->capacity;                   //? Avoid going out of bounds
  this->size++;                                     //? Size increase
}


template <typename T> void FIFOBuffer<T>::pop(T *ret_pointer) {

  if (this->size == 0) return; //? No elements in buffer

  *ret_pointer = this->buffer[this->pop_i];   //? Core pop operation on correct pop_i
  this->pop_i = this->next_pop[this->pop_i];  //? Get next pop_i
  this->size--;                               //? Size decrease
}

//? End of FIFO buffer implementations -------------------------------------------------------------------------






//? Start of LIFO buffer implementations -------------------------------------------------------------------------

template <typename T> LIFOBuffer<T>::LIFOBuffer() {
  this->size = 0;
  this->capacity = 1;

  this->buffer = std::vector<T>(1);
}

template <typename T> LIFOBuffer<T>::LIFOBuffer(long initial_capacity) {
  this->size = 0;
  this->capacity = initial_capacity;

  this->buffer = std::vector<T>(initial_capacity);
}

template <typename T> LIFOBuffer<T>::~LIFOBuffer() { this->buffer.clear(); }




template <typename T> long LIFOBuffer<T>::getSize() { return this->size; }

template <typename T> long LIFOBuffer<T>::getCapacity() { return this->capacity; }




template <typename T> void LIFOBuffer<T>::push(T elem) {

  if (this->size == this->capacity) { //? Buffer full
    this->buffer.resize(this->capacity * 2);
    this->capacity *= 2;
  }

  this->buffer[this->size++] = elem; //? Core push operation
}

template <typename T> void LIFOBuffer<T>::pop(T *ret_pointer) {

  if (this->size == 0) return;

  *ret_pointer = this->buffer[--this->size]; //? Core pop operation
}

//? End of LIFO buffer implementations -------------------------------------------------------------------------
