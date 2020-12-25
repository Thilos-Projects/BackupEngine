#ifndef _MAP_H_
#define _MAP_H_

#include <Serializer_Include.h>
#include <string>

class Map :public baseSingleton<Map>
{
public:
	//playerId is -1 if not used
	struct itemResult {
		int itemId = -1;
		int itemCount = 0;
		int playerID = -1;
	};
	/* tileID wird von map festgelegt*/
	struct tile {
	public:
		bool emptyTile = true;
		int tileId = -1;
		std::vector<int> itemIDs;

		tile();
	};
	struct place {
	public:
		tile* tile = NULL;
		int number = -1;
		int x = -1, y = -1;
		int index = -1;

		void afterSet();
	};
	struct village {
	public:
		bool isInitialized = false;

		bool isCity = false;
		int playerID = -1;
		int x, y = -1;
		int index = -1;
		std::vector<tile*> connectingTiles;

		village();
	};
	struct street {
	public:
		bool isInitialized = false;

		int playerID = -1;
		int x, y;
		int index;
	};

private:
	int maxSizeX, maxSizeY;
	tile emptyTile;
	std::vector<tile> allTiles;
	std::vector<place> allTilePlaces;
	std::vector<village> villages;
	std::vector<street> streets;
public:
	Map() {
		allTiles = std::vector<tile>();
		allTilePlaces = std::vector<place>();
		villages = std::vector<village>();
		streets = std::vector<street>();
		emptyTile = tile();
		emptyTile.emptyTile = true;
		emptyTile.tileId = -1;
	}
	~Map() {

	}
	void setMaxSize(int x, int y) {
		maxSizeX = x;
		maxSizeY = y;
	}

	void setTile(int x, int y, std::vector<int>& result, int number, bool isEmpty = false) {
		tile* currentTile = 0;
		if (isEmpty) {
			currentTile = &emptyTile;
		} else {
			for (int i = 0; i < allTiles.size(); i++)
				if (allTiles[i].itemIDs.size() == result.size()) {
					bool hit = true;
					for (int j = 0; j < result.size(); j++)
						if (allTiles[i].itemIDs[j] != result[j]) {
							hit = false;
							break;
						}
					if (hit) {
						currentTile = &allTiles[i];
						break;
					}
				}
			if (currentTile == NULL) {
				allTiles.push_back(tile());
				currentTile = &allTiles[allTiles.size() - 1];
				currentTile->emptyTile = false;
				currentTile->tileId = allTiles.size() - 1;
				for (int i = 0; i < result.size(); i++)
					currentTile->itemIDs.push_back(result[i]);
			}
		}
		
		place p = place();
		p.tile = currentTile;
		p.number = number;
		p.index = x * maxSizeY + y;
		p.x = x;
		p.y = y;
		if (allTilePlaces.size() <= p.index)
			allTilePlaces.resize(p.index + 1);
		allTilePlaces[p.index] = p;
		p.afterSet();
	}

	std::vector<place> getTiles() {
		std::vector<place> toRet = std::vector<place>();
		for (int i = 0; i < allTilePlaces.size(); i++) {
			if (allTilePlaces[i].tile != NULL)
				toRet.push_back(allTilePlaces[i]);
		}
		return toRet;
	}
	std::vector<village> getVillages() {
		std::vector<village> toRet = std::vector<village>();
		for (int i = 0; i < villages.size(); i++) {
			if (villages[i].isInitialized)
				toRet.push_back(villages[i]);
		}
		return toRet;
	}
	std::vector<street> getStreets() {
		std::vector<street> toRet = std::vector<street>();
		for (int i = 0; i < streets.size(); i++) {
			if (streets[i].isInitialized)
				toRet.push_back(streets[i]);
		}
		return toRet;
	}

	void addVillage(int x, int y, tile* tile) {
		int index = (y * (maxSizeX * 2 + 2)) + x;
		if (villages.size() <= index)
			villages.resize(index + 1);
		if (!villages[index].isInitialized) {
			villages[index] = village();
			villages[index].x = x;
			villages[index].y = y;
			villages[index].index = index;
		}
		villages[index].connectingTiles.push_back(tile);
	}
	void addStreet(int x, int y) {
		int index = (x * (maxSizeY * 2 + 2)) + y;
		if (streets.size() <= index)
			streets.resize(index + 1);
		if (!streets[index].isInitialized) {
			streets[index] = street();
			streets[index].index = index;
			streets[index].x = x;
			streets[index].y = y;
			streets[index].isInitialized = true;
		}
	}
};
Map* Map::instance = 0;

Map::tile::tile() {
	itemIDs = std::vector<int>();
}
void Map::place::afterSet() {
	if (tile->emptyTile)
		return;
	Map* instance = Map::getInstance();
	instance->addVillage(x * 2 + 0, y + 0, tile);
	instance->addVillage(x * 2 + 1, y + 0, tile);
	instance->addVillage(x * 2 + 2, y + 0, tile);
	instance->addVillage(x * 2 + 0, y + 1, tile);
	instance->addVillage(x * 2 + 1, y + 1, tile);
	instance->addVillage(x * 2 + 2, y + 1, tile);

	instance->addStreet(x + 0, y * 2 - 1);
	instance->addStreet(x + 0, y * 2 - 0);
	instance->addStreet(x + 0, y * 2 + 1);
	instance->addStreet(x + 1, y * 2 - 1);
	instance->addStreet(x + 1, y * 2 - 0);
	instance->addStreet(x + 1, y * 2 + 1);
}
Map::village::village() {
	connectingTiles = std::vector<tile*>();
	isInitialized = true;
}
#endif