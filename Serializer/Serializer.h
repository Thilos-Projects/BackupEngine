#ifndef _SERIALIZER_H_
#define _SERIALIZER_H_

#include "baseSingleton.h"
#include "baseFactory.h"

#include <map>
#include <vector>
#include <string>

struct SerializedObject;

class Serializable {
public:
	virtual void set(SerializedObject& from) = 0;
	virtual void afterSet() = 0;
	virtual SerializedObject serialize() = 0;
	virtual SerializedObject serialize(SerializedObject& inTo) = 0;
};

struct SerializedObject{
	std::vector<uint8_t> body;

protected:
	friend SerializedObject& operator << (SerializedObject& msg, SerializedObject& data)
	{
		size_t start = msg.body.size();
		size_t bodySize = data.body.size() * sizeof(uint8_t);

		msg.body.resize(start + bodySize + sizeof(size_t));

		std::memcpy(msg.body.data() + start, data.body.data(), bodySize);
		std::memcpy(msg.body.data() + start + bodySize, &bodySize, sizeof(size_t));

		return msg;
	}

	friend SerializedObject& operator >> (SerializedObject& msg, SerializedObject& data)
	{
		size_t ende = msg.body.size();
		size_t bodySize;

		std::memcpy((void*)&bodySize, msg.body.data() + ende - sizeof(size_t), sizeof(size_t));

		data.body.resize(bodySize);

		std::memcpy((void*)data.body.data(), msg.body.data() + ende - sizeof(size_t) - bodySize, bodySize);

		msg.body.resize(ende - sizeof(size_t) - bodySize);

		return msg;
	}

	friend SerializedObject& operator << (SerializedObject& msg, std::string& data)
	{
		int start = msg.body.size();
		int nameSize = data.size() * sizeof(char);

		msg.body.resize(start + nameSize + sizeof(int));

		std::memcpy(msg.body.data() + start, data.data(), nameSize);
		std::memcpy(msg.body.data() + start + nameSize, &nameSize, sizeof(int));

		return msg;
	}

	friend SerializedObject& operator >> (SerializedObject& msg, std::string& data)
	{
		int ende = msg.body.size();
		int nameSize;

		std::memcpy((void*)&nameSize, msg.body.data() + ende - sizeof(int), sizeof(int));

		data.resize(nameSize);

		std::memcpy((void*)data.data(), msg.body.data() + ende - sizeof(int) - nameSize, nameSize);

		msg.body.resize(ende - sizeof(int) - nameSize);

		return msg;
	}
	
	friend SerializedObject& operator << (SerializedObject& msg, const char* data)
	{
		int start = msg.body.size();
		int nameSize = strlen(data);

		msg.body.resize(start + nameSize + sizeof(int));

		std::memcpy(msg.body.data() + start, data, nameSize);
		std::memcpy(msg.body.data() + start + nameSize, &nameSize, sizeof(int));

		return msg;
	}

	/*friend SerializedObject& operator >> (SerializedObject& msg, const char* data)
	{
		size_t ende = msg.body.size();
		size_t nameSize;

		std::memcpy((void*)&nameSize, msg.body.data() + ende - sizeof(size_t), sizeof(size_t));

		data.resize(nameSize);

		std::memcpy((void*)data.data(), msg.body.data() + ende - sizeof(size_t) - nameSize, nameSize);

		msg.body.resize(ende - sizeof(size_t) - nameSize);

		return msg;
	}*/
	
	template<typename DataType>
	friend SerializedObject& operator << (SerializedObject& msg, const DataType& data)
	{
		static_assert(std::is_standard_layout<DataType>::value, "Dataformat is not suported" );

		size_t i = msg.body.size();

		msg.body.resize(i + sizeof(DataType));

		std::memcpy(msg.body.data() + i, &data, sizeof(DataType));

		return msg;
	}

	template<typename DataType>
	friend SerializedObject& operator >> (SerializedObject& msg, DataType& data)
	{
		static_assert(std::is_standard_layout<DataType>::value, "Dataformat is not suported");

		size_t i = msg.body.size() - sizeof(DataType);

		std::memcpy(&data, msg.body.data() + i, sizeof(DataType));

		msg.body.resize(i);

		return msg;
	}
};

#endif