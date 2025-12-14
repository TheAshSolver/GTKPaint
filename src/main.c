#include<gtk/gtk.h>
#include<string.h>
#include<stdio.h>
static cairo_surface_t *surface = NULL;
static GdkRGBA primary_colors ={.red = 1.0,.blue = 0.0,.green =  0.0, .alpha = 1.0};
static double brush_size = 6.0;
static GdkRGBA secondary_colors ={.red = 0.0, .blue=0.0, .green =0.0, .alpha=1.0};
static GdkRGBA background_color={.red=0.0, .blue=0.0, .green=0.0, .alpha=1.0};

static void clear_surface(void) {
    cairo_t *c = cairo_create(surface);

    cairo_set_source_rgb(c, background_color.red, background_color.green, background_color.blue);
    cairo_paint(c);
    cairo_destroy(c);
    



}

typedef struct TextWindow{
    GtkWidget *window;
    GtkEntryBuffer *buffer;
    GtkLabel *label;
    GtkEntryBuffer *width_buffer;
    GtkEntryBuffer *height_buffer;
}TextWindow;


static void interchange_colors(GtkGesture *gesture, int press, double x, double y, gpointer data){
GtkWidget **colorpickers = (GtkWidget **)data;
GdkRGBA temp = {.red = secondary_colors.red, .green = secondary_colors.green, .blue = secondary_colors.blue, .alpha = 1.0};
secondary_colors = (GdkRGBA){.red = primary_colors.red, .green = primary_colors.green, .blue = primary_colors.blue, .alpha= 1.0};
primary_colors = (GdkRGBA){.red = temp.red ,.green = temp.green, .blue = temp.blue, .alpha = 1.01};
    gtk_color_dialog_button_set_rgba(GTK_COLOR_DIALOG_BUTTON(colorpickers[0]), &primary_colors);
    gtk_color_dialog_button_set_rgba(GTK_COLOR_DIALOG_BUTTON(colorpickers[1]), &secondary_colors);
}

static void on_color_change(GtkColorDialogButton *button, gpointer data){
const GdkRGBA *rgba;
rgba = gtk_color_dialog_button_get_rgba(button);
primary_colors.red = rgba->red;
primary_colors.blue = rgba->blue;
primary_colors.green = rgba->green;
}

static void on_secondary_color_change(GtkColorDialogButton *button, gpointer data){
    const GdkRGBA *rgba;
    rgba = gtk_color_dialog_button_get_rgba(button);
    secondary_colors.red = rgba->red;
    secondary_colors.blue = rgba->blue;
    secondary_colors.green= rgba->green;
}

static void on_background_color(GtkColorDialogButton *button, gpointer data){
    if(!surface){
        return;
    }
    GtkWidget *bg = (GtkWidget *)data;
    const GdkRGBA *rgba = gtk_color_dialog_button_get_rgba(button);
    background_color.red = rgba->red;
    background_color.blue = rgba->blue;
    background_color.green = rgba->green;
    cairo_t *cr = cairo_create(surface);
    cairo_set_source_rgb(cr, background_color.red, background_color.green, background_color.blue);
    cairo_set_operator(cr, CAIRO_OPERATOR_DEST_OVER);
    cairo_paint(cr);
    cairo_destroy(cr);
    gtk_widget_queue_draw(bg);

}

static void on_brush_width_change(GtkRange *range, gpointer data){
    brush_size = gtk_range_get_value(range);
}

static void print_hi(GtkWidget *widget, gpointer data){
    g_print("Hello world");
}


