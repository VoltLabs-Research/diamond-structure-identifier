#include <volt/plugin/plugin_entry.h>
#include <volt/plugin/identify_diamond_service.h>

using namespace Volt;
using namespace Volt::Plugin;
using S = IdentifyDiamondService;

static const std::vector<OptionBinding<S>> bindings = {};

VOLT_SERVICE_PLUGIN("volt-identify-diamond", "Identify Diamond Structure", S, bindings)
