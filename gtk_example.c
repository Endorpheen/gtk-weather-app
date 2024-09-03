#include <gtk/gtk.h>
#include <curl/curl.h>
#include <string.h>
#include <json-glib/json-glib.h>

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(!ptr) {
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

static void fetch_weather(GtkWidget *widget, gpointer user_data) {
    GtkEntry *city_entry = GTK_ENTRY(g_object_get_data(G_OBJECT(widget), "city_entry"));
    GtkLabel *weather_label = GTK_LABEL(g_object_get_data(G_OBJECT(widget), "weather_label"));
    const gchar *city = gtk_entry_get_text(city_entry);

    CURL *curl_handle;
    CURLcode res;

    struct MemoryStruct chunk;
    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();

    if(curl_handle) {
        char url[256];
        snprintf(url, sizeof(url), "http://api.openweathermap.org/data/2.5/weather?q=%s&appid=f3db7af947c787c1151391d20f19ccbb&units=metric", city);
        
        curl_easy_setopt(curl_handle, CURLOPT_URL, url);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        res = curl_easy_perform(curl_handle);

        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            gtk_label_set_text(weather_label, "Ошибка при получении погоды");
        } else {
            JsonParser *parser = json_parser_new();
            if (json_parser_load_from_data(parser, chunk.memory, -1, NULL)) {
                JsonNode *root = json_parser_get_root(parser);
                JsonObject *object = json_node_get_object(root);
                JsonObject *main = json_object_get_object_member(object, "main");
                double temp = json_object_get_double_member(main, "temp");
                JsonArray *weather_array = json_object_get_array_member(object, "weather");
                JsonObject *weather = json_array_get_object_element(weather_array, 0);
                const char *description = json_object_get_string_member(weather, "description");
                char weather_text[256];
                snprintf(weather_text, sizeof(weather_text), "Погода в %s: %.1f°C, %s", city, temp, description);
                gtk_label_set_text(weather_label, weather_text);
            } else {
                gtk_label_set_text(weather_label, "Ошибка при разборе JSON");
            }
            g_object_unref(parser);
        }

        curl_easy_cleanup(curl_handle);
        free(chunk.memory);
    }

    curl_global_cleanup();
}

static void show_about_dialog(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_about_dialog_new();
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "Погодное приложение");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "1.1");
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), "GTK приложение для просмотра погоды");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static void activate(GtkApplication* app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *button;
    GtkWidget *label;
    GtkWidget *city_entry;
    GtkWidget *menubar;
    GtkWidget *fileitem;
    GtkWidget *filemenu;
    GtkWidget *quititem;
    GtkWidget *helpitem;
    GtkWidget *helpmenu;
    GtkWidget *aboutitem;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Погодное приложение");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);

    // Обновленный CSS с более сложными стилями
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "window { background: linear-gradient(to bottom, #2C3E50, #3498DB); }"
        "button { background-color: #E74C3C; color: white; font-weight: bold; padding: 10px; margin: 5px; border-radius: 5px; transition: all 0.3s; }"
        "button:hover { background-color: #C0392B; transform: scale(1.05); }"
        "label { color: #ECF0F1; font-size: 16px; margin: 10px; text-shadow: 1px 1px 2px #2C3E50; }"
        "entry { background-color: #ECF0F1; color: #2C3E50; padding: 5px; margin: 5px; border-radius: 3px; }"
        "menubar { background-color: rgba(52, 73, 94, 0.8); }"
        "menuitem { color: #ECF0F1; padding: 5px 10px; }"
        "menuitem:hover { background-color: #2980B9; }",
        -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    menubar = gtk_menu_bar_new();
    fileitem = gtk_menu_item_new_with_label("Файл");
    filemenu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(fileitem), filemenu);
    quititem = gtk_menu_item_new_with_label("Выход");
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), quititem);
    g_signal_connect(quititem, "activate", G_CALLBACK(gtk_main_quit), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), fileitem);

    helpitem = gtk_menu_item_new_with_label("Помощь");
    helpmenu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(helpitem), helpmenu);
    aboutitem = gtk_menu_item_new_with_label("О программе");
    gtk_menu_shell_append(GTK_MENU_SHELL(helpmenu), aboutitem);
    g_signal_connect(aboutitem, "activate", G_CALLBACK(show_about_dialog), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), helpitem);

    gtk_grid_attach(GTK_GRID(grid), menubar, 0, 0, 2, 1);

    label = gtk_label_new("Введите название города:");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 2, 1);

    city_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(city_entry), "Например: Москва");
    gtk_grid_attach(GTK_GRID(grid), city_entry, 0, 2, 2, 1);

    button = gtk_button_new_with_label("Узнать погоду");
    g_object_set_data(G_OBJECT(button), "city_entry", city_entry);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 3, 2, 1);

    label = gtk_label_new("Здесь появится информация о погоде");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 4, 2, 1);
    g_object_set_data(G_OBJECT(button), "weather_label", label);

    g_signal_connect(button, "clicked", G_CALLBACK(fetch_weather), NULL);

    gtk_widget_show_all(window);
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    app = gtk_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
