#include <pebble.h>

static Window *s_window;	
	
// Keys for AppMessage Dictionary
// These should correspond to the values you defined in appinfo.json/Settings
enum {
	STATUS_KEY = 0,	
	MESSAGE_KEY = 1,
  WRITE_POS_KEY = 6,
  SUBMIT_POS_KEY = 7,
  X_KEY = 10,
  Y_KEY = 11,
  Z_KEY = 12,
  TIME_STAMP_KEY = 20,
  DATA_KEY = 4
};

//static const uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
static uint8_t* buffer;
static AccelData* accel_buffer;
static const uint32_t req_num_samples = 25;
static const uint32_t package_size = 150;

//static const uint32_t req_num_samples = 5;
//static const uint32_t package_size = 30;

//static const uint32_t total_buffer = 3000;
static const uint32_t total_queue = 20;


static bool waiting = false;
//static uint32_t write_pos = 0;
//static uint32_t submit_pos = 0;

static uint32_t write_pos_index = 0;
static uint32_t submit_pos_index = 0;





// Write message to buffer & send
static void send_message(void){
	DictionaryIterator *iter;
	
	app_message_outbox_begin(&iter);
	//dict_write_cstring(iter, MESSAGE_KEY, "I'm a Pebble!");
  //uint8_t data[package_size];
  //data = &(buffer+submit_pos);
  dict_write_uint32(iter, WRITE_POS_KEY, write_pos_index);
  dict_write_uint32(iter, SUBMIT_POS_KEY, submit_pos_index);
  
  dict_write_data(iter, DATA_KEY, buffer+ submit_pos_index*package_size, package_size);
  
  //AccelData* cur_accel0 = &accel_buffer[submit_pos_index];  
  dict_write_int16(iter, X_KEY, accel_buffer[submit_pos_index].x);
  dict_write_int16(iter, Y_KEY, accel_buffer[submit_pos_index].y);
  dict_write_int16(iter, Z_KEY, accel_buffer[submit_pos_index].z);
  
  uint32_t smallStamp = (uint32_t) accel_buffer[submit_pos_index].timestamp;
  dict_write_uint32(iter, TIME_STAMP_KEY, smallStamp);
    

	
	dict_write_end(iter);
  app_message_outbox_send();
  waiting = true;
}

static void try_send_message(void){
  if (!waiting && write_pos_index != submit_pos_index){
    send_message();
  }
}

// Called when a message is received from PebbleKitJS
static void in_received_handler(DictionaryIterator *received, void *context) {
	Tuple *tuple;
	
	tuple = dict_find(received, STATUS_KEY);
	if(tuple) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Received Status: %d", (int)tuple->value->uint32); 
	}
	
	tuple = dict_find(received, MESSAGE_KEY);
	if(tuple) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Received Message: %s", tuple->value->cstring);
	}
  
  //send_message();
}

// Called when an incoming message from PebbleKitJS is dropped
static void in_dropped_handler(AppMessageResult reason, void *context) {	
}

// Called when PebbleKitJS does not acknowledge receipt of a message
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
}

static void outbox_sent_callback(DictionaryIterator *iter, void *context) {
  // The message just sent has been successfully delivered
  waiting = false;
  //submit_pos = (submit_pos + package_size) % total_buffer;
  submit_pos_index = (submit_pos_index + 1) % total_queue;
}

static void accel_raw_data_handle(AccelRawData *data, uint32_t num_samples, uint64_t timestamp){
  if (num_samples != req_num_samples){
    return;
  }
  
  memcpy(buffer + write_pos_index*package_size, data, package_size);
  
  AccelRawData dr0 = data[0];
  AccelData d0 = {
    .x = dr0.x,
    .y = dr0.y,
    .z = dr0.z,
    .timestamp = timestamp
  };
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "X: %d", d0.x); 
  
  memcpy(&accel_buffer[write_pos_index], &d0, sizeof(AccelData));
    
  //write_pos = (write_pos+package_size) % total_buffer;
  write_pos_index = (write_pos_index+1) % total_queue;
  
  try_send_message();
}

static void tick_handle(struct tm *tick_time, TimeUnits units_changed){
  try_send_message();
}

static void end_click_handler(ClickRecognizerRef recognizer, void *context) {
  // A single click has just occured
  accel_data_service_unsubscribe();
  vibes_double_pulse();
}

static void start_click_handler(ClickRecognizerRef recognizer, void *context) {
  // A single click has just occured
  accel_raw_data_service_subscribe(req_num_samples, accel_raw_data_handle);
  vibes_short_pulse();
}


static void click_config_provider(void *context) {
  // Subcribe to button click events here
  //ButtonId id = BUTTON_ID_DOWN;
  window_single_click_subscribe(BUTTON_ID_DOWN, start_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, end_click_handler);
}

static void init(void) {
	s_window = window_create();
  // Use this provider to add button click subscriptions
  window_set_click_config_provider(s_window, click_config_provider);
	window_stack_push(s_window, true);
	
	// Register AppMessage handlers
	app_message_register_inbox_received(in_received_handler); 
	app_message_register_inbox_dropped(in_dropped_handler); 
	app_message_register_outbox_failed(out_failed_handler);
  // Register to be notified about outbox sent events
  app_message_register_outbox_sent(outbox_sent_callback);
  
  //subscribe to accel
  //accel_raw_data_service_subscribe(req_num_samples, accel_raw_data_handle);
  
  //time update
  tick_timer_service_subscribe(SECOND_UNIT, tick_handle);
  
  // Initialize AppMessage inbox and outbox buffers with a suitable size
  const int inbox_size = 128;
  const int outbox_size = 300;
	app_message_open(inbox_size, outbox_size);
  
  buffer = malloc(total_queue * package_size);
  accel_buffer = malloc(total_queue*sizeof(AccelData));
}

static void deinit(void) {
	app_message_deregister_callbacks();
  accel_data_service_unsubscribe();
  tick_timer_service_unsubscribe();
  free(buffer);
  free(accel_buffer);
	window_destroy(s_window);
}

int main( void ) {
	init();
	app_event_loop();
	deinit();
}