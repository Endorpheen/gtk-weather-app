#include <gtk/gtk.h>
#include <curl/curl.h>
#include <string.h>
#include <json-glib/json-glib.h>
#include <stdio.h>

#define MAX_FAVORITES 10
#define FAVORITES_FILE "favorites.txt"

struct MemoryStruct {
    char *memory;
    size_t size;
};

typedef struct {
    char cities[MAX_FAVORITES][256];
    int count;
} Favorites;

static Favorites favorites;
static GtkWidget *weather_label;

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
    printf("fetch_weather called\n");
    GtkEntry *city_entry = GTK_ENTRY(g_object_get_data(G_OBJECT(widget), "city_entry"));
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
        snprintf(url, sizeof(url), "http://api.openweathermap.org/data/2.5/weather?q=%s&appid=Your_API_KEY&units=metric", city);
        
        printf("URL: %s\n", url);

        curl_easy_setopt(curl_handle, CURLOPT_URL, url);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        res = curl_easy_perform(curl_handle);

        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            if (GTK_IS_LABEL(weather_label)) {
                gtk_label_set_text(GTK_LABEL(weather_label), "Ошибка при получении погоды");
                while (g_main_context_pending(NULL)) {
                    g_main_context_iteration(NULL, FALSE);
                }
            }
        } else {
            printf("Data received:\n%s\n", chunk.memory);

            JsonParser *parser = json_parser_new();
            GError *error = NULL;
            if (!json_parser_load_from_data(parser, chunk.memory, -1, &error)) {
                g_warning("Error parsing JSON: %s", error->message);
                g_error_free(error);
                if (GTK_IS_LABEL(weather_label)) {
                    gtk_label_set_text(GTK_LABEL(weather_label), "Ошибка при разборе JSON");
                    while (g_main_context_pending(NULL)) {
                        g_main_context_iteration(NULL, FALSE);
                    }
                }
            } else {
                JsonNode *root = json_parser_get_root(parser);
                JsonObject *object = json_node_get_object(root);
                if (object) {
                    JsonObject *main = json_object_get_object_member(object, "main");
                    double temp = json_object_get_double_member(main, "temp");
                    JsonArray *weather_array = json_object_get_array_member(object, "weather");
                    JsonObject *weather = json_array_get_object_element(weather_array, 0);
                    const char *description = json_object_get_string_member(weather, "description");
                    char weather_text[256];
                    snprintf(weather_text, sizeof(weather_text), "Погода в %s: %.1f°C, %s", city, temp, description);
                    printf("Weather text: %s\n", weather_text);
                    if (GTK_IS_LABEL(weather_label)) {
                        gtk_label_set_text(GTK_LABEL(weather_label), weather_text);
                        while (g_main_context_pending(NULL)) {
                            g_main_context_iteration(NULL, FALSE);
                        }
                    }
                } else {
                    if (GTK_IS_LABEL(weather_label)) {
                        gtk_label_set_text(GTK_LABEL(weather_label), "Ошибка: некорректный JSON");
                        while (g_main_context_pending(NULL)) {
                            g_main_context_iteration(NULL, FALSE);
                        }
                    }
                }
            }
            g_object_unref(parser);
        }

        curl_easy_cleanup(curl_handle);
        free(chunk.memory);
    }

    curl_global_cleanup();
}

static void save_favorites() {
    FILE *file = fopen(FAVORITES_FILE, "w");
    if (file) {
        for (int i = 0; i < favorites.count; i++) {
            fprintf(file, "%s\n", favorites.cities[i]);
        }
        fclose(file);
    }
}

static void load_favorites() {
    FILE *file = fopen(FAVORITES_FILE, "r");
    if (file) {
        favorites.count = 0;
        while (fgets(favorites.cities[favorites.count], sizeof(favorites.cities[0]), file) && favorites.count < MAX_FAVORITES) {
            favorites.cities[favorites.count][strcspn(favorites.cities[favorites.count], "\n")] = 0;
            favorites.count++;
        }
        fclose(file);
    }
}

static void add_favorite(GtkWidget *widget, gpointer user_data) {
    GtkEntry *city_entry = GTK_ENTRY(user_data);
    const gchar *city = gtk_entry_get_text(city_entry);
    
    if (favorites.count < MAX_FAVORITES && strlen(city) > 0) {
        strncpy(favorites.cities[favorites.count], city, sizeof(favorites.cities[0]) - 1);
        favorites.count++;
        save_favorites();
        
        GtkComboBoxText *combo = GTK_COMBO_BOX_TEXT(g_object_get_data(G_OBJECT(widget), "favorites_combo"));
        gtk_combo_box_text_append_text(combo, city);
    }
}

static void remove_favorite(GtkWidget *widget, gpointer user_data) {
    GtkComboBoxText *combo = GTK_COMBO_BOX_TEXT(user_data);
    gchar *selected_city = gtk_combo_box_text_get_active_text(combo);
    
    if (selected_city) {
        for (int i = 0; i < favorites.count; i++) {
            if (strcmp(favorites.cities[i], selected_city) == 0) {
                for (int j = i; j < favorites.count - 1; j++) {
                    strcpy(favorites.cities[j], favorites.cities[j+1]);
                }
                favorites.count--;
                save_favorites();
                
                gtk_combo_box_text_remove_all(combo);
                for (int k = 0; k < favorites.count; k++) {
                    gtk_combo_box_text_append_text(combo, favorites.cities[k]);
                }
                break;
            }
        }
        g_free(selected_city);
    }
}

static void on_favorite_selected(GtkComboBox *widget, gpointer user_data) {
    gchar *selected_city = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));
    if (selected_city) {
        GtkEntry *city_entry = GTK_ENTRY(user_data);
        gtk_entry_set_text(city_entry, selected_city);
        g_free(selected_city);
    }
}

static void activate(GtkApplication* app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *button;
    GtkWidget *label;
    GtkWidget *city_entry;
    GtkWidget *favorites_combo;
    GtkWidget *add_button;
    GtkWidget *remove_button;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Погодное приложение");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);

    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    label = gtk_label_new("Введите название города:");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 2, 1);

    city_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(city_entry), "Например: Москва");
    gtk_grid_attach(GTK_GRID(grid), city_entry, 0, 1, 2, 1);

    favorites_combo = gtk_combo_box_text_new();
    gtk_grid_attach(GTK_GRID(grid), favorites_combo, 0, 2, 2, 1);

    add_button = gtk_button_new_with_label("Добавить в избранное");
    gtk_grid_attach(GTK_GRID(grid), add_button, 0, 3, 1, 1);
    gtk_widget_set_hexpand(add_button, TRUE);
    gtk_widget_set_vexpand(add_button, TRUE);
    gtk_widget_set_halign(add_button, GTK_ALIGN_FILL);
    gtk_widget_set_valign(add_button, GTK_ALIGN_FILL);

    remove_button = gtk_button_new_with_label("Удалить из избранного");
    gtk_grid_attach(GTK_GRID(grid), remove_button, 1, 3, 1, 1);
    gtk_widget_set_hexpand(remove_button, TRUE);
    gtk_widget_set_vexpand(remove_button, TRUE);
    gtk_widget_set_halign(remove_button, GTK_ALIGN_FILL);
    gtk_widget_set_valign(remove_button, GTK_ALIGN_FILL);

    button = gtk_button_new_with_label("Узнать погоду");
    g_object_set_data(G_OBJECT(button), "city_entry", city_entry);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 4, 2, 1);
    gtk_widget_set_hexpand(button, TRUE);
    gtk_widget_set_vexpand(button, TRUE);
    gtk_widget_set_halign(button, GTK_ALIGN_FILL);
    gtk_widget_set_valign(button, GTK_ALIGN_FILL);

    weather_label = gtk_label_new("Здесь появится информация о погоде");
    gtk_grid_attach(GTK_GRID(grid), weather_label, 0, 5, 2, 1);
    gtk_widget_set_hexpand(weather_label, TRUE);
    gtk_widget_set_vexpand(weather_label, TRUE);
    gtk_widget_set_halign(weather_label, GTK_ALIGN_FILL);
    gtk_widget_set_valign(weather_label, GTK_ALIGN_FILL);

    g_signal_connect(button, "clicked", G_CALLBACK(fetch_weather), NULL);
    g_signal_connect(add_button, "clicked", G_CALLBACK(add_favorite), city_entry);
    g_object_set_data(G_OBJECT(add_button), "favorites_combo", favorites_combo);
    g_signal_connect(remove_button, "clicked", G_CALLBACK(remove_favorite), favorites_combo);
    g_signal_connect(favorites_combo, "changed", G_CALLBACK(on_favorite_selected), city_entry);

    load_favorites();
    for (int i = 0; i < favorites.count; i++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(favorites_combo), favorites.cities[i]);
    }

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