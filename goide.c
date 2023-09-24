#include <libpeas.h>
#include <libide-tree.h>

#include "goide-tree-addin.h"

void peas_register_types(PeasObjectModule* module) {
	peas_object_module_register_extension_type(
		module,
		IDE_TYPE_TREE_ADDIN,
		GOIDE_TYPE_TREE_ADDIN
	);
}
