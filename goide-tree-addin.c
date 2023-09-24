#include "goide-tree-addin.h"

#include <libide-gtk.h>
#include <libide-projects.h>
#include <libide-tree.h>

#include "goide-new-class-diag.h"

struct _GoideTreeAddin {
	GObject parent_instance;
	IdeTree* tree;
};

static void goide_tree_addin_addin_init(IdeTreeAddinInterface* iface);
G_DEFINE_TYPE_WITH_CODE(GoideTreeAddin, goide_tree_addin, G_TYPE_OBJECT,
	G_IMPLEMENT_INTERFACE(IDE_TYPE_TREE_ADDIN, goide_tree_addin_addin_init)
)

static void goide_tree_addin_class_init(GoideTreeAddinClass* class) {}

static void goide_tree_addin_init(GoideTreeAddin* self) {
	self->tree = NULL;
}

static void goide_tree_addin_spawn_new_class_diag(GAction*, GVariant*, GoideTreeAddin* self) {
	IdeTreeNode* node = ide_tree_get_selected_node(self->tree);
	if (ide_tree_node_holds(node, IDE_TYPE_PROJECT_FILE)) {
		IdeProjectFile* file_node = ide_tree_node_get_item(node);

		g_autoptr(GFile) file = NULL;
		if (ide_project_file_is_directory(file_node))
			file = ide_project_file_ref_file(file_node);
		else
			file = g_object_ref(ide_project_file_get_directory(file_node));

		GtkWidget* diag = goide_new_class_diag_new(g_file_peek_path(file));
		gtk_window_set_modal(GTK_WINDOW(diag), TRUE);
		GtkRoot* root = gtk_widget_get_root(GTK_WIDGET(self->tree));
		if (GTK_IS_WINDOW(root))
			gtk_window_set_transient_for(GTK_WINDOW(diag), GTK_WINDOW(root));

		ide_gtk_window_present(GTK_WINDOW(diag)); // incorrectly labled as not taking ownership on floating refs
	}
}

static void goide_tree_addin_load(IdeTreeAddin* addin, IdeTree* tree) {
	GoideTreeAddin* self = GOIDE_TREE_ADDIN(addin);
	self->tree = tree;

	g_autoptr(GSimpleActionGroup) actions = g_simple_action_group_new();
	g_autoptr(GSimpleAction) new_class = g_simple_action_new("new_class", NULL);
	g_signal_connect(new_class, "activate", G_CALLBACK(goide_tree_addin_spawn_new_class_diag), self);
	g_action_map_add_action(G_ACTION_MAP(actions), G_ACTION(new_class));

	gtk_widget_insert_action_group(GTK_WIDGET(self->tree), "goide", G_ACTION_GROUP(actions));
}

static void goide_tree_addin_unload(IdeTreeAddin*, IdeTree* tree) {
	gtk_widget_insert_action_group(GTK_WIDGET(tree), "goide", NULL);
}

static void goide_tree_addin_addin_init(IdeTreeAddinInterface* iface) {
	iface->load = goide_tree_addin_load;
	iface->unload = goide_tree_addin_unload;
}
