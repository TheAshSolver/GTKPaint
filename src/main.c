#include<gtk/gtk.h>


static cairo_surface_t *surface = NULL;
static double brush_size = 6.0;
static double red = 1.0;
static double blue = 0;
static double green  = 0;
static void clear_surface(void) {
    cairo_t *c = cairo_create(surface);

    cairo_set_source_rgb(c, 0,0,0);
    cairo_paint(c);
    cairo_destroy(c);




}

static void eraser(GtkGesture *gesture, int press, double x, double y, GtkWidget *colorbutton){
    red = 0;
    green = 0;
    blue = 0;
    gtk_color_dialog_button_set_rgba(GTK_COLOR_DIALOG_BUTTON(colorbutton), &(GdkRGBA){0,0,0,1});
}

static void on_color_change(GtkColorDialogButton *button, gpointer data){
const GdkRGBA *rgba;
rgba = gtk_color_dialog_button_get_rgba(button);
red = rgba->red;
blue = rgba->blue;
green = rgba->green;

}


static void on_brush_width_change(GtkRange *range, gpointer data){
    brush_size = gtk_range_get_value(range);
}

static void print_hi(GtkWidget *widget, gpointer data){
    g_print("Hello world");
}


static void resize_cb(GtkWidget *widget, int width, int height, gpointer data) {
    if(surface) {
        cairo_surface_destroy(surface);
        surface = NULL;
    }
    if(gtk_native_get_surface(gtk_widget_get_native(widget))) {
        surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, gtk_widget_get_width(widget), gtk_widget_get_height(widget));
        clear_surface();
    }



}


static void draw_cb(GtkDrawingArea *area, cairo_t *cairo, int width, int height, gpointer data) {
    cairo_set_source_surface(cairo, surface, 0,0);
    cairo_paint(cairo);



}




static void draw_brush(GtkWidget *widget, double x, double y, double end_x, double end_y) {
    cairo_t *cairo = cairo_create(surface);
    cairo_set_line_width(cairo, brush_size);
    cairo_set_line_cap(cairo,  CAIRO_LINE_CAP_ROUND);
    cairo_set_antialias(cairo, CAIRO_ANTIALIAS_BEST);
    cairo_set_source_rgb(cairo, red,green, blue);
    cairo_move_to(cairo, x,y);
    cairo_line_to(cairo, end_x, end_y);
    cairo_stroke(cairo);

    cairo_destroy(cairo);
    gtk_widget_queue_draw(widget);



}


static double start_x, start_y, initial_x, initial_y;

static void drag_begin(GtkGestureDrag *gesture, double x, double y, GtkWidget *area ) {
    start_x = x;
    start_y = y;
    initial_x = x;
    initial_y = y;





}


static void drag_update(GtkGestureDrag *gesture, double x, double y, GtkWidget *area) {
    double current_x = initial_x+x;
    double current_y = initial_y +y;
    draw_brush(area, start_x, start_y, current_x, current_y);
   start_x = current_x;
   start_y = current_y;
}


static void drag_end(GtkGestureDrag *gesture, double x, double y, GtkWidget *area) {
    draw_brush(area, start_x, start_y, initial_x+x, initial_y+y);

}


static void pressed(GtkWidget *click, gpointer data) {
    GtkWidget *area = GTK_WIDGET(data);
    clear_surface();
    gtk_widget_queue_draw(area);
}


static void close_window(void) {
    if(surface) {
        cairo_surface_destroy(surface);
    }

}


static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *drawing_area;
    GtkWidget *frame;
    GtkGesture *drag;
    GtkGesture *press;
    GtkWidget *header;
    GtkWidget *button;
    GtkWidget *slider;
    GtkWidget *color_picker_dialog;
    GtkWidget *color_picker_button;



    header = gtk_header_bar_new();
    button = gtk_button_new_from_icon_name("document-open-symbolic");
    gtk_widget_set_tooltip_text(button, "clear");
   
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header), button);
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Draw");
    gtk_window_set_titlebar(GTK_WINDOW(window), header);
    

    slider = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 1, 50, 2);
    g_signal_connect(slider, "value-changed", G_CALLBACK(on_brush_width_change), NULL);
    GtkWidget *text = gtk_label_new("Increase brush size");
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header), text);
    gtk_widget_set_size_request(slider, 100, 25);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header), slider);

    color_picker_button = gtk_color_dialog_button_new(gtk_color_dialog_new());
    g_signal_connect(color_picker_button, "notify::rgba", G_CALLBACK(on_color_change), NULL);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header), color_picker_button);



    drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 100, 100);
    frame = gtk_frame_new(NULL);
    gtk_frame_set_child(GTK_FRAME(frame), drawing_area);
    gtk_window_set_child(GTK_WINDOW(window), frame);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(drawing_area), draw_cb, NULL, NULL);
    g_signal_connect_after(drawing_area, "resize", G_CALLBACK(resize_cb), drawing_area);
    g_signal_connect(button, "clicked", G_CALLBACK(pressed), drawing_area);
    drag = gtk_gesture_drag_new();
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(drag), GDK_BUTTON_PRIMARY);
    gtk_widget_add_controller(drawing_area, GTK_EVENT_CONTROLLER(drag));
    g_signal_connect(drag, "drag-begin", G_CALLBACK(drag_begin), drawing_area);
    g_signal_connect(drag, "drag-update",G_CALLBACK(drag_update), drawing_area);
    g_signal_connect(drag, "drag-end", G_CALLBACK(drag_end), drawing_area);

    press = gtk_gesture_click_new();
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(press), GDK_BUTTON_SECONDARY);
    gtk_widget_add_controller(drawing_area, GTK_EVENT_CONTROLLER(press));
    g_signal_connect(press, "pressed", G_CALLBACK(eraser), color_picker_button);






    gtk_window_present(GTK_WINDOW(window));

}






int main(int argc, char *argv[]) {
    GtkApplication *app;
    app = gtk_application_new("com.drawing", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);

    g_object_unref(app);

    return status;








    return 0;
}
