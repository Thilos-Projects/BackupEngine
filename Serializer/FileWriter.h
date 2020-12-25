#ifndef _FILEWRITER_H_
#define _FILEWRITER_H_

#include <vector>
#include <stdint.h>
#include <istream>
#include <ostream>
#include <fstream>
#include <iostream>

#include "Serializer.h"

void createFileWith(const char* path, std::vector<uint8_t>& content) {
	std::ofstream myFile(path, std::ios::out | std::ios::binary);

	if (!myFile.is_open())
		return;

	for (int i = 0; i < content.size(); i++)
			myFile.put(content[i]);

	myFile.close();
}
std::vector<uint8_t> readDataFromFile(const char* path) {
	std::ifstream myFile(path, std::ios::in | std::ios::binary);

	std::vector<uint8_t> toRet = std::vector<uint8_t>();

	if (!myFile.is_open())
		return toRet;

	char temp;
	myFile.get(temp);
	while (myFile.good()) {
		toRet.push_back(temp);
		myFile.get(temp);
	}

	myFile.close();

	return toRet;
}

void deleteFile(const char* path) {
	std::remove(path);
}

void writeToFile(SerializedObject& so, const char* path) {
	std::ofstream myFile(path, std::ios::out | std::ios::binary);

	if (!myFile.is_open())
		return;

	for (int i = 0; i < so.body.size(); i++)
		myFile.put(so.body[i]);

	myFile.close();
};

SerializedObject readSerializedObjectFromFile(const char* path) {
	std::ifstream myFile(path, std::ios::in | std::ios::binary);

	SerializedObject so;

	if (!myFile.is_open())
		return so;

	char temp;
	while (myFile.get(temp))
		so.body.push_back(temp);

	myFile.close();

	return so;
};

#endif