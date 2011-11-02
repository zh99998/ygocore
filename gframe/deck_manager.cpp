#include "deck_manager.h"
#include "data_manager.h"
#include "game.h"

extern ygo::Game* mainGame;

namespace ygo {

void DeckManager::LoadLFList() {
	LFList cur;
	std::unordered_map<int, int>* curMap = 0;
	FILE* fp = fopen("lflist.conf", "r");
	char linebuf[256];
	wchar_t strBuffer[256];
	if(fp) {
		fseek(fp, 0, SEEK_END);
		size_t fsize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		fgets(linebuf, 256, fp);
		while(ftell(fp) < fsize) {
			fgets(linebuf, 256, fp);
			if(linebuf[0] == '#')
				continue;
			int p = 0, sa = 0, code, count;
			if(linebuf[0] == '!') {
				sa = DataManager::DecodeUTF8((const char*)(&linebuf[1]), strBuffer);
				while(strBuffer[sa - 1] == L'\r' || strBuffer[sa - 1] == L'\n' ) sa--;
				memcpy(cur.listName, (const void*)strBuffer, 40);
				cur.listName[sa] = 0;
				curMap = new std::unordered_map<int, int>;
				cur.content = curMap;
				_lfList.push_back(cur);
				continue;
			}
			while(linebuf[p] != ' ' && linebuf[p] != '\t' && linebuf[p] != 0) p++;
			if(linebuf[p] == 0)
				continue;
			linebuf[p++] = 0;
			sa = p;
			code = atoi(linebuf);
			if(code == 0)
				continue;
			while(linebuf[p] == ' ' || linebuf[p] == '\t') p++;
			while(linebuf[p] != ' ' && linebuf[p] != '\t' && linebuf[p] != 0) p++;
			linebuf[p] = 0;
			count = atoi(&linebuf[sa]);
			(*curMap)[code] = count;
		}
		fclose(fp);
	}
	swprintf(cur.listName, L"无限制");
	cur.content = new std::unordered_map<int, int>;
	_lfList.push_back(cur);
}
bool DeckManager::CheckLFList(Deck& deck, int lfindex) {
	std::unordered_map<int, int> ccount;
	std::unordered_map<int, int>* list = _lfList[lfindex].content;
	int dc = 0, dec = 0;
	if(deck.maincount < 40 || deck.maincount > 60 || deck.extracount > 15 || deck.sidecount > 15)
		return false;
	for(int i = 0; i < deck.maincount; ++i) {
		ccount[deck.main[i]]++;
		dc = ccount[deck.main[i]];
		auto it = list->find(deck.main[i]);
		if(dc > 3 || (it != list->end() && dc > it->second))
			return false;
	}
	for(int i = 0; i < deck.extracount; ++i) {
		ccount[deck.extra[i]]++;
		dc = ccount[deck.extra[i]];
		auto it = list->find(deck.extra[i]);
		if(dc > 3 || (it != list->end() && dc > it->second))
			return false;
	}
	for(int i = 0; i < deck.sidecount; ++i) {
		ccount[deck.side[i]]++;
		dc = ccount[deck.side[i]];
		auto it = list->find(deck.side[i]);
		if(dc > 3 || (it != list->end() && dc > it->second))
			return false;
	}
	return true;
}
void DeckManager::LoadDeck(Deck& deck, int* dbuf, int mainc, int sidec) {
	deck.maincount = 0;
	deck.extracount = 0;
	deck.sidecount = 0;
	int code;
	CardData cd;
	for(int i = 0; i < mainc; ++i) {
		code = dbuf[i];
		if(!mainGame->dataManager.GetData(code, &cd))
			continue;
		if(cd.type & TYPE_TOKEN)
			continue;
		else if(cd.type & 0x802040 && deck.extracount < 15) {
			deck.extra[deck.extracount++] = code;
		} else if(deck.maincount < 60) {
			deck.main[deck.maincount++] = code;
		}
	}
	for(int i = 0; i < sidec; ++i) {
		code = dbuf[mainc + i];
		if(deck.sidecount < 15)
			deck.side[deck.sidecount++] = code;
	}
}
bool DeckManager::LoadDeck(const wchar_t* file) {
	int sp = 0, ct = 0, mainc = 0, sidec = 0, code;
	wchar_t deck[64];
	swprintf(deck, L"./deck/%s.ydk", file);
	int cardlist[128];
	bool is_side = false;
	FILE* fp = _wfopen(deck, L"r");
	if(!fp)
		return false;
	char linebuf[256];
	fseek(fp, 0, SEEK_END);
	size_t fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	fgets(linebuf, 256, fp);
	while(ftell(fp) < fsize && ct < 128) {
		fgets(linebuf, 256, fp);
		if(linebuf[0] == '!') {
			is_side = true;
			continue;
		}
		if(linebuf[0] < '0' || linebuf[0] > '9')
			continue;
		sp = 0;
		while(linebuf[sp] >= '0' && linebuf[sp] <= '9') sp++;
		linebuf[sp] = 0;
		code = atoi(linebuf);
		cardlist[ct++] = code;
		if(is_side) sidec++;
		else mainc++;
	}
	fclose(fp);
	LoadDeck(deckhost, cardlist, mainc, sidec);
	return true;
}

}
