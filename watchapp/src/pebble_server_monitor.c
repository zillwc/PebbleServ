/**
  PebbleServ
  Frontend view for the watch
*/

#include <pebble.h>

#define DEFAULT_HOST "DEFAULT_HOST"
#define DEFAULT_BAR_VAL 40
#define DEFAULT_UPDATE_DELAY  1000       //milliseconds
#define DEFAULT_UPDATE_TIMEOUT  30000    //GET request timeout - 30 seconds
#define DEFAULT_UPDATE_OFFSET 500 
#define BOOT_COMMS_DELAY  3000
#define PBAR_HEIGHT 18
#define PBAR_WIDTH 130

typedef Layer ProgressBarLayer;

static Window *window;
static InverterLayer *iLayer;
static TextLayer *hostname_text;
static TextLayer *cpu_usage_text;
static TextLayer *mem_usage_text;
static TextLayer *ip_text;
static TextLayer *port_text;
static TextLayer *debug_text;
static ProgressBarLayer *progress_bar_cpu;
static ProgressBarLayer *progress_bar_mem;
static AppTimer *refresh_timer;               //main data refresh timer
static AppTimer *variable_offset_timer;       //used to delay update of multiple variables

static char host[26];
static char cpu[10];
static char mem[10];
static char ip[26];
static char port[16];

static char logBuff[24];

static int auto_update = 0;
static int update_interval = 1000;
static int auto_update_prev = 0;


enum { 
    SERVER_KEY_FETCH = 0x0,
    SERVER_KEY_IP = 0x1,
    SERVER_KEY_CPU = 0x2,
    SERVER_KEY_MEM = 0x3,
    SERVER_KEY_HOST = 0x4,
    SERVER_KEY_AUTO = 0x5,
    SERVER_KEY_UPDATE_INT = 0x6,
    SERVER_KEY_ALL = 0x7,
    SERVER_KEY_PORT = 0x8,
};

typedef struct {
    unsigned int progress; // how full the bar is
} ProgressData;

static void get_all(void *data);

static void fetch_msg(unsigned int server_key) {
    Tuplet server_tuple = TupletInteger(server_key, 1);
    
    char logtemp[15];
    snprintf(logtemp, 15, "FETCH_MSG... %d", server_key);

    APP_LOG(APP_LOG_LEVEL_DEBUG, logtemp);
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    if (iter == NULL) {
        return;
    }

    dict_write_tuplet(iter, &server_tuple);
    dict_write_end(iter);

    app_message_outbox_send();
}

static void get_cpu(void) {
    fetch_msg(SERVER_KEY_CPU);
    text_layer_set_text(debug_text, "CPU");
}

static void get_mem(void) {
    fetch_msg(SERVER_KEY_MEM);
    text_layer_set_text(debug_text, "MEM");
} 

static void get_all(void *data) {
    fetch_msg(SERVER_KEY_ALL);
    if (auto_update == 1) {
        refresh_timer = app_timer_register(update_interval, get_all, NULL);  //timeout added, update_refresh reset to DEFUALT_UPDATE_DELAY after reply received - will default to TIMEOUT if no reply received. Prevents new GET requests while waiting for previous ones.
    }
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
    //refresh usage
    get_all(NULL);
    text_layer_set_text(debug_text, "Refreshing...");
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
    fetch_msg(SERVER_KEY_IP);
    text_layer_set_text(debug_text, "IP");
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
    fetch_msg(SERVER_KEY_FETCH);              //fetch settings - ip, hostname, updateinterval etc...
    text_layer_set_text(debug_text, "Fetch settings");
}

static void click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
    window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void updateBarData(ProgressBarLayer *bar, int newVal) {
    ProgressData *data = layer_get_data(bar);
    
    //percentage needs to be converted into pixels
    snprintf(logBuff, 20, "newVal: %d", newVal);
    APP_LOG(APP_LOG_LEVEL_DEBUG, logBuff);
    int bar_width = ((float)newVal/100) * PBAR_WIDTH;
    snprintf(logBuff, 20, "bar_width: %d", bar_width);
    APP_LOG(APP_LOG_LEVEL_DEBUG, logBuff);
    data->progress = bar_width;
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
    Tuple *ip_tuple = dict_find(iter, SERVER_KEY_IP);
    Tuple *port_tuple = dict_find(iter, SERVER_KEY_PORT);
    Tuple *cpu_tuple = dict_find(iter, SERVER_KEY_CPU);
    Tuple *mem_tuple = dict_find(iter, SERVER_KEY_MEM);
    Tuple *host_tuple = dict_find(iter, SERVER_KEY_HOST);
    Tuple *auto_tuple = dict_find(iter, SERVER_KEY_AUTO);
    Tuple *update_int_tuple = dict_find(iter, SERVER_KEY_UPDATE_INT);

    if (ip_tuple) {
        strncpy(ip, ip_tuple->value->cstring, 16);
        text_layer_set_text(ip_text, ip);
    }
  
    if (port_tuple) {
        strncpy(port, port_tuple->value->cstring, 16);
        text_layer_set_text(port_text, port);
    }
  
    if (cpu_tuple) {
        strncpy(cpu, cpu_tuple->value->cstring, 10);
        text_layer_set_text(cpu_usage_text, cpu);
        updateBarData(progress_bar_cpu, atoi(cpu));
        layer_mark_dirty(progress_bar_cpu);
        //app_timer_reschedule(refresh_timer, DEFAULT_UPDATE_DELAY);  //Data received, start refresh timer now
    }
  
    if (mem_tuple) {
        strncpy(mem, mem_tuple->value->cstring, 10);
        text_layer_set_text(mem_usage_text, mem);
        updateBarData(progress_bar_mem, atoi(mem));
        layer_mark_dirty(progress_bar_mem);
    }
  
    if (host_tuple) {
        strncpy(host, host_tuple->value->cstring, 16);
        text_layer_set_text(hostname_text, host);
    }
  
    if (auto_tuple) {
        auto_update_prev = auto_update;   
        auto_update = atoi(auto_tuple->value->cstring);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "AUTO UPDATE RECEIVED!!!!!!");
        snprintf(logBuff, 20, "auto_update: %d", auto_update);
        APP_LOG(APP_LOG_LEVEL_DEBUG, logBuff);

        //check if auto_update has been enabled [IF it was previously disabled]
        if ((auto_update_prev == 0) && (auto_update == 1)) {
            refresh_timer = app_timer_register(update_interval, get_all, NULL);
            APP_LOG(APP_LOG_LEVEL_DEBUG, "AUTO UPDATE enabled!");
        }
    }
    
    if (update_int_tuple) {
        update_interval = atoi(update_int_tuple->value->cstring);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "UPDATE_INT RECEIVED!!!!!!");
        snprintf(logBuff, 20, "update_interval: %d", update_interval);
        APP_LOG(APP_LOG_LEVEL_DEBUG, logBuff);
    }
}

