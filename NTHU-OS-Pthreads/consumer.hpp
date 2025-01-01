#include <pthread.h>
#include <stdio.h>
#include "thread.hpp"
#include "ts_queue.hpp"
#include "item.hpp"
#include "transformer.hpp"

#ifndef CONSUMER_HPP
#define CONSUMER_HPP

class Consumer : public Thread
{
public:
	// constructor
	Consumer(TSQueue<Item *> *worker_queue, TSQueue<Item *> *output_queue, Transformer *transformer);

	// destructor
	~Consumer();

	virtual void start() override;

	virtual int cancel() override;

private:
	TSQueue<Item *> *worker_queue;
	TSQueue<Item *> *output_queue;

	Transformer *transformer;

	bool is_cancel;

	// the method for pthread to create a consumer thread
	static void *process(void *arg);
};

Consumer::Consumer(TSQueue<Item *> *worker_queue, TSQueue<Item *> *output_queue, Transformer *transformer)
		: worker_queue(worker_queue), output_queue(output_queue), transformer(transformer)
{
	is_cancel = false;
}

Consumer::~Consumer() {}

void Consumer::start()
{
	// TODO: starts a Consumer thread
	// Creates a new thread and starts executing the process method
	pthread_create(&this->t, 0, Consumer::process, this);
}

int Consumer::cancel()
{
	// TODO: cancels the consumer thread
	// Sets the cancellation flag to true: notify "static Consumer::proceee" to end the infinity loop and delete Consumer
	is_cancel = true;
	return pthread_cancel(this->t);
}

// very same as static Producer::process
void *Consumer::process(void *arg)
{
	Consumer *consumer = (Consumer *)arg;
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, nullptr);
	while (!consumer->is_cancel)
	{
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);

		// TODO: implements the Consumer's work
		// take the item form worker_queue
		if (consumer->worker_queue->get_size() > 0)
		{
			// the same as producer::process, dequeue an item form queue
			Item *transform_item = consumer->worker_queue->dequeue();
			// new item: use "Transformer::consumer_transform" transfer data
			unsigned long long int val = consumer->transformer->consumer_transform(transform_item->opcode, transform_item->val);
			Item *new_item = new Item(transform_item->key, val, transform_item->opcode);
			// put the new item into "output_queue"
			consumer->output_queue->enqueue(new_item);

			// delete the original item
			delete transform_item;
		}
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);
	}
	delete consumer;
	return nullptr;
}

#endif // CONSUMER_HPP
