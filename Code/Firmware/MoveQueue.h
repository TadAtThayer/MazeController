/*
 * MoveQueue.h
 *
 *  Created on: Feb 8, 2024
 *      Author: Tad Truex
 */

#ifndef MOVEQUEUE_H_
#define MOVEQUEUE_H_

#include <stdint.h>

class MoveQueue {
public:
	static const unsigned int size = 128;
private:
	int16_t q[MoveQueue::size];
	unsigned int wrEntry = 0;
	unsigned int rdEntry = 0;
	uint8_t pushErrors = 0;
	uint8_t popErrors = 0;
	bool wrapped = false;

public:
	bool isEmpty();
	unsigned int count();
	bool isFull();
	void push(int16_t item);

	// Pop the top element.
	//  If we are empty, just return whatever is there.
	//  Consumer must check full/empty status.
	int16_t pop();

	// Look at the head element
	int16_t peek();
	uint8_t errCount();

};


#endif /* MOVEQUEUE_H_ */
