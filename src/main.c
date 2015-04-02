/*** Includes that are required ***/ 
#include <pebble.h>
#include <time.h>
#include <string.h>

// Define the time for the splash screen 
#define SPLASH_LOADING 1500
#define LOC_COM_LOADING 2000 
#define CHALLENGE_ACCEPT 2000
  
enum 
KEY { KEY_BUTTON, 
     KEY_VIBRATE, 
     KEY_PLAYER_NAME,
     KEY_EVENT_NAME,
     KEY_RANK,
     KEY_POINTS,
     KEY_PROGRESS,
     KEY_CLUE,
     KEY_UPDATELOC
    };

#define BUTTON_UP  0
#define BUTTON_SELECT  1
#define BUTTON_DOWN  2
// Define how big field information can be 
#define MAX_EVENT_NAME 60
#define MAX_PROGRESS 25
#define MAX_PLAYER_NAME 20
#define MAX_RANK 20
#define MAX_POINTS 20 
#define MAX_CLUE 50
  
#define CONTACT_KEY 1

// Create the windows that we need 
static Window *window;
static Window *s_splash_window;
static Window *requireChallengeWindow; 
static Window *challengeAcceptedWindow; 
static Window *locationCompletedWindow; 

// Create the text layers need for events
static TextLayer *playerName_layer; 
static TextLayer *eventName_layer; 
static TextLayer *rank_layer; 
static TextLayer *progress_layer; 
static TextLayer *points_layer; 
static TextLayer *clue_layer; 

static BitmapLayer *s_splash_bitmap_layer; 

// Create text layers needed for require challenge window

static BitmapLayer *challenge_bitmap_layer; 
static GBitmap *challenge_bitmap; 
static TextLayer *warning_msg_layer; 

// Create text layers needed for challenge accept window

static BitmapLayer *accept_bitmap_layer; 
static GBitmap *accept_bitmap; 
static TextLayer *accept_msg_layer; 

// Create text layers needed for challenge completed window
static BitmapLayer *loccompleted_bitmap_layer; 
static GBitmap *loccompleted_bitmap; 
static TextLayer *loc_msg_layer;

AppTimer *timer; 

/****** Splash Screen ********/ 

// Text layer for splash screen 
static TextLayer *s_text_loading_layer;

// Bitmap for Splash Screen 
static GBitmap *s_splash_bitmap;
/***************************/ 

/*** Structure for storing events ****/ 
typedef struct {
  char eventName[MAX_EVENT_NAME]; 
  char playerName[MAX_PLAYER_NAME];	       
  char rank[MAX_RANK];
  char points[MAX_POINTS];
  char progress[MAX_PROGRESS]; 	
  char clue[MAX_CLUE];
} eventInfo;

/***** Flags *****/
eventInfo temp; 

/***** Handling App Messages *****/
void timer_vibrate(void *data);


// For sending functions
static void send(int key, int message)
{
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter); 

  dict_write_int(iter, key, &message, sizeof(int), true);

  app_message_outbox_send(); 
}
/**
 * This is for receieving messages in the Pebble 
 * @param iterator [description]
 * @param context  [description]
 */
static void inbox_received_handler(DictionaryIterator *iterator, void *context) 
{
  // Get the first pair 
  Tuple *t = dict_read_first(iterator);

  // Process all pairs present 
  while(t != NULL)
  {
    // Process this pair's key
    switch(t->key)
    {
      case KEY_VIBRATE:
      // Trigger vibration 
      text_layer_set_text(clue_layer, "You Are Getting Closer."); 
      vibes_short_pulse(); 
      app_timer_register(300, (AppTimerCallback) timer_vibrate, NULL);
//       vibes_short_pulse();
      app_timer_register(300, (AppTimerCallback) timer_vibrate, NULL);
//       vibes_short_pulse();
      app_timer_register(300, (AppTimerCallback) timer_vibrate, NULL);
//       vibes_short_pulse();
      break;
      
      case KEY_PLAYER_NAME:
      strcpy(temp.playerName, t->value->cstring);
      text_layer_set_text(playerName_layer, t->value->cstring); 
      break;

      case KEY_EVENT_NAME:
      if (window_is_loaded(requireChallengeWindow))
      {
        window_destroy(requireChallengeWindow);
        window_stack_push(window, 1); 
        window_stack_push(challengeAcceptedWindow, 1); 
      }
      strcpy(temp.eventName, t->value->cstring);
      text_layer_set_text(eventName_layer, t->value->cstring); 
      break;

      case KEY_RANK:
      strcpy(temp.rank, t->value->cstring);
      text_layer_set_text(rank_layer, t->value->cstring);
      break;

      case KEY_POINTS:
      strcpy(temp.points, t->value->cstring);
      text_layer_set_text(points_layer, t->value->cstring);
      break;

      case KEY_PROGRESS:
      strcpy(temp.progress, t->value->cstring);
      text_layer_set_text(progress_layer, t->value->cstring);
      break; 
      
      case KEY_CLUE:
      strcpy(temp.clue, t->value->cstring);
      text_layer_set_text(clue_layer, t->value->cstring); 
      break; 
      
      case KEY_UPDATELOC:
      window_stack_push(locationCompletedWindow, 1); 
      text_layer_set_text(loc_msg_layer, t->value->cstring);
      break;       
      
      default:
      APP_LOG(APP_LOG_LEVEL_INFO, "Unknown key: %d", (int)t->key); 
      break;
    }
    // Get next pair, if any
    t = dict_read_next(iterator);
  }
//   persist_write_data(CONTACT_KEY, &temp, sizeof(temp));
//   printf("The name is %s\n", temp.playerName); 
//   printf("%d", persist_exists(CONTACT_KEY)); 
//   vibes_short_pulse(); 
//   text_layer_set_text(text_layer, "Contact Recieved.");
}

/********************************* Buttons ************************************/

/**
 * This is for catching the select button
 * @param recognizer [description]
 * @param context    [description]
 */
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(clue_layer, "o.o");

  send(KEY_BUTTON, BUTTON_SELECT);
}
/**
 * This handles the up click on the pebble 
 * @param recognizer [description]
 * @param context    [description]
 */
static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(clue_layer, "Up");

  send(KEY_BUTTON, BUTTON_UP);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(clue_layer, "Down");

  send(KEY_BUTTON, BUTTON_DOWN);
}

static void click_config_provider(void *context) {
  // Assign button handlers
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void inbox_dropped_handler(AppMessageResult reason, void *context)
{
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message was dropped!"); 
}
static void outbox_failed_handler(DictionaryIterator *iterator, AppMessageResult reason, void *context)
{
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message that was sent failed!"); 
}
static void outbox_sent_handler(DictionaryIterator *iterator, void *context)
{
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!"); 
}
void timer_vibrate(void *data) {
 vibes_short_pulse(); 
}
void timer_callback(void *data) {
  window_stack_pop(true);
}

/******************************* main_window **********************************/
static void splash_window_load(Window *window){
  // Set a 1000 millisecond to load the splash screen
  app_timer_register(SPLASH_LOADING, (AppTimerCallback) timer_callback, NULL);
  Layer *window_layer = window_get_root_layer(window); 
  GRect window_bounds = layer_get_bounds(window_layer);
  s_splash_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SPLASH);
  s_splash_bitmap_layer = bitmap_layer_create(GRect(12, 12, 120, 120));
  bitmap_layer_set_bitmap(s_splash_bitmap_layer, s_splash_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_splash_bitmap_layer));
  s_text_loading_layer = text_layer_create(GRect(5, 123, window_bounds.size.w - 5, 30));
  text_layer_set_font(s_text_loading_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text(s_text_loading_layer, "Challenge Link");
  text_layer_set_text_alignment(s_text_loading_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(s_text_loading_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_text_loading_layer));  
}

/**
 * This unloads all the layers after splash screen closes. 
 * @param Window: The window of the splash screen
 */
static void splash_window_unload(Window *window){
  text_layer_destroy(s_text_loading_layer);
  gbitmap_destroy(s_splash_bitmap);
  bitmap_layer_destroy(s_splash_bitmap_layer);
}
static void locationCompleted_load(Window *window){
  // Set a 1000 millisecond to load the splash screen
  app_timer_register(LOC_COM_LOADING, (AppTimerCallback) timer_callback, NULL);
  Layer *window_layer = window_get_root_layer(window); 
  GRect window_bounds = layer_get_bounds(window_layer);
  loccompleted_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CONGRATS);
  loccompleted_bitmap_layer = bitmap_layer_create(GRect(15, 15, 110, 110));
  bitmap_layer_set_bitmap(loccompleted_bitmap_layer, loccompleted_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(loccompleted_bitmap_layer));
  loc_msg_layer = text_layer_create(GRect(5, 120, window_bounds.size.w - 5, 40));
  text_layer_set_font(loc_msg_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text(loc_msg_layer, "Location 6 is Done.");
  text_layer_set_text_alignment(loc_msg_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(loc_msg_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(loc_msg_layer));  
}
/**
 * This unloads all the layers after splash screen closes. 
 * @param Window: The window of the splash screen
 */
static void locationCompleted_unload(Window *window){
  text_layer_destroy(loc_msg_layer);
  gbitmap_destroy(loccompleted_bitmap);
  bitmap_layer_destroy(loccompleted_bitmap_layer);
}
static void challengeAcceptedWindow_load(Window *window){
  app_timer_register(CHALLENGE_ACCEPT, (AppTimerCallback) timer_callback, NULL);
  Layer *window_layer = window_get_root_layer(window); 
  GRect window_bounds = layer_get_bounds(window_layer);
  accept_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHALLENGEJOINED);
  accept_bitmap_layer = bitmap_layer_create(GRect(15, 15, 110, 110));
  bitmap_layer_set_bitmap(accept_bitmap_layer, accept_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(accept_bitmap_layer));
  accept_msg_layer = text_layer_create(GRect(5, 110, window_bounds.size.w - 5, 40));
  text_layer_set_font(accept_msg_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text(accept_msg_layer, temp.eventName);
  text_layer_set_text_alignment(accept_msg_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(accept_msg_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(accept_msg_layer));  
}
/**
 * This unloads all the layers after splash screen closes. 
 * @param Window: The window of the splash screen
 */
static void challengeAcceptedWindow_unload(Window *window){
  text_layer_destroy(accept_msg_layer);
  gbitmap_destroy(accept_bitmap);
  bitmap_layer_destroy(accept_bitmap_layer);
}
static void requireChallenge_load(Window *window){
  Layer *window_layer = window_get_root_layer(window); 
  GRect window_bounds = layer_get_bounds(window_layer);
  challenge_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_WARNING);
  challenge_bitmap_layer = bitmap_layer_create(GRect(10, 10, 120, 120));
  bitmap_layer_set_bitmap(challenge_bitmap_layer, challenge_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(challenge_bitmap_layer));
  warning_msg_layer = text_layer_create(GRect(5, 120, window_bounds.size.w - 5, 30));
  text_layer_set_font(warning_msg_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text(warning_msg_layer, "Join A Challenge.");
  text_layer_set_text_alignment(warning_msg_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(warning_msg_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(warning_msg_layer));  
}
/**
 * This unloads all the layers after splash screen closes. 
 * @param Window: The window of the splash screen
 */
static void requireChallenge_unload(Window *window){
  text_layer_destroy(warning_msg_layer);
  gbitmap_destroy(challenge_bitmap);
  bitmap_layer_destroy(challenge_bitmap_layer);
}
static void window_load(Window *window) {
//   eventInfo tempEvent;  
//   strcpy(tempEvent.eventName, "Learn Western Campus"); 
//   strcpy(tempEvent.progress, "Completed 2 / 20");
//   strcpy(tempEvent.rank, "6th / 10");  
  
//   if (persist_exists(CONTACT_KEY))
//   {
//     printf("Time to read some previous data"); 
//     persist_read_data(CONTACT_KEY, &tempEvent, sizeof(tempEvent));
//     text_layer_set_text(name_layer, tempEvent.playerName); 
//     text_layer_set_text(email_layer, tempEvent.eventName); 
//     text_layer_set_text(phone_layer, tempEvent.rank); 
//   }
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  eventName_layer = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { bounds.size.w, 30 } });
  text_layer_set_font(eventName_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
//   text_layer_set_text(eventName_layer, tempEvent.eventName);
  text_layer_set_text(eventName_layer, "UWO Scavanger");
  text_layer_set_text_alignment(eventName_layer, GTextAlignmentCenter);
  text_layer_set_background_color(eventName_layer, GColorBlack);
  text_layer_set_text_color(eventName_layer, GColorWhite); 
  text_layer_set_overflow_mode(eventName_layer, GTextOverflowModeWordWrap);

  progress_layer = text_layer_create((GRect) { .origin = { 0, 30 }, .size = { bounds.size.w, 20 } });
  text_layer_set_font(progress_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
//   text_layer_set_text(progress_layer, tempEvent.progress);
  text_layer_set_text(progress_layer, "Progress: 2 / 20");
  text_layer_set_text_alignment(progress_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(progress_layer, GTextOverflowModeWordWrap);

  rank_layer = text_layer_create((GRect) { .origin = { 0, 50 }, .size = { bounds.size.w, 20 } });
  text_layer_set_font(rank_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
//   text_layer_set_text(rank_layer, tempEvent.rank);
  text_layer_set_text(rank_layer,"Ranking: 6 / 10"); 
  text_layer_set_text_alignment(rank_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(rank_layer, GTextOverflowModeWordWrap);

  points_layer = text_layer_create((GRect) { .origin = { 0, 70 }, .size = { bounds.size.w, 24 } });
  text_layer_set_font(points_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text(points_layer, "2300 points");
  text_layer_set_text_alignment(points_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(points_layer, GTextOverflowModeWordWrap);
  
  clue_layer = text_layer_create((GRect) { .origin = { 0, 95 }, .size = { bounds.size.w, 65 } });
  text_layer_set_font(clue_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text(clue_layer, "Clue: Where would you buy books?");
  text_layer_set_text_alignment(clue_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(clue_layer, GTextOverflowModeWordWrap);
  
  layer_add_child(window_layer, text_layer_get_layer(progress_layer));
  layer_add_child(window_layer, text_layer_get_layer(eventName_layer));
  layer_add_child(window_layer, text_layer_get_layer(rank_layer));
  layer_add_child(window_layer, text_layer_get_layer(points_layer)); 
  layer_add_child(window_layer, text_layer_get_layer(clue_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(progress_layer); 
  text_layer_destroy(eventName_layer); 
  text_layer_destroy(rank_layer); 
  text_layer_destroy(points_layer); 
  text_layer_destroy(clue_layer); 
}

static void init(void) {
  // Register callbacks
  app_message_register_inbox_received(inbox_received_handler);
  app_message_register_inbox_dropped(inbox_dropped_handler);
  app_message_register_outbox_failed(outbox_failed_handler);
  app_message_register_outbox_sent(outbox_sent_handler);

  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  window = window_create();
  s_splash_window = window_create();
  requireChallengeWindow = window_create(); 
  challengeAcceptedWindow = window_create(); 
  locationCompletedWindow = window_create(); 

  // Set the click configuration 
  window_set_click_config_provider(window, click_config_provider);

  // This sets up the main screen 
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_set_window_handlers(requireChallengeWindow, (WindowHandlers) {
    .load = requireChallenge_load, 
    .unload = requireChallenge_unload, 
  });
  window_set_window_handlers(challengeAcceptedWindow, (WindowHandlers){
    .load = challengeAcceptedWindow_load,
    .unload = challengeAcceptedWindow_unload, 
  });
  window_set_window_handlers(locationCompletedWindow, (WindowHandlers){
    .load = locationCompleted_load,
    .unload = locationCompleted_unload, 
  });

  // This creates the splash screen 
  window_set_window_handlers(s_splash_window, (WindowHandlers) {
    .load = splash_window_load,
    .unload = splash_window_unload,
  });

  const bool animated = true;
  window_stack_push(requireChallengeWindow, animated); 
  window_stack_push(s_splash_window, animated);
}

// Deinitalizing the windows 
static void deinit(void) {
  window_destroy(requireChallengeWindow);
  window_destroy(locationCompletedWindow); 
  window_destroy(challengeAcceptedWindow); 
  window_destroy(window);
  window_destroy(s_splash_window);
}

// Main function 
int main(void) {
  init();

//   APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
