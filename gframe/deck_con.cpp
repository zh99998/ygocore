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
			case BUTTON_CLEAR_LOG: {
				break;
			}
			}
			break;
		}
		}
		break;
	}
	}
	return false;
}

}
