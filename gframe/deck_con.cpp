#include "config.h"
#include "deck_con.h"
#include "game.h"

extern ygo::Game* mainGame;

namespace ygo {

bool DeckBuilder::OnEvent(const irr::SEvent& event) {
	switch(event.EventType) {
	case irr::EET_GUI_EVENT: {
		s32 id = event.GUIEvent.Caller->getID();
		irr::gui::IGUIEnvironment* env = device->getGUIEnvironment();
		switch(event.GUIEvent.EventType) {
		case irr::gui::EGET_BUTTON_CLICKED: {
			switch(id) {
			case BUTTON_CLEAR_DECK: {
				mainGame->deckManager.deckhost.maincount = 0;
				mainGame->deckManager.deckhost.extracount = 0;
				mainGame->deckManager.deckhost.sidecount = 0;
				break;
			}
			case BUTTON_DBEXIT: {
				mainGame->is_building = false;
				mainGame->wDeckEdit->setVisible(false);
				mainGame->wCategories->setVisible(false);
				mainGame->wFilter->setVisible(false);
				mainGame->wCardImg->setVisible(false);
				mainGame->wInfos->setVisible(false);
				mainGame->PopupElement(mainGame->wModeSelection);
				mainGame->device->setEventReceiver(&mainGame->dField);
				mainGame->imageManager.ClearImage();
				break;
			}
			case BUTTON_EFFECT_FILTER: {
				mainGame->PopupElement(mainGame->wCategories);
				break;
			}
			case BUTTON_CATEGORY_OK: {
				mainGame->HideElement(mainGame->wCategories);
				break;
			}
			}
			break;
		}
		case irr::gui::EGET_COMBO_BOX_CHANGED: {
			switch(id) {
			case COMBOBOX_DBLFLIST: {
				filterList = mainGame->deckManager._lfList[mainGame->cbDBLFList->getSelected()].content;
				break;
			}
			case COMBOBOX_DBDECKS: {
				mainGame->deckManager.LoadDeck(mainGame->cbDBDecks->getItem(mainGame->cbDBDecks->getSelected()));
				break;
			}
			case COMBOBOX_MAINTYPE: {
				switch(mainGame->cbCardType->getSelected()) {
				case 0: {
					mainGame->cbCardType2->setEnabled(false);
					mainGame->cbRace->setEnabled(false);
					mainGame->cbAttribute->setEnabled(false);
					mainGame->ebAttack->setEnabled(false);
					mainGame->ebDefence->setEnabled(false);
					mainGame->ebStar->setEnabled(false);
					break;
				}
				case 1: {
					mainGame->cbCardType2->setEnabled(true);
					mainGame->cbRace->setEnabled(true);
					mainGame->cbAttribute->setEnabled(true);
					mainGame->ebAttack->setEnabled(true);
					mainGame->ebDefence->setEnabled(true);
					mainGame->ebStar->setEnabled(true);
					mainGame->cbCardType2->clear();
					mainGame->cbCardType2->addItem(L"通常", 0x11);
					mainGame->cbCardType2->addItem(L"效果", 0x21);
					mainGame->cbCardType2->addItem(L"融合", 0x41);
					mainGame->cbCardType2->addItem(L"仪式", 0x81);
					mainGame->cbCardType2->addItem(L"同调", 0x2000);
					mainGame->cbCardType2->addItem(L"未知", 0x800000);
					break;
				}
				case 2: {
					mainGame->cbCardType2->setEnabled(true);
					mainGame->cbRace->setEnabled(false);
					mainGame->cbAttribute->setEnabled(false);
					mainGame->ebAttack->setEnabled(false);
					mainGame->ebDefence->setEnabled(false);
					mainGame->ebStar->setEnabled(false);
					mainGame->cbCardType2->clear();
					mainGame->cbCardType2->addItem(L"通常", 0x2);
					mainGame->cbCardType2->addItem(L"速攻", 0x10002);
					mainGame->cbCardType2->addItem(L"永续", 0x20002);
					mainGame->cbCardType2->addItem(L"仪式", 0x82);
					mainGame->cbCardType2->addItem(L"装备", 0x40002);
					mainGame->cbCardType2->addItem(L"场地", 0x80002);
					break;
				}
				case 3: {
					mainGame->cbCardType2->setEnabled(true);
					mainGame->cbRace->setEnabled(false);
					mainGame->cbAttribute->setEnabled(false);
					mainGame->ebAttack->setEnabled(false);
					mainGame->ebDefence->setEnabled(false);
					mainGame->ebStar->setEnabled(false);
					mainGame->cbCardType2->clear();
					mainGame->cbCardType2->addItem(L"通常", 0x4);
					mainGame->cbCardType2->addItem(L"永续", 0x20004);
					mainGame->cbCardType2->addItem(L"反击", 0x100004);
					break;
				}
				}
			}
			}
		}
		}
		break;
	}
	}
	return false;
}

}
