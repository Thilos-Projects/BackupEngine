#ifndef _INVENTAR_
#define _INVENTAR_

#include <Serializer_Include.h>
#include <string>
#include <vector>

struct item {
public:
	int id;
	std::string name;
	int ValueMarcet;
	int ValueHabour;
};

struct playerInventory {
public:
	int playerID;
	int streetCount;
	int cityCount;
	int villageCount;
	std::vector<std::pair<item, int>> ownedItems;
	std::vector<bool> hasHabour;

	playerInventory();
};

class Inventar : public baseSingleton <Inventar>
{
private:
	std::vector<item> knownItems;
	std::vector<playerInventory> inventorys;

public:

	Inventar() {
		knownItems = std::vector<item>();
		inventorys = std::vector<playerInventory>();
	}

	int getMaxItemCount() {
		return knownItems.size();
	}

	item* getAllItems() {
		return knownItems.data();
	}

	playerInventory* addPlayer() {
		inventorys.push_back(playerInventory());
		inventorys[inventorys.size() - 1].playerID = inventorys.size() - 1;
		return &inventorys[inventorys.size() - 1];
	}

	bool hasPlayerItems(int id, std::vector<std::pair<int,int>>& count) {
		if (id < 0 || id > inventorys.size() - 1)
			return false;
		if (count.size() > inventorys.size() - 1)
			return false;
		for (int i = 0;i < count.size(); i++) {
			if (inventorys[id].ownedItems[count[i].first].second < count[i].second)
				return false;
		}
		return true;
	}
	void removeItemFromPlayer(int id, std::vector < std::pair<int, int>>& count) {
		if (id < 0 || id > inventorys.size() - 1)
			return;
		if (count.size() > inventorys.size() - 1)
			return;
		for (int i = 0; i < count.size(); i++) {
			inventorys[id].ownedItems[count[i].first].second -= count[i].second;
			if (inventorys[id].ownedItems[count[i].first].second < 0)
				inventorys[id].ownedItems[count[i].first].second = 0;
		}
	}

	void addItemtoPlayer(int id, std::vector<std::pair<int, int>>& count) {
		if (id < 0 || id > inventorys.size() - 1)
			return;
		if (count.size() > inventorys.size() - 1)
			return;
		for (int i = 0; i < count.size(); i++)
			inventorys[id].ownedItems[count[i].first].second += count[i].second;
	}
};
Inventar* Inventar::instance = 0;

playerInventory::playerInventory() {
	ownedItems = std::vector<std::pair<item, int>>();
	hasHabour = std::vector<bool>();
	int itemSize = Inventar::getInstance()->getMaxItemCount();
	item* allItems = Inventar::getInstance()->getAllItems();
	for (int i = 0; i < itemSize; i++) {
		ownedItems.push_back(std::pair<item, int>(allItems[i], 0));
		hasHabour.push_back(false);
	}
}

#endif