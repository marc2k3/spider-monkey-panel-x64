#include <stdafx.h>

#include "advanced_config.h"

namespace
{
advconfig_branch_factory branch_smp("Spider Monkey Panel", smp::guid::adv_branch, advconfig_branch::guid_branch_tools, 0);
advconfig_branch_factory branch_performance("Performance: restart is required", smp::guid::adv_branch_performance, smp::guid::adv_branch, 0);
advconfig_branch_factory branch_gc("GC", smp::guid::adv_branch_gc, smp::guid::adv_branch_performance, 0);
advconfig_branch_factory branch_debug("Debugging", smp::guid::adv_branch_debug, smp::guid::adv_branch, 99);
advconfig_branch_factory branch_log("Logging", smp::guid::adv_branch_log, smp::guid::adv_branch_debug, 0);
}

namespace smp::config::advanced
{
advconfig_integer_factory gc_max_heap(
	"Maximum heap size (in bytes) (0 - auto configuration)",
	smp::guid::adv_var_gc_max_heap,
	smp::guid::adv_branch_gc,
	0.0,
	0,
	0,
	std::numeric_limits<uint32_t>::max()
);

advconfig_integer_factory gc_max_heap_growth(
	"Allowed heap growth before GC trigger (in bytes) (0 - auto configuration)",
	smp::guid::adv_var_gc_max_heap_growth,
	smp::guid::adv_branch_gc,
	1.0,
	0,
	0,
	256UL * 1024 * 1024
);

advconfig_integer_factory gc_budget(
	"GC cycle time budget (in ms)",
	smp::guid::adv_var_gc_budget,
	smp::guid::adv_branch_gc,
	2.0,
	5,
	1,
	100
);

advconfig_integer_factory gc_delay(
	"Delay before next GC trigger (in ms)",
	smp::guid::adv_var_gc_delay,
	smp::guid::adv_branch_gc,
	3.0,
	50,
	1,
	500
);

advconfig_integer_factory gc_max_alloc_increase(
	"Allowed number of allocations before next GC trigger",
	smp::guid::adv_var_gc_max_alloc_increase,
	smp::guid::adv_branch_gc,
	4.0,
	1000,
	1,
	100000
);

advconfig_integer_factory performance_max_runtime(
	"Script execution time limit before triggering a `slow script` warning (in seconds)",
	smp::guid::adv_var_performance_max_runtime,
	smp::guid::adv_branch_performance,
	5.0,
	5,
	0,
	60
);

advconfig_checkbox_factory debug_log_extended_include_error(
	"Log additional information when `include()` fails",
	smp::guid::adv_var_log_include_search_paths,
	smp::guid::adv_branch_log,
	50.0,
	false
);

advconfig_checkbox_factory debug_use_custom_timer_engine(
	"Use custom timer engine",
	smp::guid::adv_var_debug_timer_engine,
	smp::guid::adv_branch_debug,
	60.0,
	false
);

} // namespace smp::config::advanced
