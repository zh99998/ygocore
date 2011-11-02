#ifndef DECK_CON_H
#define DECK_CON_H

#include "config.h"
#include <set>

namespace ygo {

class DeckBuilder: public irr::IEventReceiver {
public:
	virtual bool OnEvent(const irr::SEvent& event);
	
	irr::IrrlichtDevice* device;
	wchar_t deckname[64];
};

}

#endif //DECK_CON
