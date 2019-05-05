#include "StrongWithConstructor.h"
#include "cute.h"
#include "pssst.h"
#include <stdexcept>

using namespace Pssst;

// possible but not well suited

namespace StrongWithConstructor{
// affine space: degrees (K and C)
struct degrees:strong<double,degrees>,
Affine<degrees,double>, ops<degrees,Out>{};

struct Kelvin: create_vector_space<Kelvin,degrees>
,ops<Kelvin,Eq,Order,Out>{
	constexpr Kelvin(affine_space::value_type v): create_vector_space<Kelvin,degrees>{v} {
		if(v <0) throw std::logic_error{"can not have negative temperature"};
	}
};

static_assert(sizeof(Kelvin) == sizeof(Kelvin::affine_space::value_type));

void thisIsAKelvinDegreesTest() {
	degrees hotter{20};
	Kelvin spring{15};
	auto x = spring+hotter;
	ASSERT_EQUAL(Kelvin{35},x);
}

void KelvinNegativeThrows() {
	ASSERT_THROWS(Kelvin{-1}, std::logic_error);
}

void KelvinNegativeAfterOperationThrows() {
	Kelvin k{1};
	// no check, because no ctor involved...
//	ASSERT_THROWS((k-=degrees{2}), std::logic_error);
}


}

cute::suite make_suite_StrongWithConstructor() {
	cute::suite s { };
	s.push_back(CUTE(StrongWithConstructor::thisIsAKelvinDegreesTest));
	s.push_back(CUTE(StrongWithConstructor::KelvinNegativeThrows));
	s.push_back(CUTE(StrongWithConstructor::KelvinNegativeAfterOperationThrows));
	return s;
}
