#ifndef SRC_BOOLEAN_H_
#define SRC_BOOLEAN_H_

#include "pssst.h"

// a better bool?
template <typename B>
struct Boolean {
	friend constexpr B
	operator || (B const &l, B const &r){
		// no shortcut! but no side effect here useful.
		auto const &[vl]=l;
		auto const &[vr]=r;
		return {vl || vr};
	}
	friend constexpr B
	operator && (B const &l, B const &r){
		// no shortcut! but no side effect here useful.
		auto const &[vl]=l;
		auto const &[vr]=r;
		return {vl && vr};
	}
	friend constexpr B
	operator !(B const &l){
		auto const &[vl]=l;
		return {! vl};
	}
};
struct Bool:strong<bool,Bool>,ops<Bool,Boolean>{
	constexpr explicit operator bool() const {
		return val;
	}
};

template <typename U>
using CmpB = Order<U,Bool>;
template <typename U>
using EqB = Eq<U,Bool>;





#endif /* SRC_BOOLEAN_H_ */
