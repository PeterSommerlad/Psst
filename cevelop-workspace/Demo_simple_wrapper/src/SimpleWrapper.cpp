#include "cute.h"
#include "ide_listener.h"
#include "xml_listener.h"
#include "cute_runner.h"

//----------- demo for consumption -----

struct literGas{
	double l;
};
struct kmDriven{
	double km;
};
struct literPer100km {
	double consumption;
};

literPer100km consumption(literGas l, kmDriven km) {
	return {l.l/(km.km/100.0)}; // needs curlies
}

void testConsumption1over1(){
	literGas const l {1} ;
	kmDriven const km { 1 } ;
	ASSERT_EQUAL(100.0,consumption(l,km).consumption);
}

void testConsumption40over500(){
	literGas const l { 40 };
	kmDriven const km { 500 };
	ASSERT_EQUAL(8.0,consumption(l,km).consumption);
}

void testConsumption40over500Wrong(){
	literGas const l { 40 };
	kmDriven const km { 500 };
//	ASSERT_EQUAL(8.0,consumption(km,l).consumption); // no compile
}
void testConsumptionEvenMoreStrange(){
//	ASSERT_EQUAL(8.0,consumption(consumption({40},{500}),{100})); // no compile
}


bool runAllTests(int argc, char const *argv[]) {
	cute::suite s { };
	s.push_back(CUTE(testConsumption1over1));
	s.push_back(CUTE(testConsumption40over500));
	s.push_back(CUTE(testConsumption40over500Wrong));
	s.push_back(CUTE(testConsumptionEvenMoreStrange));
	//TODO add your test here
	cute::xml_file_opener xmlfile(argc, argv);
	cute::xml_listener<cute::ide_listener<>> lis(xmlfile.out);
	auto runner = cute::makeRunner(lis, argc, argv);
	bool success = runner(s, "AllTests");
	return success;
}

int main(int argc, char const *argv[]) {
    return runAllTests(argc, argv) ? EXIT_SUCCESS : EXIT_FAILURE;
}
