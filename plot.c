#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

static void on_open(GApplication *app, GFile **files, gint n_files, const gchar *hint) {
    GtkApplication *gtk_app = GTK_APPLICATION(app);
    GtkWidget *window = gtk_application_window_new(gtk_app);
    gtk_window_set_title(GTK_WINDOW(window), "Graph");
    // gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    for (int i = 0; i < n_files; i++) {
        GFile *file = files[i];
        char *file_path = g_file_get_path(file);

        FILE *fp = fopen(file_path, "r");
        if (!fp) {
            g_warning("Could not open file: %s", file_path);
            g_free(file_path);
            continue;
        }

        printf("Reading data from file: %s\n", file_path);
        char line[256];
        while (fgets(line, sizeof(line), fp)) {
            printf("%s", line);
        }

        fclose(fp);
        g_free(file_path);
    }

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("com.example.Graph", G_APPLICATION_HANDLES_OPEN);

    g_signal_connect(app, "open", G_CALLBACK(on_open), NULL); // read data from file and create window

    int status = g_application_run(G_APPLICATION(app), argc, argv); 
    g_object_unref(app);

    return status;
}