static void in_dropped_handler(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Dropped!");
}

static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Failed to Send!");
}

static void app_message_init(void) {
    // Register message handlers
    app_message_register_inbox_received(in_received_handler);
    app_message_register_inbox_dropped(in_dropped_handler);
    app_message_register_outbox_failed(out_failed_handler);
  
    // Init buffers
    app_message_open(64, 64);
}

static void progress_bar_layer_update(ProgressBarLayer *bar, GContext *ctx) {
    ProgressData *data = (ProgressData *)layer_get_data(bar);
  
    // Outline the progress bar
    graphics_context_set_stroke_color(ctx, GColorBlack);
    GRect bounds = layer_get_bounds(bar);
    graphics_draw_round_rect(ctx, GRect(0, 0, bounds.size.w, bounds.size.h), 4);
  
    // Fill the progress bar
    graphics_context_set_fill_color(ctx, GColorBlack);
    
    //graphics_fill_rect(ctx, GRect(0, 0, data->progress, bounds.size.h), 4, GCornersAll);
    graphics_fill_rect(ctx, GRect(0, 0, data->progress, bounds.size.h), 4, GCornersAll);
}

static ProgressBarLayer * progress_bar_layer_create(int y_pos) {
    ProgressBarLayer *progress_bar_layer = layer_create_with_data(GRect(6, y_pos, PBAR_WIDTH, PBAR_HEIGHT), sizeof(ProgressData));
    layer_set_update_proc(progress_bar_layer, progress_bar_layer_update);
    layer_mark_dirty(progress_bar_layer);

    return progress_bar_layer;
}

static void progress_bar_destroy(ProgressBarLayer *progress_bar_layer) {
    layer_destroy(progress_bar_layer);
}

static void window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
  
    //hostname text layer
    hostname_text = text_layer_create(GRect(0, 0, 144, 20));
    layer_add_child(window_layer, text_layer_get_layer(hostname_text));
    text_layer_set_text(hostname_text, DEFAULT_HOST);

    //cpu layers
    cpu_usage_text = text_layer_create(GRect(0, 20, 144, 20));
    layer_add_child(window_layer, text_layer_get_layer(cpu_usage_text));
    text_layer_set_text(cpu_usage_text, "CPU Usage");
    progress_bar_cpu = progress_bar_layer_create(40);     //function takes y-pos as argument
    layer_add_child(window_layer, progress_bar_cpu);

    //mem layers
    mem_usage_text = text_layer_create(GRect(0, 60, 144, 20));
    layer_add_child(window_layer, text_layer_get_layer(mem_usage_text));
    text_layer_set_text(mem_usage_text, "Mem Usage");
    progress_bar_mem = progress_bar_layer_create(80);
    layer_add_child(window_layer, progress_bar_mem);

    //ip text layer
    ip_text = text_layer_create(GRect(0, 100, 144, 20));
    layer_add_child(window_layer, text_layer_get_layer(ip_text));
    text_layer_set_text(ip_text, "IP Address");

    //port text layer
    port_text = text_layer_create(GRect(0, 115, 54, 20));
    layer_add_child(window_layer, text_layer_get_layer(port_text));
    text_layer_set_text(port_text, "Port");

    //debug text layer 
    debug_text = text_layer_create(GRect(50, 120, 94, 20));
    layer_add_child(window_layer, text_layer_get_layer(debug_text));
    text_layer_set_text(debug_text, "DEBUG");

    //Inverter layer
    iLayer = inverter_layer_create(GRect(0, 0, 144, 152));
    layer_add_child(window_layer, inverter_layer_get_layer(iLayer));
}

static void window_unload(Window *window) {
    text_layer_destroy(hostname_text);
    text_layer_destroy(cpu_usage_text);
    text_layer_destroy(mem_usage_text);
    text_layer_destroy(ip_text);
    text_layer_destroy(port_text);
    text_layer_destroy(debug_text);
    inverter_layer_destroy(iLayer);
    progress_bar_destroy(progress_bar_cpu);
    progress_bar_destroy(progress_bar_mem);
}

static void init_comms(void *data) {
    fetch_msg(SERVER_KEY_FETCH);
}

static void init(void) {
    window = window_create();
    app_message_init();
    window_set_click_config_provider(window, click_config_provider);
    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
    
    const bool animated = true;
    window_stack_push(window, animated);

    //wait after startup before commencing comms - then request config - 3 second wait
    refresh_timer = app_timer_register(BOOT_COMMS_DELAY, init_comms, NULL);
}

static void deinit(void) {
    window_destroy(window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
