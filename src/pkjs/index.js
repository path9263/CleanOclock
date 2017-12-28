// More info here on JS battery info: https://googlechrome.github.io/samples/battery-status/
/*jshint esversion: 6 */



// Get settings from the config page
Pebble.addEventListener('webviewclosed', 
  function(e) {
  // Decode the user's preferences
  var configData = JSON.parse(decodeURIComponent(e.response));
  
  // Send to the watchapp via AppMessage
    console.log("vibe_interval from index.js: " + configData.vibe_interval);
  var dict = {
    'BGCOLOR': configData.background_color,
    'VIBEINTERVAL' : configData.vibe_interval
    //'ForegroundColor': configData.foreground_color,
    //'SecondTick': configData.second_ticks,
    //'Animations': configData.animations
  };
  
  // Send to the watchapp
  Pebble.sendAppMessage(dict, function() {
    console.log('Config data sent successfully!');
  }, function(e) {
    console.log('Error sending config data!');
    });
});


// New phone battery level checker, based on listener on phone:
function updateBattery(battery, forceUpdate) {
  var dictionary = {
    "BATTLVL": Math.round(battery.level * 100),
    "BATTSTATS": battery.charging
    };
    
   // Send to Pebble
   // to save (watch) battery we only do this every 2% or 5% when charging
   // if first run  -OR-
   // if not charge update every 2%  -OR-  
   // if charging update every 5%
    if(forceUpdate || 
       ((!battery.charging && (Math.round(battery.level * 100) % 2 == 0)) || 
        (battery.charging && (Math.round(battery.level * 100) % 5 == 0)))) { 
        Pebble.sendAppMessage(dictionary,
            function(e) {
              console.log("Battery info sent to Pebble successfully!");
            },
            function(e) {
              console.log("Error sending battery info to Pebble!");
            }
      );
    }
}

function monitorBattery(battery) {
  // Update the initial values.
  updateBattery(battery, true);

  // Monitor for futher updates.
  battery.addEventListener('levelchange', updateBattery.bind(null, battery, false));
  battery.addEventListener('chargingchange', updateBattery.bind(null, battery, true));
}

navigator.getBattery().then(monitorBattery);


// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
  }                     
);

// watch face config
Pebble.addEventListener('showConfiguration', function() {
  var url = 'https://path9263.github.io/CleanOclock/config.html';

  Pebble.openURL(url);
});







