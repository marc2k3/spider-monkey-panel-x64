#include "PCH.hpp"

#include "advanced_config.h"

namespace
{
	advconfig_branch_factory branch_smp("Spider Monkey Panel: restart is required", smp::guid::adv_branch, advconfig_branch::guid_branch_tools, 0.0);
	advconfig_branch_factory branch_gc("Garbage collect", smp::guid::adv_branch_gc, smp::guid::adv_branch, 2.0);
}

namespace config::advanced
{
	advconfig_integer_factory slow_script_limit(
		"Script execution time limit before triggering a `slow script` warning (in seconds)",
		smp::guid::adv_var_slow_script_limit,
		smp::guid::adv_branch,
		1.0,
		5,
		0,
		60,
		preferences_state::needs_restart
	);

	advconfig_integer_factory gc_max_heap(
		"Maximum heap size (in bytes) (0 - auto configuration)",
		smp::guid::adv_var_gc_max_heap,
		smp::guid::adv_branch_gc,
		0.0,
		0,
		0,
		std::numeric_limits<uint32_t>::max(),
		preferences_state::needs_restart
	);

	advconfig_integer_factory gc_max_heap_growth(
		"Allowed heap growth before GC trigger (in bytes) (0 - auto configuration)",
		smp::guid::adv_var_gc_max_heap_growth,
		smp::guid::adv_branch_gc,
		1.0,
		0,
		0,
		256UL * 1024 * 1024,
		preferences_state::needs_restart
	);

	advconfig_integer_factory gc_budget(
		"GC cycle time budget (in ms)",
		smp::guid::adv_var_gc_budget,
		smp::guid::adv_branch_gc,
		2.0,
		5,
		1,
		100,
		preferences_state::needs_restart
	);

	advconfig_integer_factory gc_delay(
		"Delay before next GC trigger (in ms)",
		smp::guid::adv_var_gc_delay,
		smp::guid::adv_branch_gc,
		3.0,
		50,
		1,
		500,
		preferences_state::needs_restart
	);

	advconfig_integer_factory gc_max_alloc_increase(
		"Allowed number of allocations before next GC trigger",
		smp::guid::adv_var_gc_max_alloc_increase,
		smp::guid::adv_branch_gc,
		4.0,
		1000,
		1,
		100000,
		preferences_state::needs_restart
	);
}
