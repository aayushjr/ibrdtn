/*
 * StaticRoutingExension.h
 *
 *  Created on: 15.02.2010
 *      Author: morgenro
 */

#ifndef STATICROUTINGEXTENSION_H_
#define STATICROUTINGEXTENSION_H_

#include "routing/BaseRouter.h"
#include <ibrdtn/data/MetaBundle.h>
#include <ibrcommon/thread/Queue.h>
#include <ibrcommon/thread/Mutex.h>
#include <regex.h>

namespace dtn
{
	namespace routing
	{
		class StaticRoutingExtension : public BaseRouter::ThreadedExtension
		{
		public:
			StaticRoutingExtension();
			virtual ~StaticRoutingExtension();

			void notify(const dtn::core::Event *evt);

			class StaticRoute
			{
			public:
				virtual ~StaticRoute() = 0;
				virtual bool match(const dtn::data::EID &eid) const = 0;
				virtual const dtn::data::EID& getDestination() const = 0;
				virtual const std::string toString() const = 0;
				virtual size_t getExpiration() const { return 0; };
			};

		protected:
			void run();
			void __cancellation();

		private:
			class RegexRoute : public StaticRoute
			{
			public:
				RegexRoute(const std::string &regex, const dtn::data::EID &dest);
				virtual ~RegexRoute();

				bool match(const dtn::data::EID &eid) const;
				const dtn::data::EID& getDestination() const;

				/**
				 * copy and assignment operators
				 * @param obj The object to copy
				 * @return
				 */
				RegexRoute(const RegexRoute &obj);
				RegexRoute& operator=(const RegexRoute &obj);

				/**
				 * Describe this route as a one-line-string.
				 * @return
				 */
				const std::string toString() const;

			private:
				dtn::data::EID _dest;
				std::string _regex_str;
				regex_t _regex;
				bool _invalid;
			};

			class EIDRoute : public StaticRoute
			{
			public:
				EIDRoute(const dtn::data::EID &match, const dtn::data::EID &nexthop, size_t expiretime = 0);
				virtual ~EIDRoute();

				bool match(const dtn::data::EID &eid) const;
				const dtn::data::EID& getDestination() const;

				/**
				 * Describe this route as a one-line-string.
				 * @return
				 */
				const std::string toString() const;

				size_t getExpiration() const;

			private:
				const dtn::data::EID _nexthop;
				const dtn::data::EID _match;
				const size_t expiretime;
			};

			class Task
			{
			public:
				virtual ~Task() {};
				virtual std::string toString() = 0;
			};

			class SearchNextBundleTask : public Task
			{
			public:
				SearchNextBundleTask(const dtn::data::EID &eid);
				virtual ~SearchNextBundleTask();

				virtual std::string toString();

				const dtn::data::EID eid;
			};

			class ProcessBundleTask : public Task
			{
			public:
				ProcessBundleTask(const dtn::data::MetaBundle &meta, const dtn::data::EID &origin);
				virtual ~ProcessBundleTask();

				virtual std::string toString();

				const dtn::data::MetaBundle bundle;
				const dtn::data::EID origin;
			};

			class ClearRoutesTask : public Task
			{
			public:
				ClearRoutesTask();
				virtual ~ClearRoutesTask();
				virtual std::string toString();
			};

			class RouteChangeTask : public Task
			{
			public:
				enum CHANGE_TYPE
				{
					ROUTE_ADD = 0,
					ROUTE_DEL = 1
				};

				RouteChangeTask(CHANGE_TYPE type, StaticRoute *route);
				virtual ~RouteChangeTask();

				virtual std::string toString();

				CHANGE_TYPE type;
				StaticRoute *route;
			};

			class ExpireTask : public Task
			{
			public:
				ExpireTask(size_t timestamp);
				virtual ~ExpireTask();

				virtual std::string toString();

				size_t timestamp;
			};

			/**
			 * hold queued tasks for later processing
			 */
			ibrcommon::Queue<StaticRoutingExtension::Task* > _taskqueue;

			/**
			 * static list of routes
			 */
			std::list<StaticRoutingExtension::StaticRoute*> _routes;
			ibrcommon::Mutex _expire_lock;
			size_t next_expire;
		};
	}
}

#endif /* STATICROUTINGEXTENSION_H_ */