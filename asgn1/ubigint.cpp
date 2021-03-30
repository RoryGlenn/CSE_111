// $Id: ubigint.cpp,v 1.12 2020-10-19 13:14:59-07 - - $

#include <cctype>
#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
#include <algorithm>

#include "debug.h"
#include "relops.h"
#include "ubigint.h"

using namespace std;

// to run test file, mk.tests



ubigint::ubigint(unsigned long that) : ubig_value(that)
{
   //DEBUGF ('~', this << " -> " << this->ubig_value)
   
   ubig_value.clear();

   while (that > 0)
   {
      int remainder = that % 10;
      this->ubig_value.push_back(remainder);
      that /= 10;
   }

}

ubigint::ubigint(const string &that) : ubig_value(0)
{
   DEBUGF ('~', "that = \"" << that << "\"");

   this->ubig_value.clear(); // this is new

   for (char digit : that)
   {
      if (!isdigit(digit))
      {
         throw invalid_argument("ubigint::ubigint(" + that + ")");
      }

      int converted_digit = digit - '0';
      ubig_value.push_back(converted_digit);
   }

   reverse(ubig_value.begin(), ubig_value.end());
}


// copy constructor
ubigint::ubigint(const ubigint &a)
{
   this->ubig_value.clear();
   int size = static_cast<int>(a.ubig_value.size());
   for (int i = 0; i < size; i++)
   {
      this->ubig_value.push_back(a.ubig_value[i]);
   }
   
}


ubigint ubigint::operator+(const ubigint &that) const
{

   DEBUGF ('u', *this << "+" << that);
   ubigint result;

   if (this->ubig_value.size() > that.ubig_value.size())
   {
      int size_diff = this->ubig_value.size() - that.ubig_value.size();
      ubigint temp_that = prepend_zeros(that, size_diff);
      result = add_ubigints(temp_that, *this);
   }
   else if (this->ubig_value.size() < that.ubig_value.size())
   {
      int size_diff = that.ubig_value.size() - this->ubig_value.size();
      ubigint temp_this = prepend_zeros(*this, size_diff);
      result = add_ubigints(temp_this, that);
   }
   else if (this->ubig_value.size() == that.ubig_value.size())
   {
      ubigint temp_this = *this;
      result = add_ubigints(*this, that);
   }

   // remove zeros in the back
   while (result.ubig_value.size() > 0 && result.ubig_value.back() == 0)
   {
      result.ubig_value.pop_back();
   }
   
   DEBUGF ('u', result);
   
   return result;
}

ubigint ubigint::operator-(const ubigint &that) const
{

   if (*this < that)
      throw domain_error("ubigint::operator-(a<b)");

   ubigint result;

   if (this->ubig_value.size() > that.ubig_value.size())
   {
      int size_diff = this->ubig_value.size() - that.ubig_value.size();
      ubigint temp_that = prepend_zeros(that, size_diff);
      result = subtract_ubigints(*this, temp_that);
   }
   else if (this->ubig_value.size() < that.ubig_value.size())
   {
      int size_diff = that.ubig_value.size() - this->ubig_value.size();
      ubigint temp_this = prepend_zeros(*this, size_diff);
      result = subtract_ubigints(that, temp_this);
   }
   else if (this->ubig_value.size() == that.ubig_value.size())
   {
      result = subtract_ubigints(*this, that);
   }

   // remove zeros in the back
   while (result.ubig_value.size() > 0 && result.ubig_value.back() == 0)
   {
      result.ubig_value.pop_back();
   }

   return result;
}

ubigint ubigint::operator*(const ubigint &that) const
{
   ubigint product;
   int new_vector_size = this->ubig_value.size();
   new_vector_size += that.ubig_value.size();
   product.ubig_value.assign(new_vector_size, 0);

   int size_i = static_cast<int>(this->ubig_value.size());
   int size_j = static_cast<int>(that.ubig_value.size());

   for (int i = 0; i < size_i; i++)
   {
      int carry = 0;
      for (int j = 0; j < size_j; j++)
      {
         int temp = product.ubig_value[i + j] +
                    this->ubig_value[i] *
                    that.ubig_value[j] +
                    carry;
         product.ubig_value[i + j] = temp % 10;
         carry = temp / 10;
      }

      product.ubig_value[i + size_j] = carry;
   }

   // remove zeros in the back
   while (product.ubig_value.size() > 0 &&
          product.ubig_value.back() == 0)
   {
      product.ubig_value.pop_back();
   }

   return product;
}

void ubigint::multiply_by_2()
{
   ubigint a;
   ubigint b;

   a.ubig_value = this->ubig_value;
   b.ubig_value = this->ubig_value; 
   *this = add_ubigints(a, b);
}

void ubigint::divide_by_2()
{
   int remainder = 0;

   for (int i = this->ubig_value.size() - 1; i >= 0; i--)
   {
      int quotient = 0;

      if (remainder > 0)
      {
         quotient += remainder;
         remainder = 0;
      }

      quotient += this->ubig_value[i] / 2;

      if (ubig_value[i] % 2)
      {
         remainder = 5;
      }

      this->ubig_value[i] = quotient;
   }

   while (this->ubig_value.size() > 0 &&
          this->ubig_value.back() == 0)
   {
      this->ubig_value.pop_back();
   }
}

struct quo_rem
{
   ubigint quotient;
   ubigint remainder;
};


// void ubigint::
//  test_udivide(const ubigint &dividend, const ubigint &divisor_) const
// {
//    ubigint divisor{divisor_};
//    printf("divisor: ");
//    print_vector(divisor);
// }


//                               this      ,           that
quo_rem udivide(const ubigint &dividend, const ubigint &divisor_)
{
   // NOTE: udivide is a non-member function.
   ubigint divisor{divisor_};

   ubigint zero{0};

   if (divisor == zero)
      throw domain_error("udivide by zero");

   ubigint power_of_2{1};
   ubigint quotient{0};
   ubigint remainder{dividend}; // left operand, dividend

   while (divisor < remainder)
   {
      divisor.multiply_by_2();
      power_of_2.multiply_by_2();
   }

   while (power_of_2 > zero)
   {

      // std::cout << divisor << " <= " << remainder << std::endl;
      
      // bool check = (divisor <= remainder);
      // printf("check: %d\n", check);



      if (divisor <= remainder) // < --- error is here !!!
      {
         // std::cout << remainder << " - " << divisor << std::endl;
         // std::cout << quotient << " + " << power_of_2 << std::endl;

         remainder = remainder - divisor;
         quotient = quotient + power_of_2;

         // std::cout << "remainder: " << remainder << std::endl;
         // std::cout << "quotient: " << quotient << std::endl;
      }

      
      divisor.divide_by_2();
      power_of_2.divide_by_2(); // <--- something goes wrong here

   }

   DEBUGF ('/', "quotient = " << quotient);
   DEBUGF ('/', "remainder = " << remainder);

   return {.quotient = quotient, .remainder = remainder};
}

ubigint ubigint::operator/(const ubigint &that) const
{
   // test_udivide(*this, that);
   return udivide(*this, that).quotient;
}

ubigint ubigint::operator%(const ubigint &that) const
{
   return udivide(*this, that).remainder;
}

bool ubigint::operator==(const ubigint &that) const
{
   if (this->ubig_value.size() != that.ubig_value.size())
   {
      return false;
   }

   for (long unsigned int i = 0; i < this->ubig_value.size(); i++)
   {
      if (this->ubig_value[i] != that.ubig_value[i])
      {
         return false;
      }
   }

   return true;
}

bool ubigint::operator<(const ubigint &that) const
{
   // printf("ubigint::operator<\n");


   if (this->ubig_value.size() < that.ubig_value.size())
   {
      return true;
   }
   else if (this->ubig_value.size() > that.ubig_value.size())
   {
      return false;
   }
   else
   {
      int size = static_cast<int>(that.ubig_value.size() - 1);

      for (int i = size; i >= 0; i--)
      {

         if (this->ubig_value[i] < that.ubig_value[i])
         {
            return true;
         }
         else if (this->ubig_value[i] > that.ubig_value[i])
         {
            return false;
         }
      }
   }

   return false;
}

