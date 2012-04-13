/*
 * SQLiteBundleStorage.h
 *
 *  Created on: 09.01.2010
 *      Author: Myrtus
 */


#ifndef SQLITEBUNDLESTORAGE_H_
#define SQLITEBUNDLESTORAGE_H_

#include "storage/BundleStorage.h"
#include "storage/SQLiteDatabase.h"

#include "Component.h"
#include "core/EventReceiver.h"
#include "core/EventReceiver.h"
#include <ibrdtn/data/MetaBundle.h>

#include <ibrcommon/thread/Thread.h>
#include <ibrcommon/thread/RWMutex.h>
#include <ibrcommon/thread/Conditional.h>
#include <ibrcommon/data/File.h>
#include <ibrcommon/thread/Queue.h>

#include <string>
#include <list>
#include <set>

namespace dtn
{
	namespace storage
	{
		class SQLiteBundleStorage: public BundleStorage, public dtn::core::EventReceiver, public dtn::daemon::IndependentComponent, public ibrcommon::BLOB::Provider
		{
		public:
			/**
			 * create a new BLOB object within this storage
			 * @return
			 */
			ibrcommon::BLOB::Reference create();

			/**
			 * Constructor
			 * @param Pfad zum Ordner in denen die Datein gespeichert werden.
			 * @param Dateiname der Datenbank
			 * @param maximale Größe der Datenbank
			 */
			SQLiteBundleStorage(const ibrcommon::File &path, const size_t &size);

			/**
			 * destructor
			 */
			virtual ~SQLiteBundleStorage();

			/**
			 * Stores a bundle in the storage.
			 * @param bundle The bundle to store.
			 */
			void store(const dtn::data::Bundle &bundle);

			/**
			 * This method returns a specific bundle which is identified by
			 * its id.
			 * @param id The ID of the bundle to return.
			 * @return A bundle object.
			 */
			dtn::data::Bundle get(const dtn::data::BundleID &id);

			/**
			 * @see BundleStorage::get(BundleFilterCallback &cb)
			 */
			const std::list<dtn::data::MetaBundle> get(BundleFilterCallback &cb);

			/**
			 * @see BundleStorage::getDistinctDestinations()
			 */
			virtual const std::set<dtn::data::EID> getDistinctDestinations();

			/**
			 * This method deletes a specific bundle in the storage.
			 * No reports will be generated here.
			 * @param id The ID of the bundle to remove.
			 */
			void remove(const dtn::data::BundleID &id);

			/**
			 * Clears all bundles and fragments in the storage. Routinginformation won't be deleted.
			 */
			void clear();

			/**
			 * Clears the hole database.
			 */
			void clearAll();

			/**
			 * @return True, if no bundles in the storage.
			 */
			bool empty();

			/**
			 * @return the count of bundles in the storage
			 */
			unsigned int count();

			/**
			 * @sa BundleStorage::releaseCustody();
			 */
			void releaseCustody(const dtn::data::EID &custodian, const dtn::data::BundleID &id);

			/**
			 * This method is used to receive events.
			 * @param evt
			 */
			void raiseEvent(const dtn::core::Event *evt);

		protected:
			virtual void componentRun();
			virtual void componentUp();
			virtual void componentDown();
			void __cancellation();

		private:
//			enum Position
//			{
//				FIRST_FRAGMENT 	= 0,
//				LAST_FRAGMENT 	= 1,
//				BOTH_FRAGMENTS 	= 2
//			};

			class Task
			{
			public:
				virtual ~Task() {};
				virtual void run(SQLiteBundleStorage &storage) = 0;
			};

			class BlockingTask : public Task
			{
			public:
				BlockingTask() : _done(false), _abort(false) {};
				virtual ~BlockingTask() {};
				virtual void run(SQLiteBundleStorage &storage) = 0;

				/**
				 * wait until this job is done
				 */
				void wait()
				{
					ibrcommon::MutexLock l(_cond);
					while (!_done && !_abort) _cond.wait();

					if (_abort) throw ibrcommon::Exception("Task aborted");
				}

				void abort()
				{
					ibrcommon::MutexLock l(_cond);
					_abort = true;
					_cond.signal(true);
				}

				void done()
				{
					ibrcommon::MutexLock l(_cond);
					_done = true;
					_cond.signal(true);
				}

			private:
				ibrcommon::Conditional _cond;
				bool _done;
				bool _abort;
			};

			class TaskRemove : public Task
			{
			public:
				TaskRemove(const dtn::data::BundleID &id)
				 : _id(id) { };

				virtual ~TaskRemove() {};
				virtual void run(SQLiteBundleStorage &storage);

			private:
				const dtn::data::BundleID _id;
			};

			class TaskIdle : public Task
			{
			public:
				TaskIdle() { };

				virtual ~TaskIdle() {};
				virtual void run(SQLiteBundleStorage &storage);

				static ibrcommon::Mutex _mutex;
				static bool _idle;
			};

			class TaskExpire : public Task
			{
			public:
				TaskExpire(size_t timestamp)
				: _timestamp(timestamp) { };

				virtual ~TaskExpire() {};
				virtual void run(SQLiteBundleStorage &storage);

			private:
				size_t _timestamp;
			};

			/**
			 * A SQLiteBLOB is container for large amount of data. Stored in the database
			 * working directory.
			 */
			class SQLiteBLOB : public ibrcommon::BLOB
			{
				friend class SQLiteBundleStorage;
			public:
				virtual ~SQLiteBLOB();

				virtual void clear();

				virtual void open();
				virtual void close();

			protected:
				std::iostream &__get_stream()
				{
					return _filestream;
				}

				size_t __get_size();

			private:
				SQLiteBLOB(const ibrcommon::File &path);
				std::fstream _filestream;
				ibrcommon::File _file;
				ibrcommon::File _blobPath;
			};


//			/**
//			 *  This Funktion gets e list and a bundle. Every block of the bundle except the PrimaryBlock is saved in a File.
//			 *  The filenames of the blocks are stored in the List. The order of the filenames matches the order of the blocks.
//			 *  @param An empty Stringlist
//			 *  @param A Bundle which should be prepared to be Stored.
//			 *  @return A number bigges than zero is returned indicating an error. Zero is returned if no error occurred.
//			 */
//			int prepareBundle(list<std::string> &filenames, dtn::data::Bundle &bundle);


			/**
			 * @see Component::getName()
			 */
			virtual const std::string getName() const;

			SQLiteDatabase _database;

			ibrcommon::File _blobPath;
			ibrcommon::File _blockPath;

			// contains all jobs to do
			ibrcommon::Queue<Task*> _tasks;

			ibrcommon::RWMutex _global_lock;

//			ibrcommon::AccessLockContext _al_context;
		};
	}
}

#endif /* SQLITEBUNDLESTORAGE_H_ */