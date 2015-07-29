var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
  console.log("Opened (xhr) " + url);
};

function HTTPGET(url) {
    var req = new XMLHttpRequest();
    req.open("GET", url, false);
    req.send(null);
    console.log("Opened (HTTPGET) " + url);  
   return req.responseText;
}

function getTemperature(url) {

  // Send request to Netatmo
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);

      // Temperaturen
      var temperatureOut = Math.round(json.body.modules[0].dashboard_data.Temperature);
      var temperatureUp = Math.round(json.body.modules[2].dashboard_data.Temperature);
      var temperatureMid = Math.round(json.body.devices[0].dashboard_data.Temperature);
      var temperatureDown = Math.round(json.body.modules[1].dashboard_data.Temperature);
      console.log("Temperatur Garten ist " + temperatureOut);
      console.log("Temperatur oben ist " + temperatureUp);
      console.log("Temperatur Schlafzimmer ist " + temperatureMid);
      console.log("Temperatur Wohnzimmer ist " + temperatureDown);

      // CO2
      var co2Up = json.body.modules[2].dashboard_data.CO2;      
      var co2Mid = json.body.devices[0].dashboard_data.CO2;      
      var co2Down = json.body.modules[1].dashboard_data.CO2;      
      console.log("CO2 oben ist " + co2Up);
      console.log("CO2 Schlafzimmer ist " + co2Mid);
      console.log("CO2 Wohnzimmer ist " + co2Down);

      // Assemble dictionary using our keys
      var dictionary = {
        "KEY_TEMPERATURE_OUT": temperatureOut,
        "KEY_TEMPERATURE_UP": temperatureUp,
        "KEY_TEMPERATURE_MID": temperatureMid,
        "KEY_TEMPERATURE_DOWN": temperatureDown,
        "KEY_CO2_UP": co2Up,
        "KEY_CO2_MID": co2Mid,
        "KEY_CO2_DOWN": co2Down
      };

      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("Temperatures sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending temperatures to Pebble!");
        }
      );
    }      
  );
}

function getWeather() {

  // Send request to Yahoo
  var url = 'https://query.yahooapis.com/v1/public/yql?q=' +
      encodeURIComponent('select * from weather.forecast where woeid in (select woeid from geo.places(1) where text="nome, ak")') + '&format=json';
  console.log('YahooURL is ' + url);
  
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);
      var condition = json.query.results.channel.item.condition;

      // Conditions
      var conditionCode = parseInt(condition.code, 10);
      console.log('Condition code is ' + conditionCode);

      // Assemble dictionary using our keys
      var dictionary = {
        "KEY_CONDITION": condition
      };

      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("Weather condition sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending weather condition to Pebble!");
        }
      );
    }      
  );
}


// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log("PebbleKit JS ready!");

    // Get the initial temperatures - first receive the API-URL including the required token
    xhrRequest("http:/url-to-token/", 'GET', 
      function(token) {
          getTemperature(token);
      }
    );
    getWeather();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");

    // Get the temperatures - first receive the API-URL including the required token
    xhrRequest("http:/url-to-token/", 'GET', 
      function(token) {
          getTemperature(token);
      }
    );
    getWeather();
  }                     
);
