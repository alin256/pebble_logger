// Function to send a message to the Pebble using AppMessage API
// We are currently only sending a message using the "status" appKey defined in appinfo.json/Settings
function sendMessage() {
	Pebble.sendAppMessage({"status": 1}, messageSuccessHandler, messageFailureHandler);
}

// Called when the message send attempt succeeds
function messageSuccessHandler() {
  console.log("Message send succeeded.");  
}

// Called when the message send attempt fails
function messageFailureHandler() {
  console.log("Message send failed.");
  sendMessage();
}

// Called when JS is ready
Pebble.addEventListener("ready", function(e) {
  console.log("JS is ready!");
  sendMessage();
});
												
// Called when incoming message from the Pebble is received
// We are currently only checking the "message" appKey defined in appinfo.json/Settings
Pebble.addEventListener("appmessage", function(e) {
  // Get the dictionary from the message
  var dict = e.payload;

  //console.log('Got message: ' + JSON.stringify(dict));
  console.log("Write: " + dict["6"] + " Read: " + dict["7"]);
  console.log("Timestamp: " + dict["20"]);
  console.log("Values: " + dict["10"] + ", " + dict["11"] + ", " + dict["12"]);
  
  console.log("Data: " + dict["4"]);
  
});