#pragma once

namespace smp
{
	using TimeStamp = std::chrono::time_point<std::chrono::steady_clock>;
	using TimeDuration = TimeStamp::duration;

	class PanelTarget;
	class Timer_Native;
	class TimerManager_Native;

	struct TimerNotifyTask
	{
		virtual ~TimerNotifyTask() = default;
		virtual void Notify() = 0;
	};

	class ITimer
	{
	public:
		virtual ~ITimer() = default;

		virtual void Start(TimerNotifyTask& task, const TimeStamp& when) = 0;
		virtual void Cancel(bool waitForDestruction) = 0;

		virtual void Fire(int64_t generation) = 0;

		[[nodiscard]] virtual PanelTarget& Target() const = 0;
		[[nodiscard]] virtual const TimeStamp& When() const = 0;
		[[nodiscard]] virtual int64_t Generation() const = 0;
	};
}
