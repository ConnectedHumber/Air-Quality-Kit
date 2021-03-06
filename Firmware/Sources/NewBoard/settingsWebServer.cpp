
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

#include "settingsWebServer.h"
#include "settings.h"
#include "sensors.h"
#include "processes.h"
#include "lora.h"

#define WEB_PAGE_BUFFER_SIZE 3000

#define WEBSERVER_OK 0
#define WEBSERVER_OFF 1
#define WEBSERVER_ERROR_NO_WIFI -1

char webPageBuffer [WEB_PAGE_BUFFER_SIZE+1];

WebServer * webServer;

const char homePageHeader[] =
"<html>"
"<head>"
//"<style>input {font-size: 1.2em; width: 100%; max-width: 360px; display: block; margin: 5px auto; } </style>"
"<style>input {margin: 5px auto; } </style>"
"</head>"
"<body>"
"<h1>Connected Humber Environmental Sensor</h1>"
"<h3>Version %d.%d</h3>" // version number goes here;
"<h1>Settings</h1>";

const char homePageFooter[] =
"<p> Select the link to the settings page that you want to edit.</p>"
"<p> Select the reset link below to reset the sensor when you have finished.</p>"
"<a href=""reset"">reset</a>"
"</body>"
"</html>";

void addItem(SettingItemCollection * settings)
{
	snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, "%s <p style=\"margin-left: 20px; line-height: 50%%\"><a href=""%s"">%s</a> </p>\n",
		webPageBuffer,
		settings->collectionName,
		settings->collectionDescription);
}

void buildHomePage()
{
	snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, homePageHeader, MAJOR_VERSION, MINOR_VERSION);

	iterateThroughProcessSettingCollections(addItem);

	iterateThroughSensorSettingCollections(addItem);

	snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, "%s %s",
		webPageBuffer, homePageFooter);

	Serial.println(webPageBuffer);
}

const char settingsPageHeader[] =
"<html>"
"<head>"
"<style>input {margin: 5px auto; } </style>"
"</head>"
"<body>"
"<h1>Settings</h1>"
"<h2>%s</h2>" // configuration description goes here
"<form id='form' action='/%s' method='post'>"; // configuration short name goes here

const char settingsPageFooter[] =
"%s"// entire page goes here
"<input type='submit' value='Update'>"
"</form>"
"</body>"
"</html>";

void buildCollectionSettingsPage(SettingItemCollection * settingCollection)
{
	int * valuePointer;
	double * doublePointer;
	boolean * boolPointer;
	u4_t * loraIDValuePointer;
	char loraKeyBuffer[LORA_KEY_LENGTH * 2 + 1];

	snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, settingsPageHeader, settingCollection->collectionDescription, settingCollection->collectionName);

	// Start the search at setting collection 0 so that the quick settings are used to build the web page
	for (int i = 0; i < settingCollection->noOfSettings; i++)
	{
		snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, "%s <label for='%s'> %s: </label>",
			webPageBuffer,
			settingCollection->settings[i]->formName,
			settingCollection->settings[i]->prompt);

		switch (settingCollection->settings[i]->settingType)
		{
		case text:
			snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, "%s <input name = '%s' type = 'text' value='%s' style=\"margin-left: 20px; line-height: 50%%\"><br>",
				webPageBuffer, settingCollection->settings[i]->formName, settingCollection->settings[i]->value);
			break;
		case password:
			snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, "%s <input name = '%s' type = 'password' value='%s'><br>",
				webPageBuffer, settingCollection->settings[i]->formName, settingCollection->settings[i]->value);
			break;
		case integerValue:
			valuePointer = (int*)settingCollection->settings[i]->value;
			snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, "%s <input name = '%s' type = 'text' value='%d'><br>",
				webPageBuffer, settingCollection->settings[i]->formName, *valuePointer);
			break;
		case doubleValue:
			doublePointer = (double*)settingCollection->settings[i]->value;
			snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, "%s <input name = '%s' type = 'text' value='%lf'><br>",
				webPageBuffer, settingCollection->settings[i]->formName, *doublePointer);
			break;
		case onOff:
			boolPointer = (boolean*)settingCollection->settings[i]->value;
			if (*boolPointer)
			{
				snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, "%s <input name = '%s' type = 'text' value='on'><br>",
					webPageBuffer, settingCollection->settings[i]->formName);
			}
			else
			{
				snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, "%s <input name = '%s' type = 'text' value='off'><br>",
					webPageBuffer, settingCollection->settings[i]->formName);
			}
			break;
		case yesNo:
			boolPointer = (boolean*)settingCollection->settings[i]->value;
			if (*boolPointer)
			{
				snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, "%s <input name = '%s' type = 'text' value='yes'><br>",
					webPageBuffer, settingCollection->settings[i]->formName);
			}
			else
			{
				snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, "%s <input name = '%s' type = 'text' value='no'><br>",
					webPageBuffer, settingCollection->settings[i]->formName);
			}
			break;

		case loraKey:
			dumpHexString(loraKeyBuffer, (uint8_t*)settingCollection->settings[i]->value, LORA_KEY_LENGTH);
			snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, "%s <input name = '%s' type = 'text' value='%s'><br>",
				webPageBuffer, settingCollection->settings[i]->formName, loraKeyBuffer);
			break;

		case loraID:
			loraIDValuePointer = (u4_t*)settingCollection->settings[i]->value;
			dumpUnsignedLong(loraKeyBuffer, *loraIDValuePointer);
			snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, "%s <input name = '%s' type = 'text' value='%s'><br>",
				webPageBuffer, settingCollection->settings[i]->formName, loraKeyBuffer);
			break;

			}
		}

	snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, settingsPageFooter, webPageBuffer);
}

