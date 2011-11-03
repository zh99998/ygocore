#ifndef DECK_CON_H
#define DECK_CON_H

#include "config.h"
#include <unordered_map>
#include <vector>
#include "client_card.h"

namespace ygo {

class DeckBuilder: public irr::IEventReceiver {
public:
	virtual bool OnEvent(const irr::SEvent& event);
	
	irr::IrrlichtDevice* device;
	wchar_t deckname[64];
	std::unordered_map<int, int>* filterList;
	std::vector<code_pointer> results;
};

}

#endif //DECK_CON
