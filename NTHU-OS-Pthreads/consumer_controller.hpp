#include <pthread.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include "consumer.hpp"
#include "ts_queue.hpp"
#include "item.hpp"
#include "transformer.hpp"

#ifndef CONSUMER_CONTROLLER
#define CONSUMER_CONTROLLER

class ConsumerController : public Thread
{
public:
	// constructor
	ConsumerController(
			TSQueue<Item *> *worker_queue,
			TSQueue<Item *> *writer_queue,
			Transformer *transformer,
			int check_period,
			int low_threshold,
			int high_threshold);

	// destructor
	~ConsumerController();

	virtual void start();

private:
	std::vector<Consumer *> consumers;

	TSQueue<Item *> *worker_queue;
	TSQueue<Item *> *writer_queue;

	Transformer *transformer;

	// Check to scale down or scale up every check period in microseconds.
	int check_period;
	// When the number of items in the worker queue is lower than low_threshold,
	// the number of consumers scaled down by 1.
	int low_threshold;
	// When the number of items in the worker queue is higher than high_threshold,
	// the number of consumers scaled up by 1.
	int high_threshold;

	static void *process(void *arg);
};

// Implementation start

ConsumerController::ConsumerController(
		TSQueue<Item *> *worker_queue,
		TSQueue<Item *> *writer_queue,
		Transformer *transformer,
		int check_period,
		int low_threshold,
		int high_threshold) : worker_queue(worker_queue),
													writer_queue(writer_queue),
													transformer(transformer),
													check_period(check_period),
													low_threshold(low_threshold),
													high_threshold(high_threshold)
{
}

ConsumerController::~ConsumerController() {}

void ConsumerController::start()
{
	// TODO: starts a ConsumerController thread
	// Creates a new thread that runs the process method
	pthread_create(&this->t, 0, ConsumerController::process, this);
}
// The main execution body of the ConsumerController thread
void *ConsumerController::process(void *arg)
{
	// TODO: implements the ConsumerController's work
	// Casts the argument to a ConsumerController object
	ConsumerController *controller = (ConsumerController *)arg;
	while (1) // Infinite loop that keeps checking the worker queue size and scaling consumers up/down
	{
		// Calculates the proportion of items in the worker queue relative to its buffer size
		double worker_proportion = (double)controller->worker_queue->get_size() / controller->worker_queue->get_buffer_size();
		// If the worker queue is more than the high threshold
		if (worker_proportion > (double)controller->high_threshold / 100)
		{
			// Creates a new consumer to handle more items and starts it
			Consumer *new_consumer = new Consumer(controller->worker_queue, controller->writer_queue, controller->transformer);
			new_consumer->start();

			// Adds the new consumer to the consumers vector
			controller->consumers.push_back(new_consumer);
			std::cout << "Scaling up consumers from " << controller->consumers.size() - 1 << " to " << controller->consumers.size() << "\n";
		}
		// If the worker queue is less than the low threshold and there are more than 1 consumer (scale down)
		else if (worker_proportion < (double)controller->low_threshold / 100 && controller->consumers.size() > 1)
		{
			// Removes and cancels the last consumer from the vector
			Consumer *delete_consumer = controller->consumers[controller->consumers.size() - 1];
			delete_consumer->cancel();
			controller->consumers.pop_back();
			std::cout << "Scaling down consumers from " << controller->consumers.size() + 1 << " to " << controller->consumers.size() << "\n";
		}
		// Pauses for the specified check period before checking again
		usleep(controller->check_period);
	}
	// Returns nullptr when the thread finishes
	return nullptr;
}

#endif // CONSUMER_CONTROLLER_HPP