const char replyPageHeader[] =
"<html>"
"<head>"
"<style>input {margin: 5px auto; } </style>"
"</head>"
"<body>"
"<h1> Monitair Configuration</h1>"
"<h2>%s</h2>"; // configuration description goes here

const char replyPageFooter[] =
"%s"
"<p>Settings updated.</p>"
"<p><a href = ""/"">return to the settings home screen </a></p>"
"</body></html>";

void updateItem(SettingItemCollection* settings)
{
	snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, "%s <p><a href=""%s"">%s</a> </p>",
		webPageBuffer,
		settings->collectionName,
		settings->collectionDescription);
}


void updateSettings(WebServer *webServer, SettingItemCollection * settingCollection)
{
	snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, replyPageHeader, settingCollection->collectionDescription, settingCollection->collectionName);

	for (int i = 0; i < settingCollection->noOfSettings; i++)
	{
		String fName = String(settingCollection->settings[i]->formName);
		String argValue = webServer->arg(fName);

		if (!settingCollection->settings[i]->validateValue(
			settingCollection->settings[i]->value,
			argValue.c_str()))
		{
			snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, "%s <p>Invalid value %s for %s</p> ",
				webPageBuffer, argValue.c_str(), settingCollection->settings[i]->prompt);
		}
	}
	saveSettings();
	snprintf(webPageBuffer, WEB_PAGE_BUFFER_SIZE, replyPageFooter, webPageBuffer);
}

void serveHome(WebServer *webServer)
{
	// Serial.println("Serve request hit");

	if (webServer->args() == 0) {
		// Serial.println("Serving the home page");
		buildHomePage();
		webServer->sendHeader("Content-Length", String(strlen(webPageBuffer)));
		webServer->send(200, "text/html", webPageBuffer);
	}
}

const char pageNotFoundMessage[] =
"<html>"
"<head>"
"<style>input {margin: 5px auto; } </style>"
"</head>"
"<body>"
"<h1>Sensor Configuration</h1>"
"<h2>Page not found</h2>"
"<p>Sorry about that.</p>"
"</body></html>";

const char sensorResetMessage[] =
"<html>"
"<head>"
"<style>input {margin: 5px auto; } </style>"
"</head>"
"<body>"
"<h1>Sensor Configuration</h1>"
"<h2>Reset</h2>"
"<p>Sensor will reset in a few seconds.</p>"
"</body></html>";


void pageNotFound(WebServer *webServer)
{
	Serial.println("Not found hit");

	String uriString = webServer->uri();

	const char * uriChars = uriString.c_str();
	const char * pageNameStart = uriChars + 1;

	if(strcasecmp(pageNameStart, "reset")==0)
	{
		Serial.println("Resetting device");
		// reset the device
		webServer->sendHeader("Content-Length", String(strlen(sensorResetMessage)));
		webServer->send(200, "text/html", sensorResetMessage);
		delay(5000);
		esp_restart();
	}

	SettingItemCollection * items = findSettingItemCollectionByName(pageNameStart);

	if (items != NULL)
	{
		// url refers to an existing set of settings
		if (webServer->args() == 0)
		{
			// Not a post - just serve out the settings form
			//Serial.printf("Got settings request for %s\n", items->collectionName);
			buildCollectionSettingsPage(items);
		}
		else
		{
			// Posting new values - now we need to read them
			//Serial.printf("Got new data for %s\n", items->collectionName);
			updateSettings(webServer, items);
		}
		webServer->sendHeader("Content-Length", String(strlen(webPageBuffer)));
		webServer->send(200, "text/html", webPageBuffer);
	}
	else
	{
		//Serial.printf("Page not found: %s\n", uriString.c_str());

		webServer->sendHeader("Content-Length", String(strlen(pageNotFoundMessage)));
		webServer->send(200, "text/html", pageNotFoundMessage);
	}
}

struct process * webServerWiFiProcess = NULL;


int startWebServer(struct process * webserverProcess)
{
	buildHomePage();

	if (webServerWiFiProcess == NULL)
	{
		webServerWiFiProcess = findProcessByName("WiFi");
	}

	if (webServerWiFiProcess->status != WIFI_OK)
	{
		webserverProcess->status = WEBSERVER_ERROR_NO_WIFI;
		return WEBSERVER_ERROR_NO_WIFI;
	}

	webServer = new WebServer(80);

	// Set up the admin page
	webServer->on("/", std::bind(serveHome, webServer));
	webServer->onNotFound(std::bind(pageNotFound, webServer));
	webServer->begin();
	webserverProcess->status = WEBSERVER_OK;
	return WEBSERVER_OK;
}

int updateWebServer(struct process * webserverProcess)
{
	if (webserverProcess->status == WEBSERVER_OK)
	{
		webServer->handleClient();
	}

	return WEBSERVER_OK;
}

int stopWebserver(struct process * webserverProcess)
{
	webserverProcess->status = WEBSERVER_OFF;
	return WEBSERVER_OFF;
}

void webserverStatusMessage(struct process * webserverProcess, char * buffer, int bufferLength)
{
	switch (webserverProcess->status)
	{
	case WEBSERVER_OK:
		snprintf(buffer, bufferLength, "Web server OK");
		break;
	case WEBSERVER_OFF:
		snprintf(buffer, bufferLength, "Web server OFF");
		break;
	case WEBSERVER_ERROR_NO_WIFI:
		snprintf(buffer, bufferLength, "No Wifi for the web server");
		break;
	default:
		snprintf(buffer, bufferLength, "Web server status invalid");
		break;
	}
}
