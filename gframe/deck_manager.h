#ifndef DECKMANAGER_H
#define DECKMANAGER_H

#include "config.h"
#include <unordered_map>
#include <vector>

namespace ygo {

struct LFList {
	wchar_t listName[20];
	std::unordered_map<int, int>* content;
};
struct Deck {
	int main[60];
	int maincount;
	int extra[15];
	int extracount;
	int side[15];
	int sidecount;
};

class DeckManager {
public:
	Deck deckhost;
	Deck deckclient;
	std::vector<LFList> _lfList;

	void LoadLFList();
	bool CheckLFList(Deck& deck, int lfindex);
	void LoadDeck(Deck& deck, int* dbuf, int mainc, int sidec);
	bool LoadDeck(const wchar_t* file);
};

}

#endif //DECKMANAGER_H
