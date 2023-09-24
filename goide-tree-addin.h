#ifndef __GOIDETREEADDIN_H__
#define __GOIDETREEADDIN_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define GOIDE_TYPE_TREE_ADDIN (goide_tree_addin_get_type())
G_DECLARE_FINAL_TYPE (GoideTreeAddin, goide_tree_addin, GOIDE, TREE_ADDIN, GObject)

G_END_DECLS

#endif // __GOIDETREEADDIN_H__
