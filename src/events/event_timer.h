#pragma once

#include <events/event.h>
#include <timeout/timer_interface_fwd.h>

namespace smp
{

class Event_Timer
	: public EventBase
{
public:
	Event_Timer(std::shared_ptr<ITimer> pTimer, uint64_t generation);

	void Run() override;

private:
	std::shared_ptr<ITimer> pTimer_;
	uint64_t generation_;
};

} // namespace smp
