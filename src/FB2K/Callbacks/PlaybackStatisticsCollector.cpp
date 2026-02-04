#include "PCH.hpp"

namespace smp
{
	class PlaybackStatisticsCollector : public playback_statistics_collector
	{
	public:
		void on_item_played(metadb_handle_ptr handle) noexcept final
		{
			EventDispatcher::Get().PutEventToAll(GenerateEvent_JsCallback(EventId::kFbItemPlayed, handle));
		}
	};

	FB2K_SERVICE_FACTORY(PlaybackStatisticsCollector);
}
