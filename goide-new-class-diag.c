#include "goide-new-class-diag.h"

struct _GoideNewClassDiag {
	AdwWindow parent_instance;

	GCancellable* diag_closed_canc;

	GtkButton* create_btn;
	AdwEntryRow* class_name_inp;
	GtkListView* templates;
	AdwEntryRow* parent_class_inp;

	gchar* path;

	GtkStringList* templates_list;
	GtkSingleSelection* selection_model;
};

G_DEFINE_TYPE (GoideNewClassDiag, goide_new_class_diag, ADW_TYPE_WINDOW)

enum {
	PROP_PATH = 1,
	N_PROPERTIES
};
static GParamSpec* obj_properties[N_PROPERTIES] = { NULL, };

static void goide_new_class_diag_object_finalize(GObject* object) {
	GoideNewClassDiag* self = GOIDE_NEW_CLASS_DIAG(object);
	g_free(self->path);
	g_object_unref(self->diag_closed_canc);
	G_OBJECT_CLASS(goide_new_class_diag_parent_class)->finalize(object);
}

static void goide_new_class_diag_object_dispose(GObject* object) {
	GoideNewClassDiag* self = GOIDE_NEW_CLASS_DIAG(object);

	gtk_widget_dispose_template(GTK_WIDGET(self), GOIDE_TYPE_NEW_CLASS_DIAG);
	g_clear_object(&self->selection_model); // also frees self->templates_list

	G_OBJECT_CLASS(goide_new_class_diag_parent_class)->dispose(object);
}

