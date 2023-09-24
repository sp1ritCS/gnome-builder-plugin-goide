#ifndef __GOIDENEWCLASSDIAG_H__
#define __GOIDENEWCLASSDIAG_H__

#include <glib-object.h>
#include <adwaita.h>

G_BEGIN_DECLS

#define GOIDE_TYPE_NEW_CLASS_DIAG (goide_new_class_diag_get_type())
G_DECLARE_FINAL_TYPE (GoideNewClassDiag, goide_new_class_diag, GOIDE, NEW_CLASS_DIAG, AdwWindow)

GtkWidget* goide_new_class_diag_new(const gchar* path);

const gchar* goide_new_class_diag_get_path(GoideNewClassDiag* self);
void goide_new_class_diag_set_path(GoideNewClassDiag* self, const gchar* path);

G_END_DECLS

#endif // __GOIDENEWCLASSDIAG_H__
