#include <pthread.h>

#ifndef TS_QUEUE_HPP
#define TS_QUEUE_HPP

#define DEFAULT_BUFFER_SIZE 200

template <class T>
class TSQueue
{
public:
	// constructor
	TSQueue();
	explicit TSQueue(int max_buffer_size); // can decided the buffer for TSQueue
	// destructor
	~TSQueue();

	// add an element to the end of the queue
	void enqueue(T item);
	// remove and return the first element of the queue
	T dequeue();
	// return the number of elements in the queue
	int get_size();
	int get_buffer_size();

private:
	// the maximum buffer size
	int buffer_size;
	// the buffer containing values of the queue
	T *buffer;
	// the current size of the buffer
	int size;
	// the index of first item in the queue
	int head;
	// the index of last item in the queue
	int tail;

	// pthread mutex lock
	pthread_mutex_t mutex;
	// pthread conditional variable
	pthread_cond_t cond_enqueue, cond_dequeue;
};

// Implementation start

template <class T>
TSQueue<T>::TSQueue() : TSQueue(DEFAULT_BUFFER_SIZE)
{
}

template <class T>
TSQueue<T>::TSQueue(int buffer_size) : buffer_size(buffer_size)
{
	// TODO: implements TSQueue constructor
	// initialize mutex
	pthread_mutex_init(&mutex, NULL);

	pthread_mutex_lock(&mutex); // For protect init: enter critical section
	/*******************critical section*********************/
	// initialize members
	buffer = new T[buffer_size];
	size = 0;
	head = 0;
	tail = -1;

	// initialize conditional variables
	pthread_cond_init(&cond_enqueue, NULL);
	pthread_cond_init(&cond_dequeue, NULL);
	/*******************critical section*********************/
	pthread_mutex_unlock(&mutex); // leave critical section
}

template <class T>
TSQueue<T>::~TSQueue()
{
	// TODO: implenents TSQueue destructor
	pthread_mutex_lock(&mutex); // For protect destory: enter critical section
	/*******************critical section*********************/
	// free members
	delete buffer;

	// destroy condition variables
	pthread_cond_destroy(&cond_enqueue);
	pthread_cond_destroy(&cond_dequeue);
	/*******************critical section*********************/
	pthread_mutex_unlock(&mutex);

	// delete the mutex
	pthread_mutex_destroy(&mutex);
}

template <class T>
void TSQueue<T>::enqueue(T item)
{
	// TODO: enqueues an element to the end of the queue
	pthread_mutex_lock(&mutex); // To protect queue: enter critical section
	/*******************critical section*********************/
	/* check for the free place in queue:
			let cond_enqueue be the condition variable to lock TSQueue::enqueue */
	while (size == buffer_size)
	{
		// if no => let it wait
		pthread_cond_wait(&cond_enqueue, &mutex);
	}

	// if there still has place for consumer => put it into queue
	tail = (tail + 1) % buffer_size;
	buffer[tail] = item;
	size++;

	/* After we done a enqueue, queue have at least one element: notify dequeue*/
	pthread_cond_signal(&cond_dequeue);
	/*******************critical section*********************/

	pthread_mutex_unlock(&mutex); // leave critical section
}

template <class T>
T TSQueue<T>::dequeue()
{
	// TODO: dequeues the first element of the queue
	pthread_mutex_lock(&mutex); // To protect queue: enter critical section
	/*******************critical section*********************/
	/* check if the queue already has item:
			let cond_enqueue be the condition variable to lock TSQueue::dequeue	*/
	while (size == 0)
	{
		// if no => let it wait
		pthread_cond_wait(&cond_dequeue, &mutex);
	}

	// if there exist at least one item in the queue=> do dequeue
	T val = buffer[head];
	head = (head + 1) % buffer_size;
	size--;

	/* After we done a dequeue, queue have at least one empty place: notify enqueue*/
	pthread_cond_signal(&cond_enqueue);
	/*******************critical section*********************/
	pthread_mutex_unlock(&mutex); // leave critical section
	return val;
}

template <class T>
int TSQueue<T>::get_size()
{
	// TODO: returns the size of the queue
	// just return the val, no need to get into critical section
	return size;
}

template <class T>
int TSQueue<T>::get_buffer_size()
{
	// just return the val, no need to get into critical section
	return buffer_size;
}

#endif // TS_QUEUE_HPP
