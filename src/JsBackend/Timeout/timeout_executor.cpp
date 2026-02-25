#include "PCH.hpp"
#include "timeout_executor.h"

#include "timeout_manager.h"
#include "timer_manager_native.h"

#include <Panel/js_panel_window.h>

namespace smp
{
	TimeoutExecutor::TimeoutExecutor(TimeoutManager& pParent, std::shared_ptr<PanelTarget> pTarget) : pParent_(pParent), pTarget_(pTarget) {}
	TimeoutExecutor::~TimeoutExecutor() {}

	void TimeoutExecutor::Shutdown()
	{
		if (pTimer_)
		{
			pTimer_->Cancel(true);
			pTimer_ = nullptr;
		}

		mode_ = Mode::Shutdown;
		deadlineOpt_.reset();
	}

	void TimeoutExecutor::Cancel(bool waitForDestruction)
	{
		if (pTimer_)
		{
			pTimer_->Cancel(waitForDestruction);
		}

		mode_ = Mode::None;
		deadlineOpt_.reset();
	}

	void TimeoutExecutor::MaybeSchedule(const TimeStamp& targetDeadline)
	{
		if (mode_ == Mode::Shutdown)
		{
			return;
		}

		if (mode_ == Mode::Immediate || mode_ == Mode::Delayed)
		{
			return MaybeReschedule(targetDeadline);
		}

		return Schedule(targetDeadline);
	}

	void TimeoutExecutor::Run()
	{
		// If the executor is canceled and then rescheduled its possible to get
		// spurious executions here.  Ignore these unless our current mode matches.
		if (mode_ == Mode::Immediate)
		{
			MaybeExecute();
		}
	}

	void TimeoutExecutor::Notify()
	{
		// If the executor is canceled and then rescheduled its possible to get
		// spurious executions here.  Ignore these unless our current mode matches.
		if (mode_ == Mode::Delayed)
		{
			MaybeExecute();
		}
	}

	void TimeoutExecutor::Schedule(const TimeStamp& targetDeadline)
	{
		const auto now = TimeStamp::clock::now();

		if (targetDeadline <= (now + TimerManager_Native::GetAllowedEarlyFiringTime()))
		{
			return ScheduleImmediate(targetDeadline);
		}

		return ScheduleDelayed(targetDeadline, now);
	}

	void TimeoutExecutor::MaybeReschedule(const TimeStamp& targetDeadline)
	{
		if (targetDeadline >= *deadlineOpt_)
		{
			return;
		}

		if (mode_ == Mode::Immediate)
		{
			// Don't reduce the deadline here as we want to execute the
			// timer we originally scheduled even if its a few microseconds
			// in the future.
			return;
		}

		Cancel(false);
		Schedule(targetDeadline);
	}

	void TimeoutExecutor::ScheduleImmediate(const TimeStamp& targetDeadline)
	{
		EventDispatcher::Get().PutRunnable(pTarget_->GetHwnd(), shared_from_this());

		mode_ = Mode::Immediate;
		deadlineOpt_ = targetDeadline;
	}

	void TimeoutExecutor::ScheduleDelayed(const TimeStamp& targetDeadline, const TimeStamp& now)
	{
		if (!pTimer_)
		{
			pTimer_ = TimerManager_Native::Get().CreateTimer(pTarget_);

			// Re-evaluate if we should have scheduled this immediately
			if (targetDeadline <= (now + TimerManager_Native::GetAllowedEarlyFiringTime()))
			{
				return ScheduleImmediate(targetDeadline);
			}
		}
		else
		{
			// Always call Cancel() in case we are re-using a timer.
			pTimer_->Cancel(false);
		}

		// Note, we cannot use timers that take
		// integer milliseconds. We need higher precision. Consider this
		// situation:
		//
		// 1. setTimeout(f, 1);
		// 2. do some work for 500us
		// 3. setTimeout(g, 1);
		//
		// This should fire f() and g() 500us apart.

		// QWR: Mozilla used delay here instead of deadline for some reason,
		// which added additional delay to timer
		pTimer_->Start(*this, targetDeadline);

		mode_ = Mode::Delayed;
		deadlineOpt_ = targetDeadline;
	}

	void TimeoutExecutor::MaybeExecute()
	{
		TimeStamp deadline(*deadlineOpt_);

		// Sometimes timer or canceled timers will fire too early.  If this
		// happens then just cap our deadline to our maximum time in the future
		// and proceed.  If there are no timers ready we will get rescheduled
		// by TimeoutManager.
		const auto now = TimeStamp::clock::now();
		const auto limit = now + TimerManager_Native::GetAllowedEarlyFiringTime();
		if (deadline > limit)
		{
			deadline = limit;
		}

		Cancel(false);

		pParent_.RunTimeout(now, deadline);
	}
}
