/*
 * MoveQueue.cpp
 *
 *  Created on: Feb 8, 2024
 *      Author: d66317d
 */

#include "MoveQueue.h"




bool MoveQueue::isEmpty() {
		return count() == 0;
	}


	unsigned int MoveQueue::count() {
		return (wrapped ? size : 0) + wrEntry - rdEntry;
	}

	bool MoveQueue::isFull() {
		return count() == size;
	}

	void MoveQueue::push(int16_t item) {
		if (!isFull()) {
			q[wrEntry++] = item;
			if (wrEntry == size)
				wrapped = true;
			wrEntry &= (size - 1);
		} else {
			pushErrors++;
		}
	}

	// Pop the top element.
	//  If we are empty, just return whatever is there.
	//  Consumer must check full/empty status.
	int16_t MoveQueue::pop() {
		int16_t val = q[rdEntry];

		if (!isEmpty()) {
			if (count() > 0) {
				// bump and wrap
				rdEntry++;
				if (rdEntry == size)
					wrapped = false;
				rdEntry &= (size - 1);
			}
		} else {
			popErrors++;
		}
		return val;
	}

	// Look at the head element
	int16_t MoveQueue::peek() {
		int16_t val = q[rdEntry];
		return val;
	}

	uint8_t MoveQueue::errCount() {
		return popErrors + pushErrors;
	}
