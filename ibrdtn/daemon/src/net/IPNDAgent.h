/*
 * IPNDAgent.h
 *
 * IPND is based on the Internet-Draft for DTN-IPND.
 *
 *
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef IPNDAGENT_H_
#define IPNDAGENT_H_

#include "Component.h"
#include "net/DiscoveryAgent.h"
#include "net/DiscoveryAnnouncement.h"
#include "core/EventReceiver.h"
#include <ibrcommon/net/vinterface.h>
#include <ibrcommon/net/vsocket.h>
#include <ibrcommon/link/LinkManager.h>
#include <ibrcommon/thread/Mutex.h>
#include <list>
#include <map>

using namespace dtn::data;

namespace dtn
{
	namespace net
	{
		class IPNDAgent : public DiscoveryAgent, public dtn::core::EventReceiver, public dtn::daemon::IndependentComponent, public ibrcommon::LinkManager::EventCallback
		{
		public:
			static const std::string TAG;

			IPNDAgent(int port);
			virtual ~IPNDAgent();

			void add(const ibrcommon::vaddress &address);
			void bind(const ibrcommon::vinterface &net);

			/**
			 * @see Component::getName()
			 */
			virtual const std::string getName() const;

			void eventNotify(const ibrcommon::LinkEvent &evt);

			/**
			 * @see EventReceiver::raiseEvent()
			 */
			void raiseEvent(const Event *evt) throw ();

		protected:
			void sendAnnoucement(const uint16_t &sn, std::list<dtn::net::DiscoveryServiceProvider*> &provider);
			virtual void componentRun() throw ();
			virtual void componentUp() throw ();
			virtual void componentDown() throw ();
			void __cancellation() throw ();

		private:
			void leave_interface(const ibrcommon::vinterface &iface) throw ();
			void join_interface(const ibrcommon::vinterface &iface) throw ();
			void send(const DiscoveryAnnouncement &a, const ibrcommon::vinterface &iface, const ibrcommon::vaddress &addr);

			void listen(const ibrcommon::vinterface &iface) throw ();
			void unlisten(const ibrcommon::vinterface &iface) throw ();

			DiscoveryAnnouncement::DiscoveryVersion _version;
			ibrcommon::vsocket _recv_socket;
			ibrcommon::vsocket _send_socket;
			bool _send_socket_state;

			std::set<ibrcommon::vaddress> _destinations;

			ibrcommon::Mutex _interface_lock;
			std::set<ibrcommon::vinterface> _interfaces;
		};
	}
}

#endif /* IPNDAGENT_H_ */
