// Get a handle to the button's HTML element
  var submitButton = document.getElementById('submit_button');

  // Add a 'click' listener
  submitButton.addEventListener('click', function() {
    // Get the config data from the UI elements
    var backgroundColor = document.getElementById('background_color_input');
	backgroundColor.value = backgroundColor.value.replace('#','');
    //var foregroundColor = document.getElementById('foreground_color_input');
    //var secondTickCheckbox = document.getElementById('second_tick_checkbox');
    //var animationsCheckbox = document.getElementById('animations_checkbox');

    // Make a data object to be sent, coercing value types to integers
    var options = {
      'background_color': parseInt(backgroundColor.value, 16)
      //'foreground_color': parseInt(foregroundColor.value, 16),
      //'second_ticks': secondTickCheckbox.checked == 'true' ? 1 : 0,
      //'animations': animationsCheckbox.checked == 'true' ? 1 : 0
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