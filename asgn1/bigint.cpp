// $Id: bigint.cpp,v 1.3 2020-10-11 12:47:51-07 - - $

#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
#include <vector>
#include "bigint.h"
#include "ubigint.h" // not sure if I need this

using namespace std;

// void bigint::print_bigint(const bigint a) const
// {
//    a.uvalue.print_vector(a.uvalue);
// }

bigint::bigint(long that) : uvalue(that), is_negative(that < 0)
{
   DEBUGF('~', this << " -> " << uvalue)
}


bigint::bigint(const ubigint &uvalue_, bool is_negative_) : 
   uvalue(uvalue_), is_negative(is_negative_)
{
}

bigint::bigint(const string &that)
{
   is_negative = that.size() > 0 and that[0] == '_';
   uvalue = ubigint(that.substr(is_negative ? 1 : 0));
}

bigint bigint::operator+() const
{
   return *this;
}

bigint bigint::operator-() const
{
   return {uvalue, not is_negative};
}

bigint bigint::operator+(const bigint &that) const
{

   bigint result;

   if (this->is_negative && that.is_negative)
   {
      result = this->uvalue + that.uvalue;
      result.is_negative = true;
   }
   else if (!this->is_negative && !that.is_negative)
   {
      result = this->uvalue + that.uvalue;
      result.is_negative = false;
   }
   else
   {
      if (this->uvalue > that.uvalue)
      {
         result = this->uvalue - that.uvalue;
         result.is_negative = this->is_negative;
      }
      else if (this->uvalue < that.uvalue)
      {
         result = that.uvalue - this->uvalue;
         result.is_negative = that.is_negative;
      }
      else
      {
         result = that.uvalue - this->uvalue;
         result.is_negative = false;
      }
   }

   return result;
}

bigint bigint::operator-(const bigint &that) const
{

   bigint result;

   if (this->is_negative != that.is_negative)
   {
      result.uvalue = this->uvalue + that.uvalue;

      if (this->uvalue > that.uvalue)
      {
         result.is_negative = this->is_negative;
      }
      else if (this->uvalue < that.uvalue)
      {
         result.is_negative = that.is_negative;
      }
   }
   else
   {
      if (this->uvalue > that.uvalue) 
      {
         result.uvalue = this->uvalue - that.uvalue;
         result.is_negative = false;
      }
      else if (this->uvalue < that.uvalue)
      {
         result.uvalue = that.uvalue - this->uvalue;
         result.is_negative = true;
      }
      else
      {
         result.uvalue = that.uvalue - this->uvalue;
         result.is_negative = false;
      }
   }

   return result;
}

bigint bigint::operator*(const bigint &that) const
{
   bigint result;
   result.is_negative = true;
   result.uvalue = this->uvalue * that.uvalue;

   if ((this->is_negative && that.is_negative) ||
       (!this->is_negative && !that.is_negative))
   {
      result.is_negative = false;
   }

   return result;
}

bigint bigint::operator/(const bigint &that) const
{
   bigint result;
   result.is_negative = true;
   result.uvalue = this->uvalue / that.uvalue;

   if ((this->is_negative && that.is_negative) ||
       (!this->is_negative && !that.is_negative))
   {
      result.is_negative = false;
   }

   return result;
}

bigint bigint::operator%(const bigint &that) const
{
   bigint result;
   result.is_negative = true;
   result.uvalue = this->uvalue % that.uvalue;


   if ((this->is_negative && that.is_negative) ||
       (!this->is_negative && !that.is_negative))
   {
      result.is_negative = false;
   }

   return result;
}

bool bigint::operator==(const bigint &that) const
{
   return (is_negative == that.is_negative) && (uvalue == that.uvalue);
}

bool bigint::operator<(const bigint &that) const
{
   if (is_negative != that.is_negative)
      return is_negative;
   return is_negative ? uvalue > that.uvalue
                      : uvalue < that.uvalue;
}

ostream &operator<<(ostream &out, const bigint &that)
{

   if (that.is_negative)
   {
      out << "-" << that.uvalue;
   }
   else
   {
      out << that.uvalue;
   }

   return out;

}
