#include "Degrees.h"
#include "cute.h"
#include "pssst.h"


using namespace Pssst;
// affine space: degrees (K and C)
struct degrees:strong<double,degrees>,
Affine<degrees,double>, ops<degrees,Out>{};

static_assert(sizeof(double)==sizeof(degrees));


struct Kelvin:create_vector_space<Kelvin,degrees>
             ,ops<Kelvin,Eq,Order,Out>{};

static_assert(sizeof(double)==sizeof(Kelvin));

struct CelsiusZero{
	constexpr degrees operator()() const noexcept{
		return {273.15};
	}
};

struct Celsius:create_vector_space<Celsius,degrees,CelsiusZero>
              , ops<Celsius,Eq,Order,Out>{};
static_assert(sizeof(degrees)==sizeof(Celsius));

constexpr Celsius fromKelvin(Kelvin k)noexcept{
	return {(k.val-(Celsius::origin - Kelvin::origin)).val};
}

constexpr Kelvin fromCelsius(Celsius c)noexcept{
	return {(c.val-(Kelvin::origin-Celsius::origin)).val};
}


void thisIsADegreesTest() {
	degrees hotter{20};
	Celsius spring{15};
	auto x = spring+hotter;
	ASSERT_EQUAL(Celsius{35},x);
}
void thisIsAKelvinDegreesTest() {
	degrees hotter{20};
	Kelvin spring{15};
	auto x = spring+hotter;
	ASSERT_EQUAL(Kelvin{35},x);
}
void testCelsiusFromKelvin(){
	Kelvin zero{273.15};
	zero += degrees{20};
	ASSERT_EQUAL(Celsius{20},fromKelvin(zero));
}

void testKelvinFromCelsius(){
	Celsius boiling{100};
	ASSERT_EQUAL(Kelvin{373.15},fromCelsius(boiling));
}

void testConversion(){
	Celsius mild{20};
	Kelvin k{convertTo<Kelvin>(mild)};
	ASSERT_EQUAL(Kelvin{293.15},k);
	ASSERT_EQUAL(mild,convertTo<Celsius>(k));
}



cute::suite make_suite_Degrees() {
	cute::suite s { };
	s.push_back(CUTE(thisIsADegreesTest));
	s.push_back(CUTE(thisIsAKelvinDegreesTest));
	s.push_back(CUTE(testCelsiusFromKelvin));
	s.push_back(CUTE(testKelvinFromCelsius));
	s.push_back(CUTE(testConversion));
	return s;
}
