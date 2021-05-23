#include "rwlock.h"

// initializing the read-write lock
void InitalizeReadWriteLock(struct read_write_lock * rw){
	rw->readers = 0;
	rw->writers = 0;
	
	sem_init(&rw->lock, 0, 1);
	sem_init(&rw->writelock, 0, 1);
	sem_init(&rw->writer_count_lock, 0, 1);
	sem_init(&rw->write_waiting_lock, 0, 1);
}

// aquiring read-write lock by the reader
void ReaderLock(struct read_write_lock * rw){
	// will keep waiting agar koi writer pehle ki wait kr rha hua to
	sem_wait(&rw->write_waiting_lock);
	
	sem_wait(&rw->lock);
	rw->readers++;
	if(rw->readers == 1)
		sem_wait(&rw->writelock);
	sem_post(&rw->lock);

	sem_post(&rw->write_waiting_lock);
}

// releasing read-write lock by the reader
void ReaderUnlock(struct read_write_lock * rw){
	sem_wait(&rw->lock);
	rw->readers--;
	if(rw->readers == 0)
		sem_post(&rw->writelock);
	sem_post(&rw->lock);
}

// aquiring read-write lock by the writer
void WriterLock(struct read_write_lock * rw){
	sem_wait(&rw->writer_count_lock);
	rw->writers++;
	if(rw->writers == 1)
		sem_wait(&rw->write_waiting_lock);
	sem_post(&rw->writer_count_lock);
	
	sem_wait(&rw->writelock);
	
	sem_wait(&rw->writer_count_lock);
	rw->writers--;
	if(rw->writers == 0)
		sem_post(&rw->write_waiting_lock);
	sem_post(&rw->writer_count_lock);
}

// releasing read-write lock by the writer
void WriterUnlock(struct read_write_lock * rw){
	sem_post(&rw->writelock);
}
