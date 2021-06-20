
#include <iostream>
#include "../flare/util/dict.h"
#include "../flare/util/string.h"

using namespace flare;

// thanks to Substitution failure is not an erro (SFINAE)
template<typename T>
struct has_typedef_value_type {
	typedef char Type1[1];
	typedef char Type2[2];
	template<typename C> static Type1& test(typename C::value_type*);
	template<typename> static Type2& test(...);
	static const bool ret = sizeof(test<T>(0)) == sizeof(Type1); // 0 == NULL
};

struct foo { typedef float lalala; };

void test_template() {

	std::cout << has_typedef_value_type<std::vector<int>>::ret << '\n';
	std::cout << has_typedef_value_type<foo>::ret << '\n';
	
	struct S {
		int a;
	};

	class C: public Object {
		public:
		String to_string() const {
			return String("class C;");
		}
	};

	class D: public Reference {
	};

	int a = 100;
	S s;
	C c;
	D d;
	
	int i = has_object_type<D>::type;
	LOG(i);
	
	LOG(_Str::to_string(&a));
	LOG(_Str::to_string(s));
	LOG(_Str::to_string(c));
	LOG(_Str::to_string(d));

}

int test2_dict(int argc, char *argv[])
{
	Dict<String, String> dict;
	
	dict.set("", "");
	
	printf("%s \n", "ok");
	
	test_template();
	
	return 0;
}