#include<iostream>
#include "test.h"
#include "hello_world.h"
using namespace std;

void test(int num);
void robot(int argc, char** argv); 

int main(int argc, char** argv) {
  test(1);
  robot(argc, argv);
  return 0;
}
