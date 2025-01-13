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
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    int margin = 40;
    int graph_width = width - 2 * margin;
    int graph_height = height - 2 * margin;

    double scale_x = graph_width / 2.0;
    double scale_y = graph_height / 2.0;

    cairo_set_source_rgba(cr, 0.8, 0.8, 0.8, 1.0);
    cairo_set_line_width(cr, 0.5);

    for (double x = -1.0; x <= 1.0; x += 0.25) {
        double screen_x = margin + (x + 1.0) * scale_x;
        cairo_move_to(cr, screen_x, margin);
        cairo_line_to(cr, screen_x, height - margin);
    }
    
    for (double y = -0.75; y <= 1.0; y += 0.25) {
        double screen_y = height - margin - (y + 1.0) * scale_y;
        cairo_move_to(cr, margin, screen_y);
        cairo_line_to(cr, width - margin, screen_y);
    }
    cairo_stroke(cr);

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 1.0);

    cairo_move_to(cr, margin + scale_x, margin);
    cairo_line_to(cr, margin + scale_x, height - margin);

    cairo_move_to(cr, margin, height - margin - scale_y);
    cairo_line_to(cr, width - margin, height - margin - scale_y);
    cairo_stroke(cr);

    cairo_set_font_size(cr, 12);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);

    for (double x = -1.0; x <= 1.0; x += 0.5) {
        char label[10];
        snprintf(label, sizeof(label), "%.1f", x);
        double screen_x = margin + (x + 1.0) * scale_x;
        cairo_move_to(cr, screen_x - 10, height - margin + 20);
        cairo_show_text(cr, label);
    }

    for (double y = -0.75; y <= 1.0; y += 0.25) {
        char label[10];
        snprintf(label, sizeof(label), "%.2f", y);
        double screen_y = height - margin - (y + 1.0) * scale_y;
        cairo_move_to(cr, margin - 35, screen_y + 5);
        cairo_show_text(cr, label);
    }

    double max_x = 0, max_y = 0;
    for (int i = 0; i < global_data_set.num_data_points; i++) {
        if (abs(global_data_set.data_points[i].x) > max_x) 
            max_x = abs(global_data_set.data_points[i].x);
        if (abs(global_data_set.data_points[i].y) > max_y)
            max_y = abs(global_data_set.data_points[i].y);
    }
    for (int i = 0; i < global_data_set.num_clusters; i++) {
        if (abs(global_data_set.centroids[i].x) > max_x)
            max_x = abs(global_data_set.centroids[i].x);
        if (abs(global_data_set.centroids[i].y) > max_y)
            max_y = abs(global_data_set.centroids[i].y);
    }

    cairo_set_source_rgb(cr, 0, 0, 1);
    for (int i = 0; i < global_data_set.num_data_points; i++) {
        double normalized_x = global_data_set.data_points[i].x / max_x;
        double normalized_y = global_data_set.data_points[i].y / max_y;
        
        double screen_x = margin + (normalized_x + 1.0) * scale_x;
        double screen_y = height - margin - (normalized_y + 1.0) * scale_y;
        
        cairo_arc(cr, screen_x, screen_y, 3, 0, 2 * M_PI);
        cairo_fill(cr);
    }

    cairo_set_source_rgb(cr, 1, 0, 0);
    for (int i = 0; i < global_data_set.num_clusters; i++) {
        double normalized_x = global_data_set.centroids[i].x / max_x;
        double normalized_y = global_data_set.centroids[i].y / max_y;
        
        double screen_x = margin + (normalized_x + 1.0) * scale_x;
        double screen_y = height - margin - (normalized_y + 1.0) * scale_y;
        
        cairo_rectangle(cr, screen_x - 4, screen_y - 4, 8, 8);
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