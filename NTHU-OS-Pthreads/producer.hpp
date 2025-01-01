#include <pthread.h>
#include "thread.hpp"
#include "ts_queue.hpp"
#include "item.hpp"
#include "transformer.hpp"

#ifndef PRODUCER_HPP
#define PRODUCER_HPP

class Producer : public Thread
{
public:
	// constructor
	Producer(TSQueue<Item *> *input_queue, TSQueue<Item *> *worker_queue, Transformer *transfomrer);

	// destructor
	~Producer();

	virtual void start();

private:
	TSQueue<Item *> *input_queue;
	TSQueue<Item *> *worker_queue;

	Transformer *transformer;

	// the method for pthread to create a producer thread
	static void *process(void *arg);
};

Producer::Producer(TSQueue<Item *> *input_queue, TSQueue<Item *> *worker_queue, Transformer *transformer)
		: input_queue(input_queue), worker_queue(worker_queue), transformer(transformer)
{
}

Producer::~Producer() {}

void Producer::start()
{
	// TODO: starts a Producer thread
	// Creates a new thread and starts executing the process method
	pthread_create(&this->t, 0, Producer::process, (void *)this);
}

void *Producer::process(void *arg)
{
	// TODO: implements the Producer's work
	// Casts the argument to a Producer object
	Producer *producer = (Producer *)arg;

	while (1) // Infinite loop that runs the producer thread
	{
		// Check if there are items in the input queue
		if (producer->input_queue->get_size() > 0)
		{
			// Dequeues an item from the input queue for processing(need to be deleted)
			Item *transform_item = producer->input_queue->dequeue();
			// Uses the transformer to process the item
			unsigned long long int val = producer->transformer->producer_transform(transform_item->opcode, transform_item->val);
			Item *new_item = new Item(transform_item->key, val, transform_item->opcode); // new Item
			// Enqueues the new item into the worker queue for further processing
			producer->worker_queue->enqueue(new_item);
			//! important for heap memory management
			// Deletes the original item as it's no longer needed
			delete transform_item;
		}
	}
	// Returns null when the thread finishes
	return nullptr;
}

#endif // PRODUCER_HPP
