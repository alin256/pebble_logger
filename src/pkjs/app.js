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

var ip_string = 'http://10.1.15.27:8000';
if (typeof localStorage.getItem('ip_string') != "undefined")
  {
    ip_string = localStorage.getItem('ip_string');
    // Show the notification
    Pebble.showSimpleNotificationOnPebble("IP loaded", ip_string);
  }


//show config
Pebble.addEventListener('showConfiguration', function() {
  var url = 'https://alin256.github.io/accel-log-config/ipconfig.html';
  Pebble.openURL(url);
});

//Called when config is executed
Pebble.addEventListener('webviewclosed', function(e) {
  // Decode the user's preferences
  var configData = JSON.parse(decodeURIComponent(e.response));
  console.log("New address: " + configData.ip_input);
  ip_string = configData.ip_input;
  // Store some data
  localStorage.setItem('ip_string', ip_string);
  // Show the notification
  Pebble.showSimpleNotificationOnPebble("IP updated", ip_string);
});

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
  
  var req = new XMLHttpRequest();
  //req.open('POST', 'http://10.1.15.27:8000', true);
  req.open('POST', ip_string, true);
  console.log("Sent to: " + ip_string);
  req.onreadystatechange = function(e) 
    {
      //console.log("Received Status: " + e.payload.status);
      if (req.readyState == 4) {
        if(req.status == 200) {
          console.log(JSON.stringify(JSON.parse(req.responseText), null, '\t'));
        }
      } 
      else {
        console.log("Error");
      }
    };
  //http.send(document.commandform.messagebody.value);

  //req.send("{\"on\": true}");
  req.send(JSON.stringify(dict));
  
});