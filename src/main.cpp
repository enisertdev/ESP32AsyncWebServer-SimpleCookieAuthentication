#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <map>
#include <ESPmDNS.h>


AsyncWebServer server(80);
const char *ssid = ".....";
const char *password = ".....";
String usernames[] = {".....","....."};
String passwords[] = {".....","....."};
std::map<String, String> sessions;
bool isLoginValid(String username, String password);

// put function declarations here:

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  int attempts = 0;

  while(WiFi.status() != WL_CONNECTED && attempts < 30)
  {
    delay(500);
    Serial.println("deneme: " + String(attempts));
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
  Serial.print("Ip address: ");
  Serial.println(WiFi.localIP());
  }

  else{
    Serial.println("Failed to connect.");
  }
  if (!MDNS.begin("home")) {  
    Serial.println("mDNS başlatılamadı");
    return;
  }
  Serial.println("mDNS başlatıldı");
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    String ipAdress = IPAddress(request->client()->remoteIP()).toString();
    String html = R"rawliteral(
    <!DOCTYPE html>
    <html lang="en">
    <body>
      <form method="GET" action="/login">
      <label for="username">Username</label>
      <input type="text" name="username">
      <label for="password">Password</label>
      <input type="text" name="password">
      <input type="submit" value="login">
      </form>
    </body>
    </html>
    )rawliteral";
    request->send(200, "text/html", html);
  });

  server.on("/login",HTTP_GET,[](AsyncWebServerRequest *request)
  {
    String inputUsername = request->getParam("username")->value();
    String inputPassword = request->getParam("password")->value();
    if(request->hasParam("username") && request->hasParam("password"))
    {
      if(isLoginValid(inputUsername, inputPassword))
      {
        String sessionId = String(random(10000000,90000000));
        sessions[sessionId] = inputUsername;
        AsyncWebServerResponse *response = request->beginResponse(302,"text/plain","");
        response->addHeader("Location","/menu"); 
        response->addHeader("Set-Cookie","sessionId=" + sessionId);
        request->send(response);
      }
    }
    else
    {
      request->send(400,"text/plain","username or password is empty.");
    }
  });

  server.on("/menu",HTTP_GET,[](AsyncWebServerRequest *request)
  {
    String storedCookie = "None";
    if(request->hasHeader("Cookie"))
    {
      String cookie = request->header("Cookie");
      int pos = cookie.indexOf("sessionId=");
      if(pos != -1)
      {
        pos+= 10;
        int endPos = cookie.indexOf(";", pos);
        if (endPos == -1) {
          endPos = cookie.length();
        }
        storedCookie = cookie.substring(pos,endPos);
        Serial.println(storedCookie);
      }
    }
    if(sessions.count(storedCookie) == 0)
    {
      AsyncWebServerResponse *response = request->beginResponse(302,"text/plain","Redirecting to login...");
      response->addHeader("Location","/");
      request->send(response);
      return;
    }
    String username = sessions[storedCookie];
    String html = R"rawliteral(
    <!DOCTYPE html>
<html lang="en">
<body>
  Logged in.
</body>
</html>
    )rawliteral";
    request->send(200, "text/html", html);
  });

  server.begin();
}

void loop()
{
}

bool isLoginValid(String username, String password)
{
  for(int i = 0; i < sizeof(usernames) / sizeof(usernames[0]); i++)
  {
    if(username == usernames[i] && password == passwords[i])
    {
      return true;
    }
  }
  return false;
  
}
