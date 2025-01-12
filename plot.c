#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>


typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    int num_data_points;
    Point *data_points;
    int num_clusters;
    Point *centroids;
} DataSet;

int parse_int(const char *line) {
    return atoi(line);
}

int parse_data_points(FILE *fp, DataSet *data_set) {
    data_set->data_points = (Point *)malloc(data_set->num_data_points * sizeof(Point));
    char line[256];
    for (int i = 0; i < data_set->num_data_points; i++) {
        fgets(line, sizeof(line), fp);
        data_set->data_points[i].x = parse_int(line);
        fgets(line, sizeof(line), fp);
        data_set->data_points[i].y = parse_int(line);
    }
    return 1;
}

int parse_centroids(FILE *fp, DataSet *data_set) {
    data_set->centroids = (Point *)malloc(data_set->num_clusters * sizeof(Point));
    char line[256];
    for (int i = 0; i < data_set->num_clusters; i++) {
        fgets(line, sizeof(line), fp);
        data_set->centroids[i].x = parse_int(line);
        fgets(line, sizeof(line), fp);
        data_set->centroids[i].y = parse_int(line);
    }
    return 1;
}


static void on_open(GApplication *app, GFile **files, gint n_files, const gchar *hint) {
    GtkApplication *gtk_app = GTK_APPLICATION(app);
    GtkWidget *window = gtk_application_window_new(gtk_app);
    gtk_window_set_title(GTK_WINDOW(window), "Graph");

    if (n_files > 0) {
        GFile *file = files[0]; 
        char *file_path = g_file_get_path(file);

        FILE *fp = fopen(file_path, "r");
        if (!fp) {
            g_warning("Could not open file: %s", file_path);
            g_free(file_path);
            return;  
        }

        printf("Reading data from file: %s\n", file_path);
        DataSet data_set;
        char line[256];
        // while (fgets(line, sizeof(line), fp)) {
        //     printf("%s", line);
        // }

        fgets(line, sizeof(line), fp);
        data_set.num_data_points = parse_int(line);

        parse_data_points(fp, &data_set);

        fgets(line, sizeof(line), fp);
        data_set.num_clusters = parse_int(line);

        parse_centroids(fp, &data_set);

        printf("Number of data points: %d\n", data_set.num_data_points);
        for (int j = 0; j < data_set.num_data_points; j++) {
            printf("Data Point %d: (%d, %d)\n", j + 1, data_set.data_points[j].x, data_set.data_points[j].y);
        }
        printf("Number of clusters: %d\n", data_set.num_clusters);
        for (int j = 0; j < data_set.num_clusters; j++) {
            printf("Centroid %d: (%d, %d)\n", j + 1, data_set.centroids[j].x, data_set.centroids[j].y);
        }
        fclose(fp);
        g_free(file_path);
        free(data_set.data_points);
        free(data_set.centroids);
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
