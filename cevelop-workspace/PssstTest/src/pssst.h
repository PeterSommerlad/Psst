#ifndef SRC_PSSST_H_
#define SRC_PSSST_H_
/* PSSST Peter Sommerlad's Simple Strong Type wrapper framework */


#include <type_traits>
#include <iosfwd> // <ostream> not really needed IMHO, because template definition only.

namespace Pssst{


// apply multiple operator mix-ins
template <typename U, template <typename ...> class ...BS>
struct ops:BS<U>...{};

// not really needed with structured bindings?
template <typename V, typename TAG>
struct strong { // can not merge ops here, because of initializers required for bases depending on incomplete type
	static_assert(std::is_object_v<V>,"must keep real values - no references or incomplete types allowed");
	using value_type=V;
	V val{};
};


namespace detail{
template<typename U, typename = void>
struct is_strong_t:std::false_type{};
template<typename U>
struct is_strong_t<U, std::void_t<
typename U::value_type
>>:std::true_type{};

template<typename U>
constexpr inline bool is_strong_v = is_strong_t<U>::value;


template <typename T,  std::enable_if_t<is_strong_v<T>,int> = 0>
constexpr auto membertype(T x) { // not meant to be evaluated, assumes is_class_v<T>
	auto [y]=x;
	return y;
}
template <typename T, std::enable_if_t<std::is_arithmetic_v<T>||std::is_enum_v<T>,int> = 0> // do not match non-aggregates or pointers
constexpr auto membertype(T x) { // not meant to be evaluated, assumes is_class_v<T>
	return x;
}
template <typename T>
using underlying_value_type = decltype(detail::membertype(std::declval<T>()));

// meta-binders for first or second template argument
// only bind2 is used here
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

} // detail

template <typename T>
struct default_zero{
	constexpr T operator()() const{
		return T{detail::underlying_value_type<T>{}};
	}
};

// Operator Mix-in templates. contain operator functions as hidden friends
// rely on structured bindings, so the sole value member must be public
// because friendship is not inherited
// allow for a "strong" boolean type, to avoid integral promotion


// the following would change with operator<=>, but we target C++17 for now
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

// unary plus and minus, plus only for completeness, because it usually is a no-op
// needed? + only for completeness
template <typename U>
struct UPlus{
	friend constexpr U
	operator+(U const &r){
		auto const &[v]=r;
		return U{+v};
	}
};
template <typename U>
struct UMinus{
	friend constexpr U
	operator-(U const &r){
		auto const &[v]=r;
		return U{-v};
	}
};

// this is how many other std::math functions could be supported
template <typename U>
struct Abs{
		friend constexpr U
		abs(U const &r){
			auto const &[v]=r;
			return U{std::abs(v)}; // might need to use using std::abs in a trampoline
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


// shorthand for usual additive operations, which might be common
template <typename V>
using Additive=ops<V,UPlus,UMinus,Abs,Add,Sub,Inc,Dec>;

// output is simple. one could use reflection to mark the strong type
// but mostly this is for testing purposes only, so that ASSERT_EQUAL produces useful output
// on failure
template <typename U>
struct Out {
	friend std::ostream&
	operator<<(std::ostream &out, U const &r) {
		auto const &[v]=r;
		return out << v;
	}
};


// prepare for 1-D linear space

// modulo operations only allowed if base type is integral
// use boost::safe_int for checking for zero
template <typename R, typename BASE, bool=false>
struct ScalarModulo{};

template <typename R, typename BASE>
struct ScalarModulo<R,BASE,true>{
	friend constexpr
	R&
	operator%=(R l, BASE const &r) noexcept {
		auto &[vl]=l;
		vl %= r; // should we guard against div by zero?
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

// scalar multiplication must know the scalar type
template<typename BASE>
using ScalarMult=detail::bind2<BASE,ScalarMultImpl>;

// a 1-d linear space without origin (or implicit zero)
template <typename V, typename BASE>
using Affine=ops<V,Additive,ScalarMult<BASE>::template apply,Eq,Order>;

template <typename V, typename BASE>
using Linear=Affine<V,BASE>; // need to check which name is better

// VS is "absolute" vector space, AS is "relative" affine space
template <typename VS, typename AS>
struct AbsRelArithmetic{
	friend constexpr VS&
	operator+=(VS& l, AS const &r) noexcept {
		auto &[vl]=l;
		auto const &[vr] = r;
		vl += vr;
		return l;
	}
	friend constexpr VS
	operator+(VS l, AS const &r) noexcept {
		return l+=r;
	}
	friend constexpr VS
	operator+(AS const & l, VS r) noexcept {
		return r+=l; // assume commutativity of +
	}
	friend constexpr VS&
	operator-=(VS& l, AS const &r) noexcept {
		auto &[vl]=l;
		auto const &[vr] = r;
		vl -= vr;
		return l;
	}
	friend constexpr VS
	operator-(VS l, AS const &r) noexcept {
		return l+=r;
	}
	friend constexpr AS
	operator+(VS  const &l, VS const &r) noexcept {
		auto const &[vl]=l;
		auto const &[vr] = r;
		return AS{vl-vr};
	}
};

// first define affine space operations and then define vector space according to affine space
// operations in vector space can be extended if origin is zero

template <typename ME, typename AFFINE, typename ZEROFUNC=default_zero<AFFINE>>
struct create_vector_space {
	using affine_space=AFFINE;
	using vector_space=ME;
	static_assert(std::is_same_v<affine_space,decltype(ZEROFUNC{}())>, "origin must be in domain affine");
	static inline constexpr affine_space origin=ZEROFUNC{}();
	affine_space val{};
	// linear
	// vs + as
	friend constexpr vector_space&
	operator+=(vector_space& l, affine_space const &r) noexcept {
		 l.val += r;
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
		l.val -= r;
		return l;
	}
	friend constexpr vector_space
	operator-(vector_space l, affine_space const &r) noexcept {
		return l -= r;
	}
	// vs - vs = as
	friend constexpr affine_space
	operator-(vector_space const &l,vector_space const &r){
		return l.val - r.val;
	}
	// vs + vs, vs * scalar // iff origin == zero and opt-in, not yet.
};
template <typename ME, typename AFFINE, typename ZEROFUNC=default_zero<AFFINE>>
struct create_vector_space_checked {
	using affine_space=AFFINE;
	using vector_space=ME;
	static_assert(std::is_same_v<affine_space,decltype(ZEROFUNC{}())>, "origin must be in domain affine");
	static inline constexpr affine_space origin=ZEROFUNC{}();
	affine_space val{};
	// linear
	// vs + as
	friend constexpr vector_space&
	operator+=(vector_space& l, affine_space const &r)  {
		auto &[affine] = l;
		//l.val += r; // need domain check here
		l = vector_space{affine + r};
		return l;
	}
	friend constexpr vector_space
	operator+(vector_space l, affine_space const &r)  {
		return l += r;
	}
	friend constexpr vector_space
	operator+(affine_space const & l, vector_space r)  {
		return r += l;
	}
	// vs - as // caution check if before origin is allowed overflow check
	friend constexpr vector_space&
	operator-=(vector_space& l, affine_space const &r) {
		auto &[affine] = l;
		l = vector_space{affine-r}; // domain check
//		l.val -= r;
		return l;
	}
	friend constexpr vector_space
	operator-(vector_space l, affine_space const &r)  {
		return l -= r;
	}
	// vs - vs = as
	friend constexpr affine_space
	operator-(vector_space const &l,vector_space const &r) {
		return l.val - r.val;
	}
	// vs + vs, vs * scalar // iff origin == zero and opt-in, not yet.
};

// must be vector spaces from same affine space
template<typename TO, typename FROM>
constexpr TO convertTo(FROM from) noexcept{
	static_assert(std::is_same_v<
			typename FROM::affine_space
			,typename TO::affine_space>);
	return {(from.val-(TO::origin-FROM::origin))};
}



namespace ___testing___{
using detail::is_strong_v;
using detail::underlying_value_type;

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



struct bla:strong<int,bla>,ops<bla,Eq>{};
static_assert(is_strong_v<bla>,"bla is a unit");
static_assert(!is_absolute_v<bla>,"bla is absolute?");
struct blu:create_vector_space<blu,bla>{};
static_assert(is_absolute_v<blu>,"blu should be absolute");
static_assert(blu::origin==blu::affine_space{0},"blu origin is zero");
static_assert(blu{42}.val==bla{42}, "rel accessible");
static_assert(std::is_same_v<int,underlying_value_type<bla>>,"..");


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

struct dummy{int i;};
static_assert(is_ebo_v<Add<dummy>>,"Add should be EBO enabled");
static_assert(is_ebo_v<Sub<dummy>>,"ScalarMult should be EBO enabled");
static_assert(is_ebo_v<Out<dummy>>,"Out should be EBO enabled");
static_assert(is_ebo_v<Order<dummy>>,"Order should be EBO enabled");
static_assert(is_ebo_v<Eq<dummy>>,"Eq should be EBO enabled");

struct dummy_d:ops<dummy,Sub,Add> {
	double v;
};
static_assert(sizeof(double)==sizeof(dummy_d),"dummy_d should be same size as double");
}

}


#endif /* SRC_PSSST_H_ */
