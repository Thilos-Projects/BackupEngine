#ifndef _BASEFACTORY_
#define _BASEFACTORY_

#include <map>
#include <string>

template<typename T, typename baseType>
baseType* createT() { return new T; }

template<typename baseType>
struct baseFactory {
public:
	typedef std::map<std::string, baseType* (*)()> map_type;

	static baseType* createInstance(std::string const& s) {
		auto it = getMap()->find(s);
		if (it == getMap()->end())
			return 0;
		return it->second();
	}

protected:
	static map_type* getMap() {
		if (!map) { map = new map_type; }
		return map;
	}

private:
	static map_type* map;
};


#endif