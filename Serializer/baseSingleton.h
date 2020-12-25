#ifndef _BASESINGLETON_
#define _BASESINGLETON_

#include <map>
#include <vector>

template<typename T>
class baseSingleton {
private:
	static T* instance;
public:
	static T* getInstance() {
		if (!instance)
			instance = new T();
		return instance;
	}
};

template<typename I, typename T, typename P>
class baseMapStorrage : public baseSingleton<P> {
protected:
	std::map<I, T*> objects;

	baseMapStorrage() {
		objects = std::map<I, T*>();
	};
public:
	void addOnId(I id, T* obj) {
		objects.insert(std::pair<int,T*>(id, obj));
	}

	T* getById(I ID) {
		auto it = objects.find(ID);
		if (it == objects.end())
			return NULL;
		return it->second;
	}

	std::map<I, T*>* getAll() {
		return &objects;
	}
};

template<typename T, typename P>
class baseVectorStorrage : public baseSingleton<P> {
private:
	std::vector<T*> objects;
protected:
	baseVectorStorrage() {
		objects = std::vector<T*>();
	};
public:
	int addOnId(T* obj) {
		objects.push_back(obj);
		return objects.size() - 1;
	}

	T* getById(int ID) {
		if (ID < 0 || ID >= objects.size())
			return NULL;
		return objects[ID];
	}

	std::vector<T*>* getAll() {
		return &objects;
	}
};

#endif