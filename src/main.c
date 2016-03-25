#include <pebble.h>

static Window *s_main_window;
static GFont s_font;
static TextLayer *s_time_layer;
static TextLayer *s_battery_layer;
static TextLayer *s_connection_layer;

/***Handle Battery***/
static void handle_battery(BatteryChargeState charge_state) {
	static char battery_text[] = "100%";
	if(charge_state.is_charging) {
		snprintf(battery_text, sizeof(battery_text), "N/A");
	} else {
		snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
	}
	
	static char s_buffer[35];
	snprintf(s_buffer, sizeof(s_buffer), "root@PC:/$ info\nbattery:   %s", battery_text);
	
	text_layer_set_text(s_battery_layer, s_buffer);
}

/***Handle Connection***/
static void handle_bluetooth(bool connected) {
	text_layer_set_text(s_connection_layer, connected ? "connected: yes" : "connected: no");
}

/***Handle Time***/
static void update_time() {
	//Get a tm structure
	time_t temp = time(NULL);
	struct tm *tick_time = localtime(&temp);
	
	//Write current hours and minutes into a buffer
	//Desired style: "Thu Mar 24 01:46"
	static char time_text[] = "day mon dd hh:mm";
	strftime(time_text, sizeof(time_text), clock_is_24h_style() ? "%a %b %e %R" : "%a %b %e %I:%M", tick_time);
	
	//Display time on the TextLayer
	static char s_buffer[50];
	snprintf(s_buffer, sizeof(s_buffer), "root@PC:/$ date\n%s",time_text);
	text_layer_set_text(s_time_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	update_time();
}

/***Handle Window***/
static void main_window_load(Window *window) {
	//Get information about the window
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);
	
	//Create Platform-specific TextLayers and GFont
	#if defined(PBL_RECT)
		s_time_layer = text_layer_create(GRect(5, 40, bounds.size.w, 40));
		s_battery_layer = text_layer_create(GRect(5, 80, bounds.size.w, 50));
		s_connection_layer = text_layer_create(GRect(5, 110, bounds.size.w, 20));
		s_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MONACO_14));
	#elif defined(PBL_ROUND)
		s_time_layer = text_layer_create(GRect(10, 40, bounds.size.w, 35));
		s_battery_layer = text_layer_create(GRect(10, 80, bounds.size.w, 40));
		s_connection_layer = text_layer_create(GRect(10, 114, bounds.size.w, 20));
		s_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MONACO_16));
	#endif
		
	//Customize the layout and add to Window's root layer
	window_set_background_color(s_main_window, GColorBlack);
	
	//Battery layer
	text_layer_set_background_color(s_battery_layer, GColorBlack);
	text_layer_set_text_color(s_battery_layer, GColorWhite);
	text_layer_set_font(s_battery_layer, s_font);
	layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));
	
	//Connection layer
	text_layer_set_background_color(s_connection_layer, GColorBlack);
	text_layer_set_text_color(s_connection_layer, GColorWhite);
	text_layer_set_font(s_connection_layer, s_font);
	handle_bluetooth(connection_service_peek_pebble_app_connection());
	layer_add_child(window_layer, text_layer_get_layer(s_connection_layer));
	
	//Time layer
	text_layer_set_background_color(s_time_layer, GColorBlack);
	text_layer_set_text_color(s_time_layer, GColorWhite);
	text_layer_set_font(s_time_layer, s_font);
	layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
}

static void main_window_unload(Window *window) {
	//Destroy elements
	fonts_unload_custom_font(s_font);
	text_layer_destroy(s_battery_layer);
	text_layer_destroy(s_connection_layer);
	text_layer_destroy(s_time_layer);
}

static void init() {
	//Create main window
	s_main_window = window_create();
	
	//Set window handlers
	window_set_window_handlers(s_main_window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload
	});
	
	//Show window on the watch, with animated = true
	window_stack_push(s_main_window, true);
	
	//Display time at the start
	update_time();
	
	//Register with services
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	battery_state_service_subscribe(handle_battery);
	connection_service_subscribe((ConnectionHandlers) {
		.pebble_app_connection_handler = handle_bluetooth
	});
	handle_battery(battery_state_service_peek());
}

static void deinit() {
	//Destroy window
	window_destroy(s_main_window);
	
	//Unregister with services
	tick_timer_service_unsubscribe();
	battery_state_service_unsubscribe();
	connection_service_unsubscribe();
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}