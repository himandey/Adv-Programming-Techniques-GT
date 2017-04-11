// Implementation of the templated Vector class
// ECE4893/8893 lab 3
// Himanshu Pandey

#include <iostream> // debugging
#include "Vector.h"
#include <cstdio>
#include <cstring>
#include <stdint.h>

// Your implementation here
// Fill in all the necessary functions below
using namespace std;

// Default constructor
template <typename T>
Vector<T>::Vector()
{
  reserved = 0;
  count = 0;
  elements = NULL;
}

// Copy constructor
template <typename T>
Vector<T>::Vector(const Vector& rhs)
{
  reserved = rhs.reserved;
  count = rhs.count;
  elements = (T *) calloc(count, sizeof(T));
  for(int j=0; j<count; j++)
    {
      elements[j] = rhs.elements[j];
    }
}

// Assignment operator
template <typename T>
Vector<T>& Vector<T>::operator=(const Vector& rhs)
{
  reserved = rhs.reserved;
  count = rhs.count;
  elements = (T *) calloc(count, sizeof(T));
  for(int j=0; j<count; j++)
    {
      elements[j] = rhs.elements[j];
    }
}

/*#ifdef GRAD_STUDENT
// Other constructors
template <typename T>
Vector<T>::Vector(size_t nReserved)
{ // Initialize with reserved memory

}

template <typename T>
Vector<T>::Vector(size_t n, const T& t)
{ // Initialize with "n" copies of "t"
}
#endif
*/

// Destructor
template <typename T>
Vector<T>::~Vector()
{
  T * current;

  for(int32_t j=count-1; j>=0; j--)
    {
      elements[j].~T();
    }
  
  free(elements);
}

// Add and access front and back
template <typename T>
void Vector<T>::Push_Back(const T& rhs)
{
  //if reserved space simply copy
  if(count < reserved)
    {
      new (&elements[count++]) T(rhs);
    }
  //else reallocate space
  else
    {
      count++;
      T * buf = (T *) realloc(elements, count * sizeof(T));
      elements = buf;
      reserved = count;
      new (&elements[count-1]) T(rhs);
    }
}

template <typename T>
void Vector<T>::Push_Front(const T& rhs)
{
  if(count < reserved)
    {
      for(uint32_t j=count-1; j>0; j--)
	{
	  elements[j] = elements[j-1];
	}
      new (&elements[0]) T(rhs);
    }
  else
    {
      count++;
      T * buf = (T *) realloc(elements, count * sizeof(T));
      elements = buf;
      for(uint32_t j=count-1; j>0; j--)
	{
	  new (&elements[j]) T(elements[j-1]);
	  elements[j-1].~T();
	}
      
      reserved = count;
      new (elements) T(rhs);
    }
	  
}

template <typename T>
void Vector<T>::Pop_Back()
{ // Remove last element
  count--;
  elements[count].~T();

}

template <typename T>
void Vector<T>::Pop_Front()
{ // Remove first element
  elements[0].~T(); 
  count--;
  for(uint32_t j=1; j<=count; j++)
    {
      new (&elements[j-1]) T(elements[j]);
      elements[j].~T();
    }
}

// Element Access
template <typename T>
T& Vector<T>::Front() const
{
  return elements[0];
}

// Element Access
template <typename T>
T& Vector<T>::Back() const
{
  return elements[count-1];
}

template <typename T>
T& Vector<T>::operator[](size_t i) const
{
  return elements[i];
}

template <typename T>
size_t Vector<T>::Size() const
{
  return count;
}

template <typename T>
bool Vector<T>::Empty() const
{
  return count == 0;
}

// Implement clear
template <typename T>
void Vector<T>::Clear()
{
  for(uint32_t j=0; j<count; j++)
    {
      elements[j].~T();
    }

  count = 0;
}

// Iterator access functions
template <typename T>
VectorIterator<T> Vector<T>::Begin() const
{
  return VectorIterator<T>(&elements[0]);
}

template <typename T>
VectorIterator<T> Vector<T>::End() const
{
  return VectorIterator<T>(&elements[count]);
}

/*
#ifdef GRAD_STUDENT
// Erase and insert
template <typename T>
void Vector<T>::Erase(const VectorIterator<T>& it)
{
}

template <typename T>
void Vector<T>::Insert(const T& rhs, const VectorIterator<T>& it)
{
}
#endif */

// Implement the iterators

// Constructors
template <typename T>
VectorIterator<T>::VectorIterator()
{
  current = NULL; 
}

template <typename T>
VectorIterator<T>::VectorIterator(T* c)
{
  current = c;
}

// Copy constructor
template <typename T>
VectorIterator<T>::VectorIterator(const VectorIterator<T>& rhs)
{
  current = rhs.current;
}

// Iterator defeferencing operator
template <typename T>
T& VectorIterator<T>::operator*() const
{
  return *current;
}

// Prefix increment
template <typename T>
VectorIterator<T>  VectorIterator<T>::operator++()
{
  current++;
  return *this;
}

// Postfix increment
template <typename T>
VectorIterator<T> VectorIterator<T>::operator++(int)
{
  VectorIterator<T> temp(current);
  current++;
  return temp;
}

// Comparison operators
template <typename T>
bool VectorIterator<T>::operator !=(const VectorIterator<T>& rhs) const
{
  return(current != rhs.current);
}

template <typename T>
bool VectorIterator<T>::operator ==(const VectorIterator<T>& rhs) const
{
  return(current == rhs.current);
}




