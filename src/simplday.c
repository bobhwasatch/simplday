#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"


#define MY_UUID { 0xB1, 0x92, 0xC2, 0xCE, 0xB2, 0x1D, 0x47, 0x9E,       \
                  0x9C, 0xE2, 0x09, 0x0A, 0xDC, 0x60, 0x61, 0xEF }
PBL_APP_INFO(MY_UUID,
             "SimplDay", "Bob Hauck <bobh@haucks.org>",
             1, 0, /* App version */
             RESOURCE_ID_IMAGE_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;
TextLayer text_day_layer;
TextLayer text_date_layer;
TextLayer text_time_layer;
Layer line_layer;

int last_wday = -1;     /* used to tell when the day changes */


void line_layer_update_callback(Layer *l, GContext *ctx)
{
    (void)l;

    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_draw_line(ctx, GPoint(8, 97), GPoint(131, 97));
    graphics_draw_line(ctx, GPoint(8, 98), GPoint(131, 98));
}


void handle_init(AppContextRef ctx)
{
    ResHandle res_d;
    ResHandle res_t;
    GFont font_date;
    GFont font_time;

    (void) ctx;
    window_init(&window, "SimplDay");
    window_stack_push(&window, true /* Animated */);
    window_set_background_color(&window, GColorBlack);

    resource_init_current_app(&APP_RESOURCES);

    res_d = resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_21);
    res_t = resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_SUBSET_49);
    font_date = fonts_load_custom_font(res_d);
    font_time = fonts_load_custom_font(res_t);

    text_layer_init(&text_day_layer, window.layer.frame);
    text_layer_set_text_color(&text_day_layer, GColorWhite);
    text_layer_set_background_color(&text_day_layer, GColorClear);
    layer_set_frame(&text_day_layer.layer, GRect(8, 44, 144-8, 168-44));
    text_layer_set_font(&text_day_layer, font_date);
    layer_add_child(&window.layer, &text_day_layer.layer);

    text_layer_init(&text_date_layer, window.layer.frame);
    text_layer_set_text_color(&text_date_layer, GColorWhite);
    text_layer_set_background_color(&text_date_layer, GColorClear);
    layer_set_frame(&text_date_layer.layer, GRect(8, 68, 144-8, 168-68));
    text_layer_set_font(&text_date_layer, font_date);
    layer_add_child(&window.layer, &text_date_layer.layer);

    text_layer_init(&text_time_layer, window.layer.frame);
    text_layer_set_text_color(&text_time_layer, GColorWhite);
    text_layer_set_background_color(&text_time_layer, GColorClear);
    layer_set_frame(&text_time_layer.layer, GRect(7, 92, 144-7, 168-92));
    text_layer_set_font(&text_time_layer, font_time);
    layer_add_child(&window.layer, &text_time_layer.layer);

    layer_init(&line_layer, window.layer.frame);
    line_layer.update_proc = &line_layer_update_callback;
    layer_add_child(&window.layer, &line_layer);

    last_wday = -1;
}


void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t)
{
    (void)ctx;

    // Need to be static because they're used by the system later.
    static char date_text[] = "Xxxxxxxxx 00";
    static char day_text[] = "Xxxxxxxxx";
    static char time_text[] = "00:00";
    char *time_format;

    if (t->tick_time->tm_wday != last_wday)
    {
        string_format_time(day_text, sizeof(day_text), "%A", t->tick_time);
        text_layer_set_text(&text_day_layer, day_text);

        string_format_time(date_text, sizeof(date_text), "%B %e", t->tick_time);
        text_layer_set_text(&text_date_layer, date_text);

        last_wday = t->tick_time->tm_wday;
    }

    time_format = clock_is_24h_style() ? "%R" : "%I:%M";
    string_format_time(time_text, sizeof(time_text), time_format, t->tick_time);

    // Kludge to handle lack of non-padded hour format string
    // for twelve hour clock.
    if (!clock_is_24h_style() && (time_text[0] == '0'))
    {
        memmove(time_text, &time_text[1], sizeof(time_text) - 1);
    }

    text_layer_set_text(&text_time_layer, time_text);
}


void pbl_main(void *params)
{
    PebbleAppHandlers handlers =
    {
        .init_handler = &handle_init,
        .tick_info =
        {
            .tick_handler = &handle_minute_tick,
            .tick_units = MINUTE_UNIT
        }
    };

    app_event_loop(params, &handlers);
}
