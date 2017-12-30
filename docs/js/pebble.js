// Get a handle to the button's HTML element
  var submitButton = document.getElementById('submit_button');

  // Add a 'click' listener
  submitButton.addEventListener('click', function() {
    // Get the config data from the UI elements
    var backgroundColor = document.getElementById('background_color_input');
	backgroundColor.value = backgroundColor.value.replace('#','');
	var vibeInterval = $('input[name="interval_time"]:checked').val();
	var batt_update_time = $('input[name="batt_update_time"]:checked').val();
	var chrg_update_time = $('input[name="chrg_update_time"]:checked').val();
	var wAPIkey = document.getElementById('wAPIkey');
	var lat = document.getElementById('lat');
	var lon = document.getElementById('lon');
	var vibeOn = (document.getElementById("vibeOn").checked) ? 1 : 0;
	var showWeather = (document.getElementById("showWeather").checked) ? 1 : 0;
	var tempUnitC = (document.getElementById("celsius").checked) ? 1 : 0;
	var btVibe = (document.getElementById("btVibe").checked) ? 1 : 0;
	var btRe = (document.getElementById("btRe").checked) ? 1 : 0;
	var qtIcon = (document.getElementById("qtIcon").checked) ? 1 : 0;
	var battStats = (document.getElementById("battStats").checked) ? 1 : 0;

    // Make a data object to be sent, coercing value types to integers
    var options = {
      'background_color': parseInt(backgroundColor.value, 16), //parseInt('4286f4', 16),
	  'vibe_interval': parseInt(vibeInterval),
	  'batt_update_time': parseInt(batt_update_time),
	  'chrg_update_time': parseInt(chrg_update_time),
	  'wAPIkey': String(wAPIkey),
	  'gps': [parseFloat(lat), parseFloat(lon)],
	  'toggles': [vibeOn, showWeather, tempUnitC, btVibe, btRe, qtIcon, battStats]
    };

    // Determine the correct return URL (emulator vs real watch)
    function getQueryParam(variable, defaultValue) {
      var query = location.search.substring(1);
      var vars = query.split('&');
      for (var i = 0; i < vars.length; i++) {
        var pair = vars[i].split('=');
        if (pair[0] === variable) {
          return decodeURIComponent(pair[1]);
        }
      }
      return defaultValue || false;
    }
    var return_to = getQueryParam('return_to', 'pebblejs://close#');

    // Encode and send the data when the page closes
    document.location = return_to + encodeURIComponent(JSON.stringify(options));
  });