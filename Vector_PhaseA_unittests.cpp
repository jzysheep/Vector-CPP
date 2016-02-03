#include <iostream>
#include <cstdint>
#include <stdexcept>
#include <future>
#include <chrono>
#include "gtest/gtest.h"
#include "Vector.h"

using std::cout;
using std::endl;
using epl::Vector;

//TEST SUITE 1
TEST(PhaseA, basic_functionality) {
	Vector<int> x;
	EXPECT_EQ(0, x.size());

	x.push_back(42);
	EXPECT_EQ(1, x.size());
	EXPECT_EQ(42, x[0]);

	x[0] = 10;

	EXPECT_EQ(10, x[0]);

}

TEST(PhaseA, constructors) {
	Vector<int> x;
	EXPECT_EQ(0, x.size());
	x.push_back(42);
	Vector<int> y{ x };
	EXPECT_EQ(1, y.size());
	EXPECT_EQ(42, y[0]);
	y[0] = 10;
	EXPECT_NE(10, x[0]);

	Vector<int> z(10); // must use () to avoid ambiguity over initializer list
	EXPECT_EQ(10, z.size());
}

TEST(PhaseA, range_checks) {
	Vector<int> x(10);
	EXPECT_NO_THROW(
	for (int k = 0; k < 10; k += 1) {
		x[k] = k;
	});

	//EXPECT_THROW(x[10] = 42, std::out_of_range);
}

/* Still under development
TEST(PhaseA, push_back) {
	Vector<int> x;
	std::future<bool> build = std::async(
		[&x](void) {
		const uint32_t size = 100000;
		for (uint32_t k = 0; k < size; k += 1) {
			x.push_back(k);
		}
		return x.size() == size && x[0] == 0 && x[size - 1] == size - 1;
	});

	std::chrono::milliseconds timeout{ 100 };
	auto status = build.wait_for(timeout);
	EXPECT_EQ(std::future_status::ready, status);
	EXPECT_TRUE(build.get());
}
*/

//TEST SUITE 2
namespace{
  //Class Instrumentation
  class Foo {
  public:
    bool alive;
    
    static uint64_t constructions;
    static uint64_t destructions;
    static uint64_t copies;
    static void reset(){ copies=destructions=constructions=0; }

    Foo(void) { alive = true; ++constructions; }
    ~Foo(void) { destructions += alive; }
    Foo(const Foo&) noexcept { alive = true; ++copies; }
    Foo(Foo&& that) noexcept { that.alive = false; this->alive = true; }
    Foo& operator=(const Foo& that) noexcept { alive=true; copies += 1; return *this; }
    //Foo& operator=(Foo&& that) noexcept { that.alive = false; this->alive = true; return *this; }
  };
  uint64_t Foo::constructions = 0;
  uint64_t Foo::destructions = 0;
  uint64_t Foo::copies = 0;

} //namespace

//Ghetto C-style way to find the size of an array
//Only used in next test
#define ARRAY_SIZE(X) (sizeof(X)/sizeof(*X))
TEST(PhaseA2, PushBackFront){
  Vector<int> x; // creates an empty Vector
  EXPECT_EQ(0, x.size());
  x.push_back(42); 
  EXPECT_EQ(1, x.size());
  for (int k = 0; k < 10; k += 1) {
    x.push_back(k);
    x.push_front(k);
  }
  int ans[] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 42, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  EXPECT_EQ(ARRAY_SIZE(ans), x.size());
  for(unsigned int i=0; i<ARRAY_SIZE(ans); ++i)
    EXPECT_EQ(x[i], ans[i]);
}

TEST(PhaseA2, CopyConstruct){
  Vector<int> x;
  x.push_back(42);
  Vector<int> y(x); // copy constructed
  x.push_back(0);
  
  EXPECT_EQ(1, y.size());
  EXPECT_EQ(42, y[0]);
  
  y.pop_back();
  EXPECT_EQ(0, y.size());
  EXPECT_EQ(2, x.size());
  EXPECT_EQ(42, x[0]);
  EXPECT_EQ(0, x[1]);
}

TEST(PhaseA2, FooCtorDtor){
  Foo::reset();
  {
    Vector<Foo> x(10); // 10 default-constructed Foo objects
    for (int k = 0; k < 11; ++k) { 
      x.push_back(Foo()); // default-construct temp, then move it
    }
  } //ensures x is destroyed


  EXPECT_EQ(21, Foo::constructions);
  EXPECT_EQ(32, Foo::destructions); // 21 for B
  EXPECT_EQ(11, Foo::copies); // 0 for B
}

/* I'm offering no guidance here, other than this is a case you should explore */
TEST(PhaseA2, ReallocCopy){
  Foo::reset();
  {
    Vector<Vector<Foo>> x(3);
    x[0].push_back(Foo()); //1 alive Foo
    x.push_back(x[0]); //1 copy, 2 alive Foo
  } //ensures x is destroyed

  EXPECT_EQ(1, Foo::constructions);
  EXPECT_EQ(4, Foo::destructions); // 2 for B
  EXPECT_EQ(3, Foo::copies); // 1 for B
}

TEST(PhaseA3, FooUnderFoo){
  Foo::reset();
  {
          using V = Vector<Vector<Vector<Foo>>>;
          using VV = Vector<Vector<Foo>>;
          using VVV = Vector<Foo>;
          V x(1);
          x.push_back(VV());
          x[0].push_back(VVV());
          x[0][0].push_back(Foo());
          x[0].push_back(x[0][0]);
          x.push_back(x[0]);
  }

  EXPECT_EQ(1, Foo::constructions);
  EXPECT_EQ(7, Foo::destructions);
  EXPECT_EQ(6, Foo::copies);

//  cout<<"Foo::constructions = "<<Foo::constructions<<endl;
//  cout<<"Foo::destructions = "<<Foo::destructions<<endl;
//  cout<<"Foo::copies = "<<Foo::copies<<endl;
}


TEST(PhaseA3, FooZeroCap){
  Foo::reset();
  Vector<Foo> x;
  {
          for(int32_t i=0; i<20; i++) {
                x.push_back(Foo());
                x.push_front(Foo());
                x.pop_front();
                x.pop_front();
          }
  }
  EXPECT_EQ(40, Foo::constructions);
  EXPECT_EQ(80, Foo::destructions);
  EXPECT_EQ(40, Foo::copies);
  EXPECT_EQ(0, x.size());
}

TEST(PhaseA3, FooEqAssign) {
  Foo::reset();
  Vector<Foo> x(40);
  Vector<Foo> y;
  y = x;
  for(int32_t i = 0; i< 40; i++) x.pop_back();
  EXPECT_EQ(40, y.size());
  EXPECT_EQ(0, x.size());
  x = y;
  EXPECT_EQ(40, x.size());
  for(int32_t i = 0; i< 40; i++) y.pop_back();
  x = y;
  EXPECT_EQ(0, x.size());
  EXPECT_EQ(0, y.size());
  EXPECT_EQ(40, Foo::constructions);
  EXPECT_EQ(120, Foo::destructions);
  EXPECT_EQ(80, Foo::copies);
}

TEST(PhaseA3, IntCorrectAccess) {
    // Zero argument constructor.
    Vector<int> x1;
    for(int i=0; i <8; ++i) {
        x1.push_back(i);
    }

    // Check Basic push_back and push_front.
    EXPECT_EQ(x1[0], 0);
    EXPECT_EQ(x1[7], 7);
    
    x1.push_front(11);
    x1.push_back(12);

    EXPECT_EQ(x1.size(), 10);
    EXPECT_EQ(x1[0], 11);
    EXPECT_EQ(x1[9], 12);

    // 5-initialize vector.
    Vector<int> x2(5);
    // Check push-pop front-back combinations.
    for(int i=0; i <5; ++i) {
        x2.pop_back();
    }
    EXPECT_EQ(x2.size(), 0);

    for(int i=0; i <5; ++i) {
        x2.push_front(10);
    }
    EXPECT_EQ(x2.size(), 5);

    for(int i=0; i <5; ++i) {
        x2.pop_front();
    }
    EXPECT_EQ(x2.size(), 0);

    for(int i=0; i<6; ++i) {
        x2.push_front(i);
    }
    EXPECT_EQ(x2[0], 5);
    EXPECT_EQ(x2[5], 0);

    // Copy Construction.
    Vector<int> x3 = x2;
    EXPECT_EQ(x3.size(), x2.size());
    EXPECT_EQ(x3[0], x2[0]);
    EXPECT_EQ(x3[5], x3[5]);

    x3.push_back(11);
    x3.push_front(12);
    EXPECT_EQ(x3.size(), 8);

    x3.push_front(13);
    EXPECT_EQ(x3.size(), 9);

    Vector<int> x4;
    x4 = x1;
    EXPECT_EQ(x4[0], x1[0]);
    EXPECT_EQ(x4[x4.size() - 1], x1[x4.size() - 1]);
}

//this is the main entry point for the program.  Other
//tests can be in other cpp files, as long as there is
//only one of these main functions.
int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}