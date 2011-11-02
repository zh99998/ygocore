/*
 * effectset.h
 *
 *  Created on: 2011-10-8
 *      Author: Argon
 */

#ifndef EFFECTSET_H_
#define EFFECTSET_H_

#include <stdlib.h>
#include "effect.h"

class effect;

int effect_sort_id(const void* e1, const void* e2);

struct effect_set {
	effect_set(): count(0) {}
	void add_item(effect* peffect) {
		if(count >= 32) return;
		container[count++] = peffect;
	}
	void remove_item(int index) {
		if(index >= count)
			return;
		if(index == count - 1) {
			count--;
			return;
		}
		for(int i = index; i < count - 1; ++i)
			container[i] = container[i + 1];
		count--;
	}
	void clear() {
		count = 0;
	}
	void sort() {
		if(count < 2)
			return;
		qsort(container, count, sizeof(effect*), effect_sort_id);
	}
	effect*& get_last() {
		return container[count - 1];
	}
	effect*& operator[] (int index) {
		return container[index];
	}
	effect*& at(int index) {
		return container[index];
	}
	effect* container[32];
	int count;
};

#endif //EFFECTSET_H_
