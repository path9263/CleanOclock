// More info here: https://googlechrome.github.io/samples/battery-status/
/*jshint esversion: 6 */


// New phone battery level checker, based on listener on phone:
function updateBattery(battery) {
  var dictionary = {
    "BATTLVL": Math.round(battery.level * 100),
    "BATTSTATS": battery.charging
    };
    
    // Send to Pebble
        Pebble.sendAppMessage(dictionary,
            function(e) {
              console.log("Battery info sent to Pebble successfully!");
            },
            function(e) {
              console.log("Error sending battery info to Pebble!");
            }
    );
}

function monitorBattery(battery) {
  // Update the initial values.
  updateBattery(battery);

  // Monitor for futher updates.
  battery.addEventListener('levelchange', updateBattery.bind(null, battery));
  battery.addEventListener('chargingchange', updateBattery.bind(null, battery));
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







