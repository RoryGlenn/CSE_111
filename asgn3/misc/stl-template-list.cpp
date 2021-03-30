// $Id: stl-template-list.cpp,v 1.7 2020-11-10 16:42:21-08 - - $

#include <iostream>
#include <list>
using namespace std;

int main() {
   list<int> lst {1, 2, 3, 4, 5};
   size_t maxcount = 20;
   cout << "&lst = " << &lst << endl;
   cout << "sizeof lst = " << sizeof lst << endl;
   cout << "&*lst.cbegin() = " << &*lst.cbegin() << endl;
   cout << "&*lst.cend() = " << &*lst.cend() << endl;
   for (auto itor = lst.cbegin(); maxcount-- > 0; ++itor) {
      cout << "&*itor = " << &*itor << " -> " << *itor << endl;
   }
}

/*
//TEST// stl-template-list >stl-template-list.out 2>&1
//TEST// more stl-template-list.cpp stl-template-list.out \
//TEST//      >stl-template-list.lis 2>&1 </dev/null
//TEST// mkpspdf stl-template-list.ps stl-template-list.lis
*/