static void resize_cb(GtkWidget *widget, int width, int height, gpointer data) {
    if(surface) {
        /*clear_surface();
        cairo_surface_destroy(surface);
        surface = NULL;*/
        int original_width = cairo_image_surface_get_width(surface);
        int original_height = cairo_image_surface_get_height(surface);
        int new_width, new_height;
        new_width= original_width>width?original_width:width;
        new_height = original_height>height?original_height:height;
        if(new_width>original_width || new_height>original_height){
        cairo_surface_t *new = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, new_width, new_height);
        cairo_surface_t *prev = surface;
        cairo_t *cr = cairo_create(new);
        cairo_set_source_rgb(cr, background_color.red, background_color.green, background_color.blue);
        cairo_paint(cr);
        cairo_set_source_surface(cr, prev,0,0);
        cairo_paint(cr);
        cairo_destroy(cr);
        surface = new;
        cairo_surface_destroy(prev);
        gtk_widget_set_size_request(widget, new_width, new_height);
        }
        

    }
    if(!surface&&gtk_native_get_surface(gtk_widget_get_native(widget))) {
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
    cairo_set_source_rgb(cairo, primary_colors.red,primary_colors.green, primary_colors.blue);
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
static void save_image(char *file_name, int height, int width){
    if(!surface){
        g_print("Error");
        return;
    }
    cairo_surface_t *target_surface = cairo_image_surface_create(cairo_image_surface_get_format(surface), width, height);
    if(!target_surface){
        g_print("Error. Target surface not created");
        return;
    }
    cairo_t *cr= cairo_create(target_surface);
    if(!cr){
        g_print("Error in creating error context");
        return;
    }
    int original_width = cairo_image_surface_get_width(surface);
    int original_height = cairo_image_surface_get_height(surface);
    if(height>original_height||width>original_width){
        g_print("not possible");
        return;
    }
    cairo_scale(cr, (float)width/original_width, (float)height/original_height);
    cairo_set_source_surface(cr, surface, 0,0);
    cairo_paint(cr);
    cairo_status_t status = cairo_surface_write_to_png(target_surface, file_name);
    g_print("%d", status);


    cairo_destroy(cr);
    cairo_surface_destroy(target_surface);


}
static void on_save_button(GtkWidget *widget, gpointer data){
    TextWindow *a = (TextWindow *)data;
    char file_name[600];
    strcpy(file_name, gtk_entry_buffer_get_text( a->buffer));
    strcat(file_name, ".png");
   g_autofree char *path = g_build_filename(gtk_label_get_text(a->label),file_name, NULL);
    GtkWidget *window = a->window;
  char *given_width = gtk_entry_buffer_get_text(a->width_buffer);
  char *given_height = gtk_entry_buffer_get_text( a->height_buffer);
  int give_height, give_width;
  sscanf(given_height, "%d", &give_height);
  sscanf(given_width, "%d", &give_width);
    if(surface){
   save_image(path,give_height, give_width );
    }
    gtk_window_close(GTK_WINDOW(window));
}
static void on_close_button(GtkWidget *widget, gpointer data){
    GtkWidget *window = (GtkWidget*) data;
    gtk_window_close(GTK_WINDOW(window));
}

static void folder_selected(GtkNativeDialog *dialog, int response, gpointer data){
    GtkWidget *label = (GtkWidget *) data;
    if(response ==GTK_RESPONSE_ACCEPT){
        gtk_label_set_text(GTK_LABEL(label),g_file_get_path( gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog))));

    }
    else{
        gtk_label_set_text(GTK_LABEL(label), "Select your folder");
    }
    g_object_unref(dialog);
}

static void on_folder_choose(GtkWidget *button, gpointer data){
    //use data to access label and implement folder name                                                `                                                                                                                                               
    GtkWindow *parent_window = GTK_WINDOW(gtk_widget_get_root(button));
    GtkWidget *label = (GtkWidget *)data;

    GtkNativeDialog *dialog= GTK_NATIVE_DIALOG(gtk_file_chooser_native_new("Select folder", parent_window, GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, "Select", "Cancel"));

    g_autofree char *home_path = g_get_home_dir();
    g_autoptr(GFile) dir = g_file_new_for_path(home_path);
    if(dir){
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),dir, NULL);
    }

    g_signal_connect(dialog, "response", G_CALLBACK(folder_selected), label);

    gtk_native_dialog_show(dialog);




}



