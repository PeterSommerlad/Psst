#ifndef SRC_PSSST_H_
#define SRC_PSSST_H_
/* PSSST Peter Sommerlad's Simple Strong Type wrapper framework */


#include <type_traits>
#include <ostream>

#include <utility>
namespace Pssst{
// trait: is_ebo
namespace detail{
template <typename EBase>
struct is_ebo_impl{
	struct non_empty{ char x;};
	struct test:std::conditional_t<std::is_class_v<EBase> && !std::is_final_v<EBase>,EBase,non_empty> {
		char c;
	};
	static_assert(sizeof(non_empty)==sizeof(char),"structs should have optimal size");
	static inline constexpr bool value=sizeof(test)==sizeof(non_empty);
};
}
template <typename EBase>
struct is_ebo: std::bool_constant<detail::is_ebo_impl<EBase>::value>{};

template <typename U>
constexpr inline bool is_ebo_v=is_ebo<U>::value;

// should go into detail?

template<typename B, template<typename...>class T>
struct bind2{
	template<typename A, typename ...C>
	using apply=T<A,B,C...>;
};
template<typename A, template<typename...>class T>
struct bind1{
	template<typename ...B>
	using apply=T<A,B...>;
};

// apply multiple operator mix-ins
template <typename U, template <typename ...> class ...BS>
struct ops:BS<U>...{};

// not really needed with structured bindings?
template <typename V, typename TAG>
struct strong { // can not merge ops here, because of initializers required for bases depending on incomplete type
	static_assert(std::is_object_v<V>,"must keep real values - no references or incomplete types allowed");
	using value_type=V;
	V val;
};

template<typename U, typename = void>
struct is_strong_t:std::false_type{};
template<typename U>
struct is_strong_t<U, std::void_t<
typename U::value_type
>>:std::true_type{};

template<typename U>
constexpr inline bool is_strong_v = is_strong_t<U>::value;
//{ std::is_class_v<U> /*&& std::is_aggregate_v<U>*/}; // more universal something in there and not a simple one

namespace detail{
template <typename T,  std::enable_if_t<is_strong_v<T>,int> = 0>
constexpr auto membertype(T x) { // not meant to be evaluated, assumes is_class_v<T>
	auto [y]=x;
	return y;
}
template <typename T, std::enable_if_t<std::is_arithmetic_v<T>||std::is_enum_v<T>,int> = 0> // do not match non-aggregates or pointers
constexpr auto membertype(T x) { // not meant to be evaluated, assumes is_class_v<T>
	return x;
}
}
template <typename T>
using underlying_value_type = decltype(detail::membertype(std::declval<T>()));
template <typename T>
struct default_zero{
	constexpr T operator()() const{
		return T{underlying_value_type<T>{}};
	}
};



template <typename U>
struct is_absolute{
		template<typename T>
		static char test(decltype(T::origin)*);
		template<typename T>
		static long long test(...);
		static inline constexpr bool value=sizeof(test<U>(nullptr))==sizeof(char);
};
template<typename U>
constexpr inline  bool is_absolute_v=is_absolute<U>::value;


static_assert(!is_strong_v<int>,"int is no unit");
static_assert(!is_absolute_v<int>,"int is no absolute unit");


// ops templates


// the following would change with operator<=>
template <typename U, typename Bool=bool>
struct Eq{
	friend constexpr Bool
	operator==(U const &l, U const& r) noexcept {
		auto const &[vl]=l;
		auto const &[vr]=r;
		return {vl == vr};
	}
	friend constexpr Bool
	operator!=(U const &l, U const& r) noexcept {
		return !(l==r);
	}
};

template <typename U, typename Bool=bool>
struct Order{
	friend constexpr Bool
	operator<(U const &l, U const& r) noexcept {
		auto const &[vl]=l;
		auto const &[vr]=r;
		return {vl < vr};
	}
	friend constexpr Bool
	operator>(U const &l, U const& r) noexcept {
		return r < l;
	}
	friend constexpr Bool
	operator<=(U const &l, U const& r) noexcept {
		return !(r < l);
	}
	friend constexpr Bool
	operator>=(U const &l, U const& r) noexcept {
		return !(l < r);
	}
};

// unary
// needed? + only for completeness
template <typename U>
struct UPlus{
	friend constexpr U
	operator+(U const &r){
		auto const &[v]=r;
		return {+v};
	}
};
template <typename U>
struct UMinus{
	friend constexpr U
	operator-(U const &r){
		auto const &[v]=r;
		return {-v};
	}
};


template <typename U>
struct Inc{
	friend constexpr auto operator++(U &rv) noexcept {
		auto &[val]=rv;
		++val;
		return rv;
	}
	friend constexpr auto operator++(U &rv,int) noexcept {
		auto res=rv;
		++rv;
		return res;
	}
};
template <typename U>
struct Dec{
	friend constexpr auto operator--(U &rv) noexcept {
		auto &[val]=rv;
		--val;
		return rv;
	}
	friend constexpr auto operator--(U &rv,int) noexcept {
		auto res=rv;
		--rv;
		return res;
	}

};

/// arithmetic

template <typename R>
struct Add {
	friend constexpr R&
	operator+=(R& l, R const &r) noexcept {
		auto &[vl]=l;
		auto const &[vr] = r;
		vl += vr;
		return l;
	}
	friend constexpr R
	operator+(R l, R const &r) noexcept {
		return l+=r;
	}
};
template <typename U>
struct Sub {
	friend constexpr U&
	operator-=(U& l, U const &r) noexcept {
		auto &[vl]=l;
		auto const &[vr] = r;
		vl -= vr;
		return l;
	}
	friend constexpr U
	operator-(U l, U const &r) noexcept {
		return l-=r;
	}
};

template <typename R, typename BASE, bool>
struct ScalarModulo{};

template <typename R, typename BASE>
struct ScalarModulo<R,BASE,true>{
	friend constexpr
	R&
	operator%=(R l, BASE const &r) noexcept {
		auto &[vl]=l;
		vl %= r;
		return l;
	}
	friend constexpr
	R
	operator%(R l, BASE const &r) noexcept {
		return l%=r;
	}
};



// multiplicative operations with scalar, provide commutativity of *
template <typename R, typename BASE>
struct ScalarMultImpl : ScalarModulo<R,BASE,std::is_integral_v<BASE>> {
	friend constexpr R&
	operator*=(R& l, BASE const &r) noexcept {
		auto &[vl]=l;
		vl *= r;
		return l;
	}
	friend constexpr R
	operator*(R l, BASE const &r) noexcept {
		return l*=r;
	}
	friend constexpr R
	operator*(BASE const & l, R r) noexcept {
		return r*=l;
	}
	friend constexpr R&
	operator/=(R& l, BASE const &r) noexcept {
		auto &[vl]=l;
		// need to check if r is 0 and handle error
		vl /= r; // times 1/r could be more efficient
		return l;
	}
	friend constexpr R
	operator/(R l, BASE const &r) noexcept {
		return l/=r;
	}
};
template<typename BASE>
using ScalarMult=bind2<BASE,ScalarMultImpl>;

template <typename V>
using Additive=ops<V,UPlus,UMinus,Add,Sub,Inc,Dec>;
template <typename V, typename BASE>
using Affine=ops<V,Additive,ScalarMult<BASE>::template apply,Eq,Order>;

//todo other useful arithmetic - * with / by
// combination: arithmetic, relative, absolute_relative combination,
// absolute mix in tag? duration+absolutetag = time_point


template <typename A, typename R>
struct AbsRelArithmetic{
	friend constexpr A&
	operator+=(A& l, R const &r) noexcept {
		auto &[vl]=l;
		auto const &[vr] = r;
		vl += vr;
		return l;
	}
	friend constexpr A
	operator+(A l, R const &r) noexcept {
		return l+=r;
	}
	friend constexpr A
	operator+(R const & l, A r) noexcept {
		return r+=l; // assume commutativity of +
	}
	friend constexpr A&
	operator-=(A& l, R const &r) noexcept {
		auto &[vl]=l;
		auto const &[vr] = r;
		vl -= vr;
		return l;
	}
	friend constexpr A
	operator-(A l, R const &r) noexcept {
		return l+=r;
	}
	friend constexpr R
	operator+(A  const &l, A const &r) noexcept {
		auto const &[vl]=l;
		auto const &[vr] = r;
		return vl-vr;
	}
};


template <typename U>
struct Out {
	friend std::ostream&
	operator<<(std::ostream &l, U const &r) {
		auto const &[v]=r;
		return l << v;
	}
};

template<typename ME, typename VT>
struct Relative:strong<VT,ME>,ops<ME,Affine,Out>{};

// first define affine space operations and then define vector space according to affine space
// operations in vector space can be extended if origin is zero

template <typename ME, typename AFFINE, typename ZEROFUNC=default_zero<AFFINE>>
struct create_vector_space {
	using affine_space=AFFINE;
	using vector_space=ME;
	static_assert(std::is_same_v<affine_space,decltype(ZEROFUNC{}())>, "origin must be in domain affine");
	static inline constexpr affine_space origin=ZEROFUNC{}();
	affine_space offset;
	// linear
	// vs + as
	friend constexpr vector_space&
	operator+=(vector_space& l, affine_space const &r) noexcept {
		l.offset += r;
		return l;
	}
	friend constexpr vector_space
	operator+(vector_space l, affine_space const &r) noexcept {
		return l += r;
	}
	friend constexpr vector_space
	operator+(affine_space const & l, vector_space r) noexcept {
		return r += l;
	}
	// vs - as // caution check if before origin is allowed overflow check
	friend constexpr vector_space&
	operator-=(vector_space& l, affine_space const &r) noexcept {
		l.offset -= r;
		return l;
	}
	friend constexpr vector_space
	operator-(vector_space l, affine_space const &r) noexcept {
		return l -= r;
	}
	// vs - vs = as
	friend constexpr affine_space
	operator-(vector_space const &l,vector_space const &r){
		return l.offset - r.offset;
	}
	// vs + vs, vs * scalar // iff origin == zero and opt-in, not yet.
};
// must be vector spaces from same affine space
template<typename TO, typename FROM>
constexpr TO convertTo(FROM from) noexcept{
	static_assert(std::is_same_v<
			typename FROM::affine_space
			,typename TO::affine_space>);
	return {(from.offset-(TO::origin-FROM::origin))};
}



namespace ___testing___{
struct bla:strong<int,bla>,ops<bla,Eq>{};
static_assert(is_strong_v<bla>,"bla is a unit");
static_assert(!is_absolute_v<bla>,"bla is absolute?");
struct blu:create_vector_space<blu,bla>{};
static_assert(is_absolute_v<blu>,"blu should be absolute");
static_assert(blu::origin==blu::affine_space{0},"blu origin is zero");
static_assert(blu{42}.offset==bla{42}, "rel accessible");
static_assert(std::is_same_v<int,underlying_value_type<bla>>,"..");

struct dummy{int i;};
static_assert(is_ebo_v<Add<dummy>>,"Add should be EBO enabled");
static_assert(is_ebo_v<Sub<dummy>>,"ScalarMult should be EBO enabled");
static_assert(is_ebo_v<Out<dummy>>,"Out should be EBO enabled");
static_assert(is_ebo_v<Order<dummy>>,"Order should be EBO enabled");
static_assert(is_ebo_v<Eq<dummy>>,"Eq should be EBO enabled");
#if 0
static_assert(is_ebo_v<ops<dummy,EqWith<int>::apply>>,"EqWith::apply should be EBO enabled");
static_assert(is_ebo_v<ops<dummy,CmpWith<int>::apply>>,"CmpWith::apply should be EBO enabled");
#endif

struct dummy_d:ops<dummy,Sub,Add> {
	double v;
};
static_assert(sizeof(double)==sizeof(dummy_d),"dummy_d should be same size as double");
}

}


#endif /* SRC_PSSST_H_ */