static void goide_new_class_diag_object_get_property(GObject* object, guint prop_id, GValue* val, GParamSpec* pspec) {
	GoideNewClassDiag* self = GOIDE_NEW_CLASS_DIAG(object);
	switch (prop_id) {
		case PROP_PATH:
			g_value_set_string(val, goide_new_class_diag_get_path(self));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void goide_new_class_diag_object_set_property(GObject* object, guint prop_id, const GValue* val, GParamSpec* pspec) {
	GoideNewClassDiag* self = GOIDE_NEW_CLASS_DIAG(object);
	switch (prop_id) {
		case PROP_PATH:
			goide_new_class_diag_set_path(self, g_value_get_string(val));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static gboolean goide_new_class_diag_window_close_req(GtkWindow* window) {
	GoideNewClassDiag* self = GOIDE_NEW_CLASS_DIAG(window);
	g_cancellable_cancel(self->diag_closed_canc);
	return FALSE;
}

static void goide_new_class_diag_class_init(GoideNewClassDiagClass* class) {
	GObjectClass* object_class = G_OBJECT_CLASS(class);
	GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(class);
	GtkWindowClass* window_class = GTK_WINDOW_CLASS(class);

	object_class->finalize = goide_new_class_diag_object_finalize;
	object_class->dispose = goide_new_class_diag_object_dispose;
	object_class->get_property = goide_new_class_diag_object_get_property;
	object_class->set_property = goide_new_class_diag_object_set_property;

	window_class->close_request = goide_new_class_diag_window_close_req;

	gtk_widget_class_set_template_from_resource(widget_class, "/plugins/goide/gtk/new_class.ui");
	gtk_widget_class_bind_template_child(widget_class, GoideNewClassDiag, create_btn);
	gtk_widget_class_bind_template_child(widget_class, GoideNewClassDiag, class_name_inp);
	gtk_widget_class_bind_template_child(widget_class, GoideNewClassDiag, templates);
	gtk_widget_class_bind_template_child(widget_class, GoideNewClassDiag, parent_class_inp);

	obj_properties[PROP_PATH] = g_param_spec_string("path", "Path", "The path to put the generated files in", ".", G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_EXPLICIT_NOTIFY);
	g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
}

static void goide_new_class_diag_got_template_written_cb(GSubprocess* sub, GAsyncResult* res, GoideNewClassDiag* self) {
	gtk_widget_set_sensitive(GTK_WIDGET(self), TRUE);

	GError* err = NULL;
	gboolean status = g_subprocess_wait_check_finish(sub, res, &err);
	if (err) {
		g_critical("Unable to write template: %s", err->message);
		g_error_free(err);
		return;
	}
	if (!status || !g_subprocess_get_successful(sub)) {
		g_critical("Internal got failure occurred when writing template");
		return;
	}

	gtk_window_close(GTK_WINDOW(self));
}

static void goide_new_class_diag_create_btn_clicked_cb(GtkButton*, GoideNewClassDiag* self) {
	const gchar* template = gtk_string_list_get_string(self->templates_list, gtk_single_selection_get_selected(self->selection_model));
	const gchar* class = gtk_editable_get_text(GTK_EDITABLE(self->class_name_inp));
	const gchar* parent_class = gtk_editable_get_text(GTK_EDITABLE(self->parent_class_inp));
	if (parent_class && !*parent_class)
		parent_class = NULL;

	GError* err = NULL;
	g_autoptr(GSubprocess) sub = g_subprocess_new(G_SUBPROCESS_FLAGS_NONE, &err, "got", "--output", self->path, template, class, parent_class, NULL);
	if (err) {
		g_critical("Failed creating subprocess: %s", err->message);
		g_error_free(err);
		return;
	}

	gtk_widget_set_sensitive(GTK_WIDGET(self), FALSE);
	g_subprocess_wait_check_async(sub, self->diag_closed_canc, (GAsyncReadyCallback)goide_new_class_diag_got_template_written_cb, self);
}

static void goide_new_class_diag_got_available_templates_resolved_cb(GSubprocess* sub, GAsyncResult* res, GoideNewClassDiag* self) {
	GBytes* stdout = NULL;
	GError* err = NULL;
	gboolean status = g_subprocess_communicate_finish(sub, res, &stdout, NULL, &err);
	if (err) {
		g_critical("Unable to fetch available templates: %s", err->message);
		g_error_free(err);
		return;
	}
	if (!status || !g_subprocess_get_successful(sub)) {
		g_critical("Internal got failure occurred when fetching available templates");
		return;
	}

	gsize len;
	const gchar* data = g_bytes_get_data(stdout, &len);

	const gchar* start = data;
	const gchar* end = data;
	while (end < data + len) {
		if (*end == '\n') {
			gsize line_length = end - start;
			gchar* line = g_strndup(start, line_length);
			gtk_string_list_take(self->templates_list, line);
			start = end + 1;
		}
		end++;
	}
	if (start < end) { // if there is still some data without ending newline left
		gsize line_length = end - start;
		gchar* line = g_strndup(start, line_length);
		gtk_string_list_take(self->templates_list, line);
	}
}

static void goide_new_class_diag_init(GoideNewClassDiag* self) {
	self->diag_closed_canc = g_cancellable_new();
	self->path = NULL;
	self->templates_list = gtk_string_list_new(NULL);
	self->selection_model = gtk_single_selection_new(G_LIST_MODEL(self->templates_list));
	gtk_widget_init_template(GTK_WIDGET(self));
	gtk_list_view_set_model(self->templates, GTK_SELECTION_MODEL(self->selection_model));

	g_signal_connect(self->create_btn, "clicked", G_CALLBACK(goide_new_class_diag_create_btn_clicked_cb), self);

	GError* err = NULL;
	g_autoptr(GSubprocess) sub = g_subprocess_new(G_SUBPROCESS_FLAGS_STDOUT_PIPE, &err, "got", "--list-templates", NULL);
	if (err) {
		g_critical("Failed creating subprocess: %s", err->message);
		g_error_free(err);
	} else {
		g_subprocess_communicate_async(sub, NULL, self->diag_closed_canc, (GAsyncReadyCallback)goide_new_class_diag_got_available_templates_resolved_cb, self);
	}
}

GtkWidget* goide_new_class_diag_new(const gchar* path) {
	return g_object_new(GOIDE_TYPE_NEW_CLASS_DIAG, "path", path, NULL);
}

const gchar* goide_new_class_diag_get_path(GoideNewClassDiag* self) {
	g_return_val_if_fail(GOIDE_IS_NEW_CLASS_DIAG(self), NULL);
	return self->path;
}

void goide_new_class_diag_set_path(GoideNewClassDiag* self, const gchar* path) {
	g_return_if_fail(GOIDE_IS_NEW_CLASS_DIAG(self));
	if (self->path)
		g_free(self->path);
	self->path = g_strdup(path);

	g_object_notify_by_pspec(G_OBJECT(self), obj_properties[PROP_PATH]);
}