static void on_save_click(GtkWidget *button, gpointer user_data){
    GtkEntryBuffer *buffer = gtk_entry_buffer_new("filename", 400);
    GtkEntryBuffer *width_buffer = gtk_entry_buffer_new("", 400);
    char original_height[300], original_width[300];
    sprintf(original_height, "%d", cairo_image_surface_get_height(surface));
    sprintf(original_width, "%d", cairo_image_surface_get_width(surface));
    GtkEntryBuffer *height_buffer = gtk_entry_buffer_new("", 400);
    gtk_entry_buffer_set_text(height_buffer, original_height, strlen(original_height));
    gtk_entry_buffer_set_text(width_buffer, original_width, strlen(original_width));

    GtkWidget *parent_window = (GtkWidget*) user_data;
    GtkWidget *dialog_window = gtk_window_new();
    gtk_window_set_transient_for(GTK_WINDOW(dialog_window), GTK_WINDOW(parent_window));
    gtk_window_set_modal(GTK_WINDOW(dialog_window), true);
    gtk_window_set_title(GTK_WINDOW(dialog_window), "Document save");
    GtkWidget *parent_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_window_set_child(GTK_WINDOW(dialog_window), parent_box);
    GtkWidget *label = gtk_label_new("Do you want to save your file as png?");
    gtk_box_append(GTK_BOX(parent_box), label);
    label = gtk_label_new("Enter file name");
    GtkWidget *child_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_box_append(GTK_BOX(child_box), label);
    GtkWidget *textbox = gtk_entry_new_with_buffer(buffer);
    gtk_box_append(GTK_BOX(child_box), textbox);
    gtk_box_append(GTK_BOX(parent_box), child_box);
    GtkWidget *folder_chooser = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    label = gtk_label_new("Choose the directory");
    gtk_box_append(GTK_BOX(folder_chooser), label);
    GtkWidget *folder_button = gtk_button_new_with_label("Select Folder");
    g_signal_connect(folder_button, "clicked", G_CALLBACK(on_folder_choose), label);
    gtk_box_append(GTK_BOX(folder_chooser), folder_button);
    gtk_box_append(GTK_BOX(parent_box), folder_chooser);
    GtkWidget *width_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *height_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_append(GTK_BOX(parent_box), width_box);
    gtk_box_append(GTK_BOX(parent_box), height_box);

    GtkWidget *width_label = gtk_label_new("Enter your width");
    gtk_box_append(GTK_BOX(width_box), width_label);
    GtkWidget *width_entry = gtk_entry_new();
    gtk_entry_set_buffer(GTK_ENTRY(width_entry), width_buffer);
    gtk_box_append(GTK_BOX(width_box), width_entry);

    GtkWidget *height_label = gtk_label_new("Enter your height");
    gtk_box_append(GTK_BOX(height_box), height_label);
    GtkWidget *height_entry = gtk_entry_new();
    gtk_entry_set_buffer(GTK_ENTRY(height_entry), height_buffer);
    gtk_box_append(GTK_BOX(height_box), height_entry);




    


    GtkWidget *actionbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_append(GTK_BOX(parent_box), actionbar);
    TextWindow *a = g_new(TextWindow, 1);
    a->buffer = buffer;
    a->window = dialog_window;
    a->label = GTK_LABEL(label);
    a->height_buffer = height_buffer;
    a->width_buffer = width_buffer;
    GtkWidget *savebutton = gtk_button_new_with_label("Save");
    g_signal_connect(savebutton, "clicked", G_CALLBACK(on_save_button),a );
    GtkWidget *closebutton = gtk_button_new_with_label("Close");
    g_signal_connect(closebutton, "clicked", G_CALLBACK(on_close_button), dialog_window);
    gtk_box_append(GTK_BOX(actionbar), savebutton);
    gtk_box_append(GTK_BOX(actionbar), closebutton);

    gtk_window_present(GTK_WINDOW(dialog_window));



    

    

    




}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *drawing_area;
    GtkWidget *scrolled_window;
    GtkWidget *frame;
    GtkWidget *background_color_picker;
    GtkGesture *drag;
    GtkGesture *press;
    GtkWidget *header;
    GtkWidget *button;
    GtkWidget *slider;
    GtkWidget *color_picker_dialog;
    GtkWidget *color_picker_button;
    GtkWidget *color_picker2;
    GtkWidget *save_button;
    

    scrolled_window = gtk_scrolled_window_new();
    gtk_scrolled_window_set_has_frame(GTK_SCROLLED_WINDOW(scrolled_window), true);
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
    color_picker2 = gtk_color_dialog_button_new(gtk_color_dialog_new());
    color_picker_button = gtk_color_dialog_button_new(gtk_color_dialog_new());
    color_picker2 = gtk_color_dialog_button_new(gtk_color_dialog_new());
    gtk_color_dialog_button_set_rgba(GTK_COLOR_DIALOG_BUTTON(color_picker2), &secondary_colors);
    g_signal_connect(color_picker2, "notify::rgba", G_CALLBACK(on_secondary_color_change), NULL);
    g_signal_connect(color_picker_button, "notify::rgba", G_CALLBACK(on_color_change), NULL);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header), color_picker_button);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header), color_picker2);
    save_button = gtk_button_new_from_icon_name("document-generic");
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), save_button);
    g_signal_connect(save_button, "clicked", G_CALLBACK(on_save_click), window);
    
   
   

    drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 100, 100);
    background_color_picker = gtk_color_dialog_button_new(gtk_color_dialog_new());
    gtk_color_dialog_button_set_rgba(GTK_COLOR_DIALOG_BUTTON(background_color_picker), &background_color);
    g_signal_connect(background_color_picker, "notify::rgba", G_CALLBACK(on_background_color), drawing_area);
     gtk_header_bar_pack_end(GTK_HEADER_BAR(header), background_color_picker);
    //frame = gtk_frame_new(NULL);
    //gtk_frame_set_child(GTK_FRAME(frame), drawing_area);
    gtk_window_set_child(GTK_WINDOW(window), scrolled_window);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), drawing_area);
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
    static GtkWidget *pickers[2];
    pickers[0]= color_picker_button;
    pickers[1] = color_picker2;
    g_signal_connect(press, "pressed", G_CALLBACK(interchange_colors), pickers);






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
