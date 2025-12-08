#pragma once

namespace smp::config::advanced
{

extern advconfig_integer_factory gc_budget;
extern advconfig_integer_factory gc_delay;
extern advconfig_integer_factory gc_max_alloc_increase;
extern advconfig_integer_factory gc_max_heap;
extern advconfig_integer_factory gc_max_heap_growth;
extern advconfig_integer_factory performance_max_runtime;

extern advconfig_checkbox_factory debug_log_extended_include_error;
extern advconfig_checkbox_factory debug_use_custom_timer_engine;

} // namespace smp::config::advanced
