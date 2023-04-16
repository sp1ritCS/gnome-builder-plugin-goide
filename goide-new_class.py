import gi
from gi.repository import GObject
from gi.repository import Gio
from gi.repository import Gtk
from gi.repository import Adw

stream_chunk_size = 2048

@Gtk.Template(resource_path='/plugins/goide/gtk/new_class.ui')
class GObjectIdeExtNewClassDiag(Adw.Window):
    __gtype_name__ = "GObjectIdeExtNewClassDiag"

    create_btn = Gtk.Template.Child()
    class_name_inp = Gtk.Template.Child()
    templates = Gtk.Template.Child()

    def got_template_written_cb(self, src, res):
        self.set_sensitive(True)
        try:
            if (src.wait_check_finish(res)):
                self.close()
            else:
                print("failure")
        except Exception as err:
            print("Failure: " + err.message)



    @Gtk.Template.Callback()
    def create_btn_clicked_cb(self, *args):
        selected_template = self.templates_list.get_string(self.selection_model.get_selected())
        subproc = Gio.Subprocess.new(["got", "--output", self.output_dir, selected_template, self.class_name_inp.get_text()], Gio.SubprocessFlags.NONE)
        subproc.wait_check_async(None, self.got_template_written_cb)
        self.set_sensitive(False)

    def got_available_templates_resolved_cb(self, src, res):
        try:
            if (src.wait_check_finish(res)):
                stream = src.get_stdout_pipe()

                output = bytearray(0)
                buf = stream.read_bytes(stream_chunk_size, None).get_data()
                while buf:
                    output += buf
                    buf = stream.read_bytes(stream_chunk_size, None).get_data()
                stream.close()

                available_templates = output.decode("utf-8").strip().split('\n')
                for template in available_templates:
                    self.templates_list.append(template)

            else:
                print("failure")
        except Exception as err:
            print("Failure: " + err.message)


    def __init__(self, output_dir="."):
        self.output_dir = output_dir
        Gtk.Widget.__init__(self)
        
        self.templates_list = Gtk.StringList()
        self.selection_model = Gtk.SingleSelection.new(self.templates_list)
        self.templates.set_model(self.selection_model)

        subproc = Gio.Subprocess.new(["got", "--list-templates"], Gio.SubprocessFlags.STDOUT_PIPE)
        subproc.wait_check_async(None, self.got_available_templates_resolved_cb)
