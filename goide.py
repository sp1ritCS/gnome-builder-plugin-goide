#!/usr/bin/python3

import os
import sys

import gi
from gi.repository import GObject
from gi.repository import Gio
from gi.repository import Gtk
from gi.repository import Adw

from gi.repository import Ide

new_class_module = "goide-new_class"

class GObjectIdeExtensions(GObject.Object, Ide.TreeAddin):
    def spawn_new_class_diag(self, action, _value):
        node = self.tree.get_selected_node()
        if (node.holds(Ide.ProjectFile)):
            file = node.props.item.ref_file()
            filetype = file.query_file_type(Gio.FileQueryInfoFlags.NONE, None)
            if (filetype != Gio.FileType.DIRECTORY):
                file = file.get_parent()

            diag = sys.modules[new_class_module].GObjectIdeExtNewClassDiag(output_dir=file.get_path())
            diag.set_modal(True)
            # Ide.Tree also implements a different "get_root" method
            root = Gtk.Widget.get_root(self.tree)
            if (isinstance(root, Gtk.Window)):
                diag.set_transient_for(root)
            Ide.gtk_window_present(diag)
 
    def do_load(self, tree):
        print("tree addin init")
        __import__(new_class_module)
        
        self.tree = tree

        actions = Gio.SimpleActionGroup.new()

        new_class = Gio.SimpleAction.new("new_class", None)
        new_class.connect("activate", self.spawn_new_class_diag)
        actions.add_action(new_class)

        tree.insert_action_group("goide", actions)


    def do_unload(self, tree):
        print("tree addin uninit")
        tree.insert_action_group("goide", None)
