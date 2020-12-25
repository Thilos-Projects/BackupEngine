#include "Inventar.h"
#include "Map.h"

int main() {

	Inventar* i = Inventar::getInstance();
	Map* m = Map::getInstance();
	m->setMaxSize(5, 5);
	std::vector<int> items = std::vector<int>();
	items.push_back(0);
	m->setTile(2, 2, items, 0);
	m->setTile(2, 3, items, 0);
	std::vector<Map::place> allTiles = m->getTiles();
	std::vector<Map::street> allStreets = m->getStreets();
	std::vector<Map::village> allVillages = m->getVillages();
	std::cout << allTiles.size() << ' ' << allStreets.size() << ' ' << allVillages.size() << std::endl;
	return 0;
}