#include "Consumption.h"
#include "pssst.h"
#include "cute.h"

using namespace Pssst;
struct literGas
		: strong<double,literGas>
		, ops<literGas,Additive,Order,Out>{};

struct literPer100km
		:strong<double,literPer100km>
		,ops<literPer100km,Eq,Out>{};

//struct kmDriven0:strong<double,kmDriven0>
//,ops<kmDriven0, ScalarMult<double>::template apply>{};

struct kmDriven:strong<double,kmDriven>
,ScalarMultImpl<kmDriven,double>{};

static_assert(sizeof(double)==sizeof(kmDriven));


literPer100km consumption(literGas l, kmDriven km) {
	return {l.value/(km/100.0).value};
}

void testConsumption1over1(){
	literGas const l {1} ;
	kmDriven const km { 1 } ;
	ASSERT_EQUAL(literPer100km{100.0},consumption(l,km));
}

void testConsumption40over500(){
	literGas const l { 40 };
	kmDriven const km { 500 };
	ASSERT_EQUAL(literPer100km{8.0},consumption(l,km));
}

// try mix-in without strong...



struct liter : ops<liter,Additive,Order,Out>{
	// needs ctor to avoid need for extra {}
	constexpr explicit liter(double lit):l{lit}{};
	double l{};
};
static_assert(sizeof(liter)==sizeof(double)); // ensure empty bases are squashed

void testLiterWithoutStrong(){
	liter l1 {43. };
	liter l2 {42.1 };
	ASSERT_EQUAL_DELTA(l1,l2,liter{0.9});
}



cute::suite make_suite_Consumption() {
	cute::suite s { };
	s.push_back(CUTE(testConsumption1over1));
	s.push_back(CUTE(testConsumption40over500));
	s.push_back(CUTE(testLiterWithoutStrong));
	return s;
}
