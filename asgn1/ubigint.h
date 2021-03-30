// $Id: ubigint.h,v 1.5 2020-10-11 12:25:22-07 - - $

#ifndef __UBIGINT_H__
#define __UBIGINT_H__

#include <exception>
#include <iostream>
#include <limits>
#include <utility>
#include <vector>
#include "debug.h"
using namespace std;

class ubigint
{
   friend ostream &operator<<(ostream &, const ubigint &);

private:
   using ubigvalue_t = vector<uint8_t>;
   ubigvalue_t ubig_value;

public:
   void multiply_by_2();
   void divide_by_2();

   ubigint() = default; // Need default ctor as well.
   ubigint(unsigned long);
   ubigint(const string &);

   // copy constructor
   ubigint(const ubigint &a);

   // why doesn't this work?
   // ubigint(const ubigint &a) { this->ubig_value = a.ubig_value; }

   ubigint operator+(const ubigint &) const;
   ubigint operator-(const ubigint &) const;
   ubigint operator*(const ubigint &) const;
   ubigint operator/(const ubigint &) const;
   ubigint operator%(const ubigint &) const;

   bool operator==(const ubigint &) const;
   bool operator<(const ubigint &) const;
   
   // mine
   ubigint add_ubigints(const ubigint a, const ubigint b) const;
   ubigint subtract_ubigints(const ubigint a, const ubigint b) const;
   ubigint prepend_zeros(const ubigint a, int size_diff) const;
   ubigint pop_back_zeros(ubigint a) const;

   // print functions
   void print_vector(ubigint a);
   void print_vector(ubigint &a);
   void print_vector(ubigint &a) const;
   void print_vector(const ubigint &a);
   void print_vector(const ubigint &a) const;
   void print_vector(ubigint *a);
   void print_vector(ubigvalue_t _ubig_value);


   // void test_udivide(const ubigint &dividend, const ubigint &divisor_) const;


};

#endif
