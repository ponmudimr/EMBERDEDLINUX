#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "SPIDEY";
const char* password = "ponmudi123";

WebServer server(80);

String htmlPage() {

  int n = WiFi.scanNetworks();

  String page = R"rawliteral(
<!DOCTYPE html>
<html>

<head>

<title>ESP32 WiFi Visualizer</title>

<script src="https://cdn.jsdelivr.net/npm/chart.js"></script>

<style>

body {
font-family: Arial;
padding: 20px;
}

canvas {
max-width: 1000px;
}

</style>

</head>

<body>

<h1>ESP32 WiFi Traffic Visualizer</h1>

<canvas id="wifiChart"></canvas>

<script>

const ctx = document.getElementById('wifiChart');

const wifiChart = new Chart(ctx, {

type: 'bar',

data: {

labels: [
)rawliteral";

  for (int i = 0; i < n; i++) {

    page += "'" + WiFi.SSID(i) + "'";

    if (i < n - 1) page += ",";
  }

  page += R"rawliteral(
],

datasets: [{

label: 'RSSI Signal Strength',

data: [
)rawliteral";

  for (int i = 0; i < n; i++) {

    page += String(WiFi.RSSI(i));

    if (i < n - 1) page += ",";
  }

  page += R"rawliteral(
],

borderWidth: 1

}]

}

});

</script>

</body>
</html>
)rawliteral";

  return page;
}

void handleRoot() {

  server.send(200, "text/html", htmlPage());
}

void setup() {

  Serial.begin(115200);

  WiFi.begin(ssid, password);

  Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    Serial.print(".");
  }

  Serial.println("");

  Serial.println("Connected!");

  Serial.print("IP Address: ");

  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);

  server.begin();

  Serial.println("Web server started");
}

void loop() {

  server.handleClient();
}
