/*jshint esversion: 6 */


///////////////////////////////////////  -- CONFIG SETTINGS --  ////////////////////////////////////////////////////////////////////////
// Get settings from the config page
Pebble.addEventListener('webviewclosed', 
  function(e) {
  // Decode the user's preferences
  var configData = JSON.parse(decodeURIComponent(e.response));
  
  // Send to the watchapp via AppMessage
    //console.log("test_str from pebble.js: " + configData.test_str);
  var dict = {
    'BGCOLOR': configData.background_color,
    'INTERVALS' : [configData.vibe_interval, configData.batt_update_time, configData.chrg_update_time],
    'GPS' : [configData.gps[0], configData.gps[1]],
    'WAPIKEY' : configData.wAPIkey,
    'TOGGLES' : [configData.toggles[0], configData.toggles[1], configData.toggles[2], configData.toggles[3], configData.toggles[4], configData.toggles[5], configData.toggles[6]]
  };
  
  // Send to the watchapp
  Pebble.sendAppMessage(dict, function() {
    console.log('Config data sent successfully!');
  }, function(e) {
    console.log('Error sending config data!');
    });
});

///////////////////////////////////////  -- BATT LEVEL --  ////////////////////////////////////////////////////////////////////////
// More info here on JS battery info: https://googlechrome.github.io/samples/battery-status/
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







