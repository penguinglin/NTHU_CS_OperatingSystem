#include <assert.h>
#include <stdlib.h>
#include "ts_queue.hpp"
#include "item.hpp"
#include "reader.hpp"
#include "writer.hpp"
#include "producer.hpp"
#include "consumer_controller.hpp"
#include <chrono> // for timing

#define READER_QUEUE_SIZE 200
#define WORKER_QUEUE_SIZE 200
#define WRITER_QUEUE_SIZE 4000
#define CONSUMER_CONTROLLER_LOW_THRESHOLD_PERCENTAGE 20
#define CONSUMER_CONTROLLER_HIGH_THRESHOLD_PERCENTAGE 80
#define CONSUMER_CONTROLLER_CHECK_PERIOD 1000000

int main(int argc, char **argv)
{
	assert(argc == 4);

	int n = atoi(argv[1]);
	std::string input_file_name(argv[2]);
	std::string output_file_name(argv[3]);

	// TODO: implements main function
	TSQueue<Item *> *input_queue = new TSQueue<Item *>(READER_QUEUE_SIZE);
	TSQueue<Item *> *woker_queue = new TSQueue<Item *>(WORKER_QUEUE_SIZE);
	TSQueue<Item *> *output_queue = new TSQueue<Item *>(WRITER_QUEUE_SIZE);

	// Start the threads for reading, writing, producing, and controlling consumers
	Transformer *transformer = new Transformer();
	Reader *reader = new Reader(n, input_file_name, input_queue);
	Writer *writer = new Writer(n, output_file_name, output_queue);
	Producer *p1 = new Producer(input_queue, woker_queue, transformer);
	Producer *p2 = new Producer(input_queue, woker_queue, transformer);
	Producer *p3 = new Producer(input_queue, woker_queue, transformer);
	Producer *p4 = new Producer(input_queue, woker_queue, transformer);
	ConsumerController *controller = new ConsumerController(
			woker_queue, output_queue, transformer,
			CONSUMER_CONTROLLER_CHECK_PERIOD,
			CONSUMER_CONTROLLER_LOW_THRESHOLD_PERCENTAGE,
			CONSUMER_CONTROLLER_HIGH_THRESHOLD_PERCENTAGE);

	// Start all the threads

	// 記錄開始時間
	// auto start_time = std::chrono::high_resolution_clock::now();
	reader->start();
	writer->start();
	controller->start();
	p1->start();
	p2->start();
	p3->start();
	p4->start();

	// Wait for the reader and writer threads to finish (join them to the main thread)
	reader->join();
	writer->join();

	// Once reading and writing are complete, clean up dynamically allocated memory
	// 記錄結束時間
	auto end_time = std::chrono::high_resolution_clock::now();
	delete p1;
	delete p2;
	delete p3;
	delete p4;
	delete controller;
	delete reader;
	delete writer;
	delete transformer;
	delete input_queue;
	delete woker_queue;
	delete output_queue;

	// 計算並輸出執行時間
	// auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
	// std::cout << "Total execution time: " << duration.count() << " ms" << std::endl;

	// End the program
	return 0;
}
