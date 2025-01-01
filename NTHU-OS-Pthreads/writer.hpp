#include <fstream>
#include "thread.hpp"
#include "ts_queue.hpp"
#include "item.hpp"

#ifndef WRITER_HPP
#define WRITER_HPP

class Writer : public Thread
{
public:
	// constructor
	Writer(int expected_lines, std::string output_file, TSQueue<Item *> *output_queue);

	// destructor
	~Writer();

	virtual void start() override;

private:
	// the expected lines to write,
	// the writer thread finished after output expected lines of item
	int expected_lines;

	std::ofstream ofs;
	TSQueue<Item *> *output_queue;

	// the method for pthread to create a writer thread
	static void *process(void *arg);
};

// Implementation start

Writer::Writer(int expected_lines, std::string output_file, TSQueue<Item *> *output_queue)
		: expected_lines(expected_lines), output_queue(output_queue)
{
	ofs = std::ofstream(output_file);
}

Writer::~Writer()
{
	ofs.close();
}

void Writer::start()
{
	// TODO: starts a Writer thread
	// Create a new thread that runs the Writer::process method
	pthread_create(&this->t, 0, Writer::process, (void *)this);
}

// Static method: implements the logic executed by the writer thread
void *Writer::process(void *arg)
{
	// TODO: implements the Writer's work
	// Cast the argument to a Writer object
	Writer *writer = (Writer *)arg;

	// Loop until the expected number of lines is written
	while (writer->expected_lines--)
	{
		// Dequeue an item from the output queue (blocking operation if queue is empty)
		Item *item = writer->output_queue->dequeue();
		// Write the item's content to the output file using the ofstream object
		writer->ofs << *item;
	}

	// Exit the thread
	return nullptr;
}

#endif // WRITER_HPP