ostream &operator<<(ostream &out, const ubigint &that)
{
   auto begin = that.ubig_value.rbegin();
   auto end = that.ubig_value.rend();
   int count = 0;

   for (; begin != end; ++begin)
   {
      out << +(*begin);
      count += 1;

      if (count % 69 == 0 && that.ubig_value.size() > 69)
      {
         out << "\\" << "\n";
      }
   }

   return out;
}

///////////////////////////////////////////////////////
//           Custom functions for asgn1
///////////////////////////////////////////////////////

void ubigint::print_vector(ubigint a)
{
   for (long unsigned int i = 0; i < a.ubig_value.size(); i++)
   {
      printf("%d", a.ubig_value[i]);
   }
   printf("\n");
}

void ubigint::print_vector(ubigint &a)
{
   for (long unsigned int i = 0; i < a.ubig_value.size(); i++)
   {
      printf("%d", a.ubig_value[i]);
   }
   printf("\n");
}

void ubigint::print_vector(const ubigint &a)
{
   for (long unsigned int i = 0; i < a.ubig_value.size(); i++)
   {
      printf("%d", a.ubig_value[i]);
   }
   printf("\n");
}

void ubigint::print_vector(ubigint &a) const
{
   for (long unsigned int i = 0; i < a.ubig_value.size(); i++)
   {
      printf("%d", a.ubig_value[i]);
   }
   printf("\n");
}

void ubigint::print_vector(const ubigint &a) const
{
   for (long unsigned int i = 0; i < a.ubig_value.size(); i++)
   {
      printf("%d", a.ubig_value[i]);
   }
   printf("\n");
}

void ubigint::print_vector(ubigint *a)
{
   for (long unsigned int i = 0; i < a->ubig_value.size(); i++)
   {
      printf("%d", a->ubig_value[i]);
   }
   printf("\n");
}

void ubigint::print_vector(ubigvalue_t _ubig_value)
{
   for (long unsigned int i = 0; i < ubig_value.size(); i++)
   {
      printf("%d", _ubig_value[i]);
   }
   printf("\n");
}

ubigint ubigint::
    add_ubigints(const ubigint a, const ubigint b) const
{
   ubigint result;
   int carry = 0;

   for (size_t i = 0; i < a.ubig_value.size(); i++)
   {
      int sum = a.ubig_value[i] + b.ubig_value[i] + carry;
      int remainder = sum % 10;
      carry = sum / 10;
      result.ubig_value.push_back(remainder);
   }

   if (carry > 0)
   {
      result.ubig_value.push_back(carry);
   }

   return result;
}

ubigint ubigint::
    subtract_ubigints(const ubigint a, const ubigint b) const
{

   ubigint result;
   int carry = 0;
   int a_size = a.ubig_value.size();
   int b_size = b.ubig_value.size();

   for (int i = 0; i < b_size; i++)
   {
      int difference = a.ubig_value[i] - b.ubig_value[i] - carry;

      if (difference < 0)
      {
         difference += 10;
         carry = 1;
      }
      else
      {
         carry = 0;
      }

      result.ubig_value.push_back(difference);
   }

   for (int j = b_size; j < a_size; j++)
   {
      int difference = a.ubig_value[j] - carry;

      if (difference < 0)
      {
         difference += 10;
         carry = 1;
      }
      else
      {
         carry = 0;
      }

      result.ubig_value.push_back(difference);
   }

   return result;
}

ubigint ubigint::pop_back_zeros(ubigint a) const
{
   ubigint copy;
   copy.ubig_value = a.ubig_value;

   while (true)
   {
      if (copy.ubig_value.back() != 0)
      {
         break;
      }

      copy.ubig_value.pop_back();
   }

   return copy;
}

ubigint ubigint::prepend_zeros(const ubigint a, int size_diff) const
{
   ubigint temp;
   temp.ubig_value = a.ubig_value;

   for (int i = 0; i < size_diff; i++)
   {
      temp.ubig_value.push_back(0);
   }

   return temp;
}

