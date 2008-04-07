#include <gtk/gtk.h>

int
main (int argc, char **argv)
{
  GtkWidget *window, *vbox, *entry, *button;

  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (window,
		    "destroy",
		    G_CALLBACK (gtk_main_quit),
		    NULL);

  if (1)
    {
      vbox = gtk_vbox_new (FALSE, 12);
      gtk_container_add (GTK_CONTAINER (window), vbox);
      gtk_container_set_border_width (GTK_CONTAINER (window), 12);

      entry = gtk_entry_new ();
      gtk_box_pack_start (GTK_BOX (vbox), entry, FALSE, FALSE, 0);

      button = gtk_button_new_with_label ("Quit");
      g_signal_connect (button, 
                        "clicked",
                        G_CALLBACK (gtk_main_quit),
                        NULL);
      gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
    }

  gtk_widget_show_all (window);
  gtk_main ();

  return 0;
}
