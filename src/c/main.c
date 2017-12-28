#include <pebble.h>

#define SETTINGS_KEY 1

static Window *s_main_window;
static Layer *s_canvas_layer;
static TextLayer *s_time_h_layer;
static TextLayer *s_time_m_layer;
static TextLayer *s_date_layer;
static TextLayer *s_battery_layer;
static GFont s_font_consolas;
static GFont s_font_montserrat;
static GFont s_font_square;

static int s_watch_battery_level;
static int s_phone_battery_level;
static int s_phone_battery_status;
static int s_last_vibe;

static BitmapLayer *s_icons_layer;
static GBitmap *s_quiet_icon_bitmap;
static GBitmap *s_bt_icon_bitmap;
static GBitmap *s_batt_icon_bitmap;

static GBitmap *s_phone_icon_bitmap;
static GBitmap *s_watch_icon_bitmap;
static GBitmap *s_plug_icon_bitmap;

static GColor bg_color;

static const VibePattern vibeLowBatt = {
  .durations = (uint32_t[]) {100, 200, 200, 200, 400}, // on, off, on, off, etc... in ms)
  .num_segments = 5
};

static const VibePattern vibeBT = {
  .durations = (uint32_t[]) {100, 100, 100, 100, 300, 200, 300, 200, 100, 100, 100}, // on, off, on, off, etc... in ms)
  .num_segments = 11
};

// A structure containing our saved settings
typedef struct SavedSettings {
  GColor BackgroundColor;
  int vibeInterval;
} __attribute__((__packed__)) SavedSettings;

// An instance of the struct
static SavedSettings settings;

static void prv_default_settings() {
  settings.BackgroundColor = GColorWhite;
  settings.vibeInterval = 0;
}

// Read settings from persistent storage
static void prv_load_settings() {
  // Load the default settings
  prv_default_settings();
  // Read settings from persistent storage, if they exist
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}
// Save the settings to persistent storage
static void prv_save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
  // Update the display based on new settings
  //prv_update_display();
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer_h[8];
  strftime(s_buffer_h, sizeof(s_buffer_h), clock_is_24h_style() ?
                                          "%H" : "%I", tick_time); //  "%H\n%M" : "%I\n%M", tick_time);
  static char s_buffer_m[8];
  strftime(s_buffer_m, sizeof(s_buffer_m), clock_is_24h_style() ?
                                          "%M" : "%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_h_layer, s_buffer_h);
  text_layer_set_text(s_time_m_layer, s_buffer_m);
}

char lower_to_upper(char c) { // converts to upper case
  return (c>='a' && c<='z')?c&0xdf:c;
}

static void update_date() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write  into a buffer
  static char s_buffer_date[8];
  strftime(s_buffer_date, sizeof(s_buffer_date), "%a\n%d", tick_time); 
  for(char* pc=s_buffer_date;*pc!=0;++pc) *pc = lower_to_upper(*pc); // uppercase it
  // Display  on the TextLayer
  text_layer_set_text(s_date_layer, s_buffer_date);
}

static void update_battery(){
  static char battlvl_buffer[32];
  static char watch_battlvl_buffer[32];
  
  if (s_phone_battery_level != 0){
    snprintf(battlvl_buffer, sizeof(battlvl_buffer), "%d", s_phone_battery_level);
  } else {
    snprintf(battlvl_buffer, sizeof(battlvl_buffer), "%s", "--");
  }
  
  snprintf(watch_battlvl_buffer, sizeof(watch_battlvl_buffer), "\n%d", s_watch_battery_level);
  strcat(battlvl_buffer, watch_battlvl_buffer);

  text_layer_set_text(s_battery_layer, battlvl_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
    
  // update date
  if( (units_changed & DAY_UNIT) != 0 ) {
       update_date(); 
  }
  // periodic vibration
  //if((tick_time->tm_min % settings.vibeInterval == 0) && !quiet_time_is_active()) {
  if((tick_time->tm_min % 5 == 0) && !quiet_time_is_active()) {
    if(tick_time->tm_min != s_last_vibe){   //fixes multiple vibrations during intended minute
      vibes_short_pulse();
    }
  }
  s_last_vibe = tick_time->tm_min;
  
}

// object containing data
struct icon {
   GBitmap *icon_bitmap;
   int width;
};



// this thing draws graphics:
static void canvas_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  
  // Draw bg rectangle
  GRect bg_rect_bounds = GRect(0, 0, bounds.size.w, 126);
  graphics_context_set_fill_color(ctx, settings.BackgroundColor);//GColorFromHEX(4359924));//
  graphics_context_set_stroke_color(ctx, GColorClear);
  graphics_context_set_stroke_width(ctx, 0);
  graphics_fill_rect(ctx, bg_rect_bounds, 0, GCornersAll);
  //graphics_draw_rect(ctx, bg_rect_bounds);
  
  // set color based on quiet time
  if(quiet_time_is_active() || !connection_service_peek_pebble_app_connection() || (s_phone_battery_level <= 20 && s_phone_battery_level != 0)){
    graphics_context_set_fill_color(ctx, GColorDarkCandyAppleRed);
  } else {
    graphics_context_set_fill_color(ctx, GColorBlack);
  }
  
  // draw bottom bar rect
  GRect bar_rect_bounds = GRect(0, 126, bounds.size.w, 42);
  //graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, 0);
  graphics_fill_rect(ctx, bar_rect_bounds, 0, GCornersAll);
  //graphics_draw_rect(ctx, bar_rect_bounds);
  
  // draw icons:
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  
  // draw watch and phone icons:
  
  if(s_phone_battery_status){
    graphics_draw_bitmap_in_rect(ctx, s_plug_icon_bitmap, GRect(5, 130, 11, 16)); // x,y,w,h
  } else {
    graphics_draw_bitmap_in_rect(ctx, s_phone_icon_bitmap, GRect(5, 130, 11, 16)); // x,y,w,h
  }
  graphics_draw_bitmap_in_rect(ctx, s_watch_icon_bitmap, GRect(5, 149, 11, 16)); // x,y,w,h
  
  
  //////// draw any notification/alert icons:
  
  struct icon iconList[3];
  int iconCount = 0;
  int totalWidth = 0;
  
  // Bluetooth
  if(!connection_service_peek_pebble_app_connection()){ // bluetooth lost
  //if(true){
    //graphics_draw_bitmap_in_rect(ctx, s_bt_icon_bitmap, GRect(41, 129, 21, 31)); // x,y,w,h
    iconList[iconCount].icon_bitmap = s_bt_icon_bitmap;
    iconList[iconCount].width = 16; totalWidth += 16;
    iconCount++;
  }
  // Battery
  if(s_watch_battery_level <= 10 || (s_phone_battery_level <= 20 && s_phone_battery_level != 0)){ 
  //if(true){
    //graphics_draw_bitmap_in_rect(ctx, s_batt_icon_bitmap, GRect(86, 129, 21, 31)); // x,y,w,h
    iconList[iconCount].icon_bitmap = s_batt_icon_bitmap;
    iconList[iconCount].width = 16; totalWidth += 16;
    iconCount++;
  }
  //Quiet Time
  if(quiet_time_is_active()){
  //if(true){
    //graphics_draw_bitmap_in_rect(ctx, s_quiet_icon_bitmap, GRect(61, 129, 21, 31)); // x,y,w,h
    iconList[iconCount].icon_bitmap = s_quiet_icon_bitmap;
    iconList[iconCount].width = 21; totalWidth += 21;
    iconCount++;
    //graphics_draw_bitmap_in_rect(ctx, iconList[iconCount].icon_bitmap, GRect(61, 129, 21, 31)); // this is a test
    
  }
  
  // dynamically arrange and center:
  if(iconCount > 0){
    // add weather if it fits:
    int listCount = iconCount; // because it might get changed
    int addWeather = 0;
    /*if(totalWidth <= 63/3){ // this works but haven't actually added weather yet
      totalWidth += 42;
      iconCount++;
      addWeather = 1;
    }*/
    
    int space = (63 - totalWidth) / (iconCount + 1); // get spacing
    int xSoFar = space; // cumulative x position 
    int i;
    for(i = 0; i < listCount; i++){
      graphics_draw_bitmap_in_rect(ctx, iconList[i].icon_bitmap, GRect(41 + xSoFar, 129, iconList[i].width, 31)); // x,y,w,h
      xSoFar += iconList[i].width + space;
    }
    if(addWeather){
      GRect weather_rect_bounds = GRect(41 + xSoFar, 129, 42, 31);
      graphics_context_set_fill_color(ctx, GColorLimerick);
      graphics_fill_rect(ctx, weather_rect_bounds, 0, GCornersAll);
    }
    
  } else {
    // draw weather in center
  }
}

static void bluetooth_callback(bool connected) {
  static int btState = 1;
  if(!connected && !quiet_time_is_active()) { // not connected
    // update the background color
    layer_mark_dirty(s_canvas_layer);
    // Issue a vibrating alert
    vibes_enqueue_custom_pattern(vibeBT);
    btState = 0;
  } 
  if(connected && !btState && !quiet_time_is_active()) {
    vibes_double_pulse();
    btState = 1;
    layer_mark_dirty(s_canvas_layer);
  }
}

static void main_window_load(Window *window) {
  
  // load saved settings
  prv_load_settings();
  
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  s_canvas_layer = layer_create(bounds);
  
    // load icons
  s_phone_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PHONE_ICON);
  s_watch_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_WATCH_ICON);
  s_plug_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PLUG_ICON);
  
  s_quiet_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_QUIET_ICON);
  s_bt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT_ICON);
  s_batt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_ICON);
  
  // draw bg:
  bg_color = GColorWhite;
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  // Add to Window
  layer_add_child(window_get_root_layer(window), s_canvas_layer);

  // Create the TextLayer with specific bounds
  s_time_h_layer = text_layer_create(
      //GRect(0, -11, bounds.size.w, 85)); //MONTSERRAT_BOLD_68
      GRect(5, -36, bounds.size.w, 100)); // x,y,w,h

  s_time_m_layer = text_layer_create(
      //GRect(0, 47, bounds.size.w, 85)); //MONTSERRAT_BOLD_68
      GRect(5, 23, bounds.size.w, 100)); // x,y,w,h

  
    // Load the custom font
  //s_font_consolas = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CONSOLAS_BOLD_70));
  //s_font_montserrat = fonts_load_custom_font(resource_get_handle(MONTSERRAT_BOLD_68)); //MONTSERRAT_BOLD_68
  s_font_square = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FORCED_SQUARE_94)); //FORCED_SQUARE_94
  
  // custom drawing procedure, background rect
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(window_get_root_layer(window), s_canvas_layer);
  
  // hour text layer format
  text_layer_set_background_color(s_time_h_layer, GColorClear);
  text_layer_set_text_color(s_time_h_layer, GColorBlack);
  text_layer_set_text(s_time_h_layer, "00\n00");
  text_layer_set_font(s_time_h_layer, s_font_square);
  text_layer_set_text_alignment(s_time_h_layer, GTextAlignmentCenter);
  
  // minute text layer format
  text_layer_set_background_color(s_time_m_layer, GColorClear);
  text_layer_set_text_color(s_time_m_layer, GColorBlack);
  text_layer_set_text(s_time_m_layer, "00\n00");
  text_layer_set_font(s_time_m_layer, s_font_square);
  text_layer_set_text_alignment(s_time_m_layer, GTextAlignmentCenter);

  // Add time text layers to window layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_h_layer));
  
  layer_add_child(window_layer, text_layer_get_layer(s_time_m_layer));
  
  
  // Create battery Layer
  s_battery_layer = text_layer_create(
      GRect(20, 126, 44, 50)); // x,y,w,h
  // Style the text
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_text_color(s_battery_layer, GColorWhite);
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentLeft);
  text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text(s_battery_layer, "--\n--");
  // add layer to window
  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));
  
  // Create date Layer
  s_date_layer = text_layer_create(
      GRect(100, 126, 44, 50)); // x,y,w,h
  // Style the text
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text(s_date_layer, "--\n--");
  // add layer to window
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  
  // Show the correct state of the BT connection from the start
  bluetooth_callback(connection_service_peek_pebble_app_connection());

}


static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_h_layer);
  text_layer_destroy(s_time_m_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_battery_layer);
  layer_destroy(s_canvas_layer);
  // Unload GFont
  fonts_unload_custom_font(s_font_consolas);
  fonts_unload_custom_font(s_font_montserrat);
  fonts_unload_custom_font(s_font_square);
  
  gbitmap_destroy(s_quiet_icon_bitmap);
  gbitmap_destroy(s_bt_icon_bitmap);
  gbitmap_destroy(s_batt_icon_bitmap);
  gbitmap_destroy(s_phone_icon_bitmap);
  gbitmap_destroy(s_watch_icon_bitmap);
  gbitmap_destroy(s_plug_icon_bitmap);

  bitmap_layer_destroy(s_icons_layer);
}

// callback for updating watch battery level
static void battery_callback(BatteryChargeState state) {
  // Record the new battery level
  s_watch_battery_level = state.charge_percent;
  update_battery();
}

// Callbacks for using javascript to get info from web/phone: 
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Message recieved!");
  // Store incoming information

  //s_phone_battery_status
  
  Tuple *battlvl_tuple = dict_find(iterator, MESSAGE_KEY_BATTLVL);
  Tuple *battStats_tuple = dict_find(iterator, MESSAGE_KEY_BATTSTATS);
  
  Tuple *bg_color_tuple = dict_find(iterator, MESSAGE_KEY_BGCOLOR);
  Tuple *vibeInterval_tuple = dict_find(iterator, MESSAGE_KEY_VIBEINTERVAL);
  
  if(bg_color_tuple && vibeInterval_tuple) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Vibe Interval: %d", vibeInterval_tuple->value->int32);
    bg_color = GColorFromHEX(bg_color_tuple->value->int32);
    
    settings.BackgroundColor = GColorFromHEX(bg_color_tuple->value->int32);
    settings.vibeInterval = vibeInterval_tuple->value->int8;
    
    layer_mark_dirty(s_canvas_layer);
    prv_save_settings();
  }
  
  if(battlvl_tuple && battStats_tuple) {
    s_phone_battery_level = (int)battlvl_tuple->value->int32;
    s_phone_battery_status = (int)battStats_tuple->value->int8;
    //APP_LOG(APP_LOG_LEVEL_INFO, "Phone charge status: %d", s_phone_battery_status);
    
    update_battery();
    if(s_phone_battery_level == 20 || s_phone_battery_level == 10){
      // Issue a vibrating alert
      if(!s_phone_battery_status && !quiet_time_is_active()){ // if not charging 
         vibes_enqueue_custom_pattern(vibeLowBatt);
        //vibes_double_pulse();
      }
    }
  } else {  //APP_LOG(APP_LOG_LEVEL_ERROR, "All data is not available!"); }
    }
}
static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Make sure the time is displayed from the start
  update_time();
  update_date();
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Register for battery level updates
  battery_state_service_subscribe(battery_callback);
  // Ensure battery level is displayed from the start
  battery_callback(battery_state_service_peek());
  
  // Register for Bluetooth connection updates
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });
  
  // Register callbacks for javascript communication 
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  const int inbox_size = 128;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);

}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}






