#ifndef FACTORY_HPP_
#define FACTORY_HPP_

#include <string>
#include <vector>
#include <map>

/* Singleton */
template <typename interface> class Factory {
	private:
		typedef interface* (*creator_t)();
		typedef std::map<std::string, creator_t> map_t;
		map_t implementations;
	public:
		inline void register_impl(std::string key, creator_t creator) {
			implementations[key] = creator;
		}

		interface* create(std::string key) {
			/* Omitting 'typename' here gives a useless error */
			typename map_t::const_iterator c;
			c = implementations.find(key);
			/* Didn't find it: return NULL */
			if(c == implementations.end()) { return NULL; }
			return c->second();
		}

		std::vector<std::string>* list() {
			std::vector<std::string>* keys =
				new std::vector<std::string>;
			typename map_t::const_iterator k;
			for(k = implementations.begin();
				k != implementations.end(); k++) {
				keys->push_back(k->first);
			}
			return keys;
		}
};

#define FACTORY_REGISTER_IF(interf) \
	public: \
	static Factory<interf>& __factory() {static Factory<interf> f;return f;}
	/* Making that a plain static member seems to forget to allocate it */

/* Use this one OUTSIDE the class scope. You can init static things with static
 * function calls _outside_ of classes, but not as class members. */
#define FACTORY_REGISTER_IMPL(interf,klass) \
	static interf* __##klass##__create() { return new klass; } \
	static int __##klass##__autoregister() { \
		interf::__factory().register_impl(#klass,__##klass##__create); \
		return 0; } \
	static int const __##klass##__autoregistration = \
		__##klass##__autoregister();

#define FACTORY_FOR(interf) \
	(interf::__factory())

#endif

