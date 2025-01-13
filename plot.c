#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <cairo.h>

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

static DataSet global_data_set;

int parse_int(const char *line) {
    return atoi(line);
}

int parse_data_points(FILE *fp, DataSet *data_set) {
    data_set->data_points = (Point *)malloc(data_set->num_data_points * sizeof(Point));
    if (!data_set->data_points) {
        fprintf(stderr, "Memory allocation failed for data points.\n");
        return 0;
    }

    char line[256];
    for (int i = 0; i < data_set->num_data_points; i++) {
        if (fgets(line, sizeof(line), fp)) {
            sscanf(line, "%d %d", &data_set->data_points[i].x, &data_set->data_points[i].y);
        } else {
            fprintf(stderr, "Error reading data point %d.\n", i + 1);
            return 0;
        }
    }
    return 1;
}

int parse_centroids(FILE *fp, DataSet *data_set) {
    data_set->centroids = (Point *)malloc(data_set->num_clusters * sizeof(Point));
    if (!data_set->centroids) {
        fprintf(stderr, "Memory allocation failed for centroids.\n");
        return 0;
    }

    char line[256];
    for (int i = 0; i < data_set->num_clusters; i++) {
        if (fgets(line, sizeof(line), fp)) {
            sscanf(line, "%d %d", &data_set->centroids[i].x, &data_set->centroids[i].y);
        } else {
            fprintf(stderr, "Error reading centroid %d.\n", i + 1);
            return 0;
        }
    }
    return 1;
}

static void draw_function(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer user_data) {
    // Clear background
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    // Calculate scaling factors to fit all points
    double max_x = 0, max_y = 0;
    for (int i = 0; i < global_data_set.num_data_points; i++) {
        if (global_data_set.data_points[i].x > max_x) max_x = global_data_set.data_points[i].x;
        if (global_data_set.data_points[i].y > max_y) max_y = global_data_set.data_points[i].y;
    }
    for (int i = 0; i < global_data_set.num_clusters; i++) {
        if (global_data_set.centroids[i].x > max_x) max_x = global_data_set.centroids[i].x;
        if (global_data_set.centroids[i].y > max_y) max_y = global_data_set.centroids[i].y;
    }

    // Add padding
    max_x += 2;
    max_y += 2;

    double scale_x = (width - 40) / max_x;
    double scale_y = (height - 40) / max_y;
    double scale = scale_x < scale_y ? scale_x : scale_y;

    // Draw coordinate axes
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 1);
    cairo_move_to(cr, 20, height - 20);
    cairo_line_to(cr, width - 20, height - 20);
    cairo_move_to(cr, 20, height - 20);
    cairo_line_to(cr, 20, 20);
    cairo_stroke(cr);

    // Draw data points in blue
    cairo_set_source_rgb(cr, 0, 0, 1);
    for (int i = 0; i < global_data_set.num_data_points; i++) {
        double x = 20 + global_data_set.data_points[i].x * scale;
        double y = height - 20 - global_data_set.data_points[i].y * scale;
        cairo_arc(cr, x, y, 5, 0, 2 * M_PI);
        cairo_fill(cr);
    }

    // Draw centroids in red
    cairo_set_source_rgb(cr, 1, 0, 0);
    for (int i = 0; i < global_data_set.num_clusters; i++) {
        double x = 20 + global_data_set.centroids[i].x * scale;
        double y = height - 20 - global_data_set.centroids[i].y * scale;
        cairo_rectangle(cr, x - 5, y - 5, 10, 10);
        cairo_fill(cr);
    }
}

static void on_open(GApplication *app, GFile **files, gint n_files, const gchar *hint) {
    GtkApplication *gtk_app = GTK_APPLICATION(app);
    GtkWidget *window = gtk_application_window_new(gtk_app);
    gtk_window_set_title(GTK_WINDOW(window), "Graph");

    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(drawing_area), draw_function, NULL, NULL);
    gtk_widget_set_size_request(drawing_area, 800, 600);
    gtk_window_set_child(GTK_WINDOW(window), drawing_area);

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

        char line[256];
        fgets(line, sizeof(line), fp);
        global_data_set.num_data_points = parse_int(line);
        parse_data_points(fp, &global_data_set);

        fgets(line, sizeof(line), fp);
        global_data_set.num_clusters = parse_int(line);
        parse_centroids(fp, &global_data_set);

        printf("Number of data points: %d\n", global_data_set.num_data_points);
        for (int j = 0; j < global_data_set.num_data_points; j++) {
            printf("Data Point %d: (%d, %d)\n", j + 1, global_data_set.data_points[j].x, global_data_set.data_points[j].y);
        }
        printf("Number of clusters: %d\n", global_data_set.num_clusters);
        for (int j = 0; j < global_data_set.num_clusters; j++) {
            printf("Centroid %d: (%d, %d)\n", j + 1, global_data_set.centroids[j].x, global_data_set.centroids[j].y);
        }

        fclose(fp);
        g_free(file_path);
    }

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("com.example.Graph", G_APPLICATION_HANDLES_OPEN);
    g_signal_connect(app, "open", G_CALLBACK(on_open), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    free(global_data_set.data_points);
    free(global_data_set.centroids);

    return status;
}