#include <FastLED.h>

#include "pixels.h"
#include "settings.h"
#include "airquality.h"
#include "powercontrol.h"

struct PixelSettings pixelSettings;

boolean validateColour(void* dest, const char* newValueStr)
{
	int value;

	if (sscanf(newValueStr, "%d", &value) == 1)
	{
		*(int*)dest = value;
		return true;
	}

	if (value < 0)
		return false;

	if (value > 255)
		return false;

	return true;
}

void setDefaultPixelControlPinNo(void* dest)
{
	int* destInt = (int*)dest;
	*destInt = 26;
}

struct SettingItem pixelControlPinSetting = { "Pixel Control Pin",
		"pixelcontrolpin",
		&pixelSettings.pixelControlPinNo,
		NUMBER_INPUT_LENGTH,
		integerValue,
		setDefaultPixelControlPinNo,
		validateInt };

void setDefaultNoOfPixels(void* dest)
{
	int* destInt = (int*)dest;
	*destInt = 0;
}

struct SettingItem pixelNoOfPixelsSetting = { "Number of pixels (0 for pixels not fitted)",
		"noofpixels",
		&pixelSettings.noOfPixels,
		NUMBER_INPUT_LENGTH,
		integerValue,
		setDefaultNoOfPixels,
		validateInt };


void setDefaultPixelRed(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 0;
}

struct SettingItem pixelRedLevel = {
		"Pixel red (0-255)",
		"pixelred",
		&pixelSettings.pixelRed,
		NUMBER_INPUT_LENGTH,
		integerValue,
		setDefaultPixelRed,
		validateColour
};

void setDefaultPixelGreen(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 255;
}

struct SettingItem pixelGreenLevel = {
		"Pixel green (0-255)",
		"pixelgreen",
		&pixelSettings.pixelGreen,
		NUMBER_INPUT_LENGTH,
		integerValue,
		setDefaultPixelGreen,
		validateColour
};

void setDefaultPixelBlue(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 0;
}

struct SettingItem pixelBlueLevel = {
		"Pixel blue (0-255)",
		"pixelblue",
		&pixelSettings.pixelBlue,
		NUMBER_INPUT_LENGTH,
		integerValue,
		setDefaultPixelBlue,
		validateColour
};

void setDefaultAirqLowLimit(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 15;
}

struct SettingItem pixelAirQLowLimit = {
		"AirQ Low Limit",
		"airqlowlimit",
		&pixelSettings.airqLowLimit,
		NUMBER_INPUT_LENGTH,
		integerValue,
		setDefaultAirqLowLimit,
		validateInt,
};


void setDefaultAirqLowWarnLimit(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 40;
}

struct SettingItem pixelAirqLowWarnLimit = {
		"AirQ Low Warning Limit",
		"airqlowwarnlimit",
		& pixelSettings.airqLowWarnLimit,
		NUMBER_INPUT_LENGTH,
		integerValue,
		setDefaultAirqLowWarnLimit,
		validateInt,
};

void setDefaultAirqMidWarnLimit(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 65;
}

struct SettingItem pixelAirqMidWarnLimit = {
		"AirQ Mid Warning Limit",
		"airqmidwarnlimit",
		&pixelSettings.airqMidWarnLimit,
		NUMBER_INPUT_LENGTH,
		integerValue,
		setDefaultAirqMidWarnLimit,
		validateInt,
};


void setDefaultAirqHighWarnLimit(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 150;
}

struct SettingItem pixelAirqHighWarnLimit = {
		"AirQ High Warning Limit",
		"airqhighwarnlimit",
		&pixelSettings.airqHighWarnLimit,
		NUMBER_INPUT_LENGTH,
		integerValue,
		setDefaultAirqHighWarnLimit,
		validateInt
};


void setDefaultAirqHighAlertLimit(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 250;
}

struct SettingItem pixelAirqHighAlertLimit = {
		"AirQ High Alert Limit",
		"airqhighalertlimit",
		&pixelSettings.airqHighAlertLimit,
		NUMBER_INPUT_LENGTH,
		integerValue,
		setDefaultAirqHighAlertLimit,
		validateInt
};

struct SettingItem* pixelSettingItemPointers[] =
{
  &pixelControlPinSetting,
  &pixelNoOfPixelsSetting,
  &pixelAirQLowLimit,
  &pixelAirqLowWarnLimit,
  &pixelAirqMidWarnLimit,
  &pixelAirqHighWarnLimit,
  &pixelAirqHighAlertLimit
};

struct SettingItemCollection pixelSettingItems = {
	"pixel",
	"Settings for the NeoPixel output from the device",
	pixelSettingItemPointers,
	sizeof(pixelSettingItemPointers) / sizeof(struct SettingItem*)
};

// All the brightness values are between 0 and 1
// Scale them for the particular display

float overall_brightness = 1.0;

struct Light_Factor
{
	float factor_value, max, min, update;
	void(*do_update) (Light_Factor * factor);
};

#define CLOSE_TOLERANCE 0.01

inline bool close_to(float a, float b)
{
	float diff = a - b;
	if (diff > 0)
	{
		if (diff > CLOSE_TOLERANCE)
			return false;
		else
			return true;
	}
	else
	{
		if (diff < -CLOSE_TOLERANCE)
			return false;
		else
			return true;
	}
}

boolean coloursEqual(ColourValue a, ColourValue b)
{
	if (!close_to(a.r,b.r)) return false;
	if (!close_to(a.g,b.g)) return false;
	if (!close_to(a.b,b.b)) return false;
	return true;
}

void do_no_update(Light_Factor * target)
{
}


void do_update_larsen(Light_Factor * target)
{
	target->factor_value += target->update;
	if (target->factor_value > target->max)
	{
		target->factor_value = target->max;
		target->update = -target->update;
	}
	if (target->factor_value < target->min)
	{
		target->factor_value = target->min;
		target->update = -target->update;
	}
}

void do_update_loop(Light_Factor * target)
{
	target->factor_value += target->update;
	if (target->factor_value > target->max)
	{
		target->factor_value = target->min;
		return;
	}
	if (target->factor_value < target->min)
	{
		target->factor_value = target->max;
	}
}

void dump_light_factor(Light_Factor * factor)
{
	Serial.print("Value: ");
	Serial.print(factor->factor_value);
	Serial.print(" Max: ");
	Serial.print(factor->max);
	Serial.print(" Min: ");
	Serial.print(factor->min);
	Serial.print(" Update: ");
	Serial.print(factor->update);
}

#define RED_FACTOR 0
#define GREEN_FACTOR (RED_FACTOR+1)
#define BLUE_FACTOR (GREEN_FACTOR+1)
#define FLICKER_FACTOR (BLUE_FACTOR+1)
#define POSITION_FACTOR (FLICKER_FACTOR+1)
#define WIDTH_FACTOR (POSITION_FACTOR+1)
#define NO_OF_FACTORS (WIDTH_FACTOR+1)

char * factor_names[] = { "Red", "Green", "Blue", "Flicker", "Position", "Width" };

struct VirtualPixel
{
	Light_Factor factors[NO_OF_FACTORS];
};

struct VirtualPixel lamps[NO_OF_VIRTUAL_PIXELS];

void dumpVirtualPixel(VirtualPixel * lamp)
{
	for (int i = 0; i < NO_OF_FACTORS; i++)
	{
		Serial.print("   ");
		Serial.print(factor_names[i]);
		Serial.print(": ");
		dump_light_factor(&lamp->factors[i]);
		Serial.println();
	}
}

void dumpVirtualPixels(struct VirtualPixel * lamps)
{
	for (int i = 0; i < NO_OF_VIRTUAL_PIXELS; i++)
	{
		dumpVirtualPixel(&lamps[i]);
		Serial.println();
	}
}

struct Pixel
{
	byte r, g, b;
};

struct Pixel pixels[MAX_NO_OF_PIXELS];

// ColourValue struct is defined in utils.h

struct ColourValue color_list[] =
{
	RED_PIXEL_COLOUR,
	GREEN_PIXEL_COLOUR,
	BLUE_PIXEL_COLOUR,
	YELLOW_PIXEL_COLOUR,
	MAGENTA_PIXEL_COLOUR,
	CYAN_PIXEL_COLOUR,
	WHITE_PIXEL_COLOUR
};

void clear_pixels()
{
	for (int i = 0; i < pixelSettings.noOfPixels; i++)
	{
		pixels[i].r = 0;
		pixels[i].g = 0;
		pixels[i].b = 0;
	}
}

byte clamp_colour(int c)
{
	if (c > 255)
		return 255;
	if (c < 0)
		return 0;
	return c;
}

void add_color_to_pixel(int pos, int r, int g, int b)
{
	byte newr = clamp_colour((int)pixels[pos].r + r);
	byte newg = clamp_colour((int)pixels[pos].g + g);
	byte newb = clamp_colour((int)pixels[pos].b + b);

	pixels[pos].r = newr;
	pixels[pos].g = newg;
	pixels[pos].b = newb;
}

void renderSingleVirtualPixel(VirtualPixel * lamp)
{
	float brightness = lamp->factors[FLICKER_FACTOR].factor_value * MAX_BRIGHTNESS;

	// Map the position value from 360 degrees to a pixel number

	float pixel_pos = (lamp->factors[POSITION_FACTOR].factor_value / 360.0) * pixelSettings.noOfPixels;

	int pos = (int)(pixel_pos);

	float diff = pixel_pos - pos;

	float low_factor = 1 - diff;

	byte r = (byte)(lamp->factors[RED_FACTOR].factor_value * brightness * low_factor);
	byte g = (byte)(lamp->factors[GREEN_FACTOR].factor_value * brightness * low_factor);
	byte b = (byte)(lamp->factors[BLUE_FACTOR].factor_value * brightness * low_factor);

	add_color_to_pixel(pos, r, g, b);

	r = (byte)(lamp->factors[RED_FACTOR].factor_value * brightness * diff);
	g = (byte)(lamp->factors[GREEN_FACTOR].factor_value * brightness * diff);
	b = (byte)(lamp->factors[BLUE_FACTOR].factor_value * brightness * diff);

	add_color_to_pixel((pos + 1) % pixelSettings.noOfPixels, r, g, b);

}


void renderVirtualPixel(VirtualPixel* lamp)
{
	if (lamp->factors[WIDTH_FACTOR].factor_value <= 1)
	{
		renderSingleVirtualPixel(lamp);
		return;
	}

	float brightness = lamp->factors[FLICKER_FACTOR].factor_value * MAX_BRIGHTNESS;

	// Map the position value from 360 degrees to a pixel number

	float half_width = lamp->factors[WIDTH_FACTOR].factor_value / 2.0;
	float pos = lamp->factors[POSITION_FACTOR].factor_value;
	float left_pos = pos - half_width;
	float right_pos = pos + half_width;

	float left_pixel_pos = (left_pos / 360 * pixelSettings.noOfPixels);
	float right_pixel_pos = (right_pos / 360 * pixelSettings.noOfPixels);

	int left_int_pos = (int)(left_pixel_pos);
	int right_int_pos = (int)(right_pixel_pos);

	for (int i = left_int_pos; i <= right_int_pos; i++)
	{
		byte r = (byte)(lamp->factors[RED_FACTOR].factor_value * brightness);
		byte g = (byte)(lamp->factors[GREEN_FACTOR].factor_value * brightness);
		byte b = (byte)(lamp->factors[BLUE_FACTOR].factor_value * brightness);

		add_color_to_pixel(i, r, g, b);
	}

	float left_diff = left_pixel_pos - left_int_pos;

	float left_low_factor = 1 - left_diff;

	byte r = (byte)(lamp->factors[RED_FACTOR].factor_value * brightness * left_low_factor);
	byte g = (byte)(lamp->factors[GREEN_FACTOR].factor_value * brightness * left_low_factor);
	byte b = (byte)(lamp->factors[BLUE_FACTOR].factor_value * brightness * left_low_factor);

	add_color_to_pixel(left_int_pos - 1, r, g, b);

	r = (byte)(lamp->factors[RED_FACTOR].factor_value * brightness * left_diff);
	g = (byte)(lamp->factors[GREEN_FACTOR].factor_value * brightness * left_diff);
	b = (byte)(lamp->factors[BLUE_FACTOR].factor_value * brightness * left_diff);

	//	add_color_to_pixel((left_int_pos) % settings.noOfPixels, strip.gamma8(r), strip.gamma8(g), strip.gamma8(b));

	float right_diff = right_pixel_pos - right_int_pos;

	float right_low_factor = 1 - right_diff;

	r = (byte)(lamp->factors[RED_FACTOR].factor_value * brightness * right_diff);
	g = (byte)(lamp->factors[GREEN_FACTOR].factor_value * brightness * right_diff);
	b = (byte)(lamp->factors[BLUE_FACTOR].factor_value * brightness * right_diff);

	add_color_to_pixel(right_int_pos + 1, r, g, b);

	r = (byte)(lamp->factors[RED_FACTOR].factor_value * brightness * right_low_factor);
	g = (byte)(lamp->factors[GREEN_FACTOR].factor_value * brightness * right_low_factor);
	b = (byte)(lamp->factors[BLUE_FACTOR].factor_value * brightness * right_low_factor);

	//	add_color_to_pixel((right_int_pos) % settings.noOfPixels, strip.gamma8(r), strip.gamma8(g), strip.gamma8(b));
}


CRGB leds[20];

void setPixelFromStruct(int pixel, struct ColourValue colour)
{
	leds[pixel].setRGB(colour.r, colour.g, colour.b );
}

void renderVirtualPixels(struct VirtualPixel * lamps)
{
	clear_pixels();

	for (int i = 0; i < NO_OF_VIRTUAL_PIXELS; i++)
	{
		renderVirtualPixel(&lamps[i]);
	}

	for (int i = 0; i < pixelSettings.noOfPixels; i++)
	{
		// Serial.printf("p:%d r:%x g:%x b:%x  ", i, pixels[i].r, pixels[i].g, pixels[i].b);

		setPixelFromStruct(i, { pixels[i].r,pixels[i].g,pixels[i].b });

	}

	// Serial.println();
	FastLED.show();
}


void updateVirtualPixel(VirtualPixel * target)
{
	for (int i = 0; i < NO_OF_FACTORS; i++)
	{
		Light_Factor * factor = &target->factors[i];
		factor->do_update(factor);
	}
}

void updateVirtualPixels(struct VirtualPixel * lamps)
{
	for (int i = 0; i < NO_OF_VIRTUAL_PIXELS; i++)
	{
		updateVirtualPixel(&lamps[i]);
	}
	renderVirtualPixels(lamps);
}


void clear_factor(Light_Factor * target)
{
	target->factor_value = 0;
	target->max = 1.0;
	target->min = 0.0;
	target->update = 0.0;
	target->do_update = do_no_update;
}

void clearVirtualPixel(VirtualPixel * target)
{
	for (int i = 0; i < NO_OF_FACTORS; i++)
	{
		clear_factor(&target->factors[i]);
	}
}

void clearVirtualPixels(struct VirtualPixel * lamps)
{
	for (int i = 0; i < NO_OF_VIRTUAL_PIXELS; i++)
	{
		clearVirtualPixel(&lamps[i]);
	}
}

void setupVirtualPixel(VirtualPixel * target, float r, float g, float b, float pos, float width, float flicker)
{
	clearVirtualPixel(target);
	target->factors[RED_FACTOR].factor_value = r;
	target->factors[GREEN_FACTOR].factor_value = g;
	target->factors[BLUE_FACTOR].factor_value = b;
	target->factors[POSITION_FACTOR].factor_value = pos;
	target->factors[WIDTH_FACTOR].factor_value = width;
	target->factors[FLICKER_FACTOR].factor_value = flicker;
}

void setupVirtualPixelFactor(VirtualPixel * target, byte factor_number, float factor_value, float min, float max, float update, void(*do_update)(Light_Factor * f))
{
	target->factors[factor_number].factor_value = factor_value;
	target->factors[factor_number].max = max;
	target->factors[factor_number].min = min;
	target->factors[factor_number].update = update;
	target->factors[factor_number].do_update = do_update;
}

// If the pixels are illuminating a message these are the numbers of the 
// pixels that are actually in the message
// Need to change this is the message changes. 

void setupWalkingColour(ColourValue colour)
{
	if (pixelSettings.noOfPixels == 0)
		return;

	byte color_pos;
	float start_speed = 0.125;
//	float speed_update = 0.125;    // speed for the desktop sensor
	float speed_update = .25;      // speed for the top hat

	float degreesPerPixel = 360.0 / NO_OF_VIRTUAL_PIXELS;

	clearVirtualPixels(lamps);

	for (int i = 0; i < NO_OF_VIRTUAL_PIXELS; i++)
	{
		int pos = i * degreesPerPixel;
		// Serial.printf("Pixel: %d position:%d", i, pos);

		setupVirtualPixel(&lamps[i], colour.r, colour.g, colour.b, pos, 0, 1.0);
		setupVirtualPixelFactor(&lamps[i], POSITION_FACTOR, pos, 0, 359, start_speed, do_update_loop);
		start_speed += speed_update;
	}

	//dumpVirtualPixels(lamps);

	renderVirtualPixels(lamps);
}

float calculateStepSize(float start, float end, int noOfSteps)
{
	float range = end - start;
	return range / noOfSteps;
}

void do_update_fade(Light_Factor * target)
{
	target->factor_value += target->update;
	if (close_to(target->factor_value, target->max))
	{
		target->factor_value = target->max;
		target->do_update = do_no_update;
	}
}

void startFade(Light_Factor * factor, float targetValue, int noOfsteps)
{
	if (close_to(factor->factor_value, targetValue))
		return;

	factor->update = calculateStepSize(factor->factor_value, targetValue, noOfsteps);
	factor->max = targetValue;
	factor->do_update = do_update_fade;
}

void fadeWalkingColour(struct VirtualPixel * lamps, ColourValue newColour, int noOfSteps)
{
	for (int i = 0; i < NO_OF_VIRTUAL_PIXELS; i++)
	{
		startFade(&lamps[i].factors[RED_FACTOR], newColour.r, noOfSteps);
		startFade(&lamps[i].factors[GREEN_FACTOR], newColour.g, noOfSteps);
		startFade(&lamps[i].factors[BLUE_FACTOR], newColour.b, noOfSteps);
	}
}

void fadeWalkingColours(struct VirtualPixel * lamps, ColourValue * newColours, int noOfColours, int noOfSteps)
{
	for (int i = 0; i < NO_OF_VIRTUAL_PIXELS; i++)
	{
		int colourNo = i % noOfColours;
		startFade(&lamps[i].factors[RED_FACTOR], newColours[colourNo].r, noOfSteps);
		startFade(&lamps[i].factors[GREEN_FACTOR], newColours[colourNo].g, noOfSteps);
		startFade(&lamps[i].factors[BLUE_FACTOR], newColours[colourNo].b, noOfSteps);
	}
}

enum readingDisplayStates { veryLow, lowWarn, lowMid, lowHigh, highHigh, alert, sensorFailed, noAverage };

readingDisplayStates displayState;

readingDisplayStates getDisplayStateFromValue(float value)
{
	if (value < pixelSettings.airqLowLimit)
		return veryLow;

	if (value < pixelSettings.airqLowWarnLimit)
		return lowWarn;

	if (value < pixelSettings.airqMidWarnLimit)
		return lowMid;

	if (value < pixelSettings.airqHighWarnLimit)
		return lowHigh;

	if (value < pixelSettings.airqHighAlertLimit)
		return highHigh;

	return alert;
}

struct ColourValue badAirQReadingColours[] = { BLUE_PIXEL_COLOUR, RED_PIXEL_COLOUR };
struct ColourValue noAverageAirQReadingColours[] = { YELLOW_PIXEL_COLOUR, GREEN_PIXEL_COLOUR };

void setDisplayState(readingDisplayStates newState)
{
	displayState = newState;

	switch (displayState)
	{
	case veryLow:
		fadeWalkingColour(lamps, GREEN_PIXEL_COLOUR, 100);
		break;
	case lowWarn:
		fadeWalkingColour(lamps, YELLOW_PIXEL_COLOUR, 100);
		break;
	case lowMid:
		fadeWalkingColour(lamps, ORANGE_PIXEL_COLOUR, 100);
		break;
	case lowHigh:
		fadeWalkingColour(lamps, RED_PIXEL_COLOUR, 100);
		break;
	case highHigh:
		fadeWalkingColour(lamps, MAGENTA_PIXEL_COLOUR, 100);
		break;
	case alert:
		fadeWalkingColour(lamps, WHITE_PIXEL_COLOUR, 100);
		break;
	case sensorFailed:
		fadeWalkingColour(lamps, BLUE_PIXEL_COLOUR, 100);
//		fadeWalkingColours(lamps, badAirQReadingColours, sizeof(badAirQReadingColours) / sizeof(struct ColourValue),
//			100);
		break;
	case noAverage:
		fadeWalkingColours(lamps, noAverageAirQReadingColours, sizeof(noAverageAirQReadingColours) / sizeof(struct ColourValue),
			100);
		break;
	}
}

int previousAirqSensorStatus = -1;

void updateReadingDisplay()
{
	if (pixelSettings.noOfPixels == 0)
		return;

	readingDisplayStates newDisplayState;

	if (airqSensor.status == SENSOR_OK)
	{
		airqualityReading * sourceAirqReading = 
			(airqualityReading *)airqSensor.activeReading;

		unsigned long millisSinceAverage = ulongDiff(offsetMillis(), sourceAirqReading->lastAirqAverageMillis);

		if (millisSinceAverage < AIRQ_AVERAGE_LIFETIME_MSECS)
		{
			newDisplayState = getDisplayStateFromValue(sourceAirqReading->pm25Average);
		}
		else
		{
			newDisplayState = noAverage;
		}
	}
	else
	{
		newDisplayState = sensorFailed;
	}

	if (newDisplayState == displayState)
		return;

	setDisplayState(newDisplayState);
}

void setupWalkingMultipleColours()
{
	if (pixelSettings.noOfPixels == 0)
		return ;

	byte color_pos;
	float start_speed = 0.01;
	float speed_update = 0.005;

	clearVirtualPixels(lamps);

	for (int i = 0; i < 8; i++)
	{
		int cn = i % (sizeof(color_list) / sizeof(ColourValue));
		setupVirtualPixel(&lamps[i], color_list[cn].r, color_list[cn].g, color_list[cn].b, i, 0, 1.0);
		setupVirtualPixelFactor(&lamps[i], POSITION_FACTOR, i, 0, 359, start_speed, do_update_loop);
		start_speed += speed_update;
	}
}

int lastColour = -1;

void handleTransition(int targetColor)
{
	if (lastColour != targetColor)
	{
		fadeWalkingColour(lamps, color_list[targetColor], 100);
		lastColour = targetColor;
	}
}

unsigned long millisOfLastPixelUpdate;

void startPixelStrip() 
{
	FastLED.addLeds<NEOPIXEL, 26>(leds, 20);
}

// status display doesn't use the animated leds
// this means that it can overlay the display 

int statusPixelNo = 0;

void initialiseStatusDisplay(byte r, byte g, byte b)
{
	if (pixelSettings.noOfPixels == 0)
		return ;

	statusPixelNo = 0;

	for (int i = 0; i < pixelSettings.noOfPixels; i++)
	{
		setPixelFromStruct(i, { r,g,b });
	}

	renderStatusDisplay();
}


boolean setStatusDisplayPixel(int pixelNumber, boolean statusOK)
{
	if (pixelSettings.noOfPixels == 0)
		return false;

	if (pixelNumber >= pixelSettings.noOfPixels)
		return false;

	if (statusOK)
	{
		setPixelFromStruct(pixelNumber, { 0,255,0 });
	}
	else
	{
		setPixelFromStruct(pixelNumber, { 255,0,0 });
	}

	return true;
}


void beginStatusDisplay()
{
	initialiseStatusDisplay(0, 0, 10);
}

void beginWifiStatusDisplay()
{
	initialiseStatusDisplay(10, 0, 10);
}

void renderStatusDisplay()
{
	FastLED.show();
	delay(200);
}

boolean addStatusItem(boolean status)
{
	if (statusPixelNo >= pixelSettings.noOfPixels)
		return false;

	setStatusDisplayPixel(statusPixelNo, status);

	statusPixelNo++;

	return true;
}

int startPixel(struct process * pixelProcess)
{
	if (pixelSettings.noOfPixels == 0)
	{
		pixelProcess->status = PIXEL_ERROR_NO_PIXELS;
		return PIXEL_ERROR_NO_PIXELS;
	}

	startPixelStrip();

	millisOfLastPixelUpdate = offsetMillis();

	pixelProcess->status = PIXEL_OK;

	return PIXEL_OK;
}

void showDeviceStatus();  // declared in control.h
boolean getInputSwitchValue(); // declared in inputswitch.h

int updatePixel(struct process * pixelProcess)
{
	if (pixelSettings.noOfPixels == 0)
		return PIXEL_ERROR_NO_PIXELS;
	
	if (pixelProcess->status != PIXEL_OK)
	{
		return pixelProcess->status;
	}

	unsigned long currentMillis = offsetMillis();
	unsigned long millisSinceLastUpdate = ulongDiff(currentMillis, millisOfLastPixelUpdate);

	if (millisSinceLastUpdate >= MILLIS_BETWEEN_UPDATES) {
		updateReadingDisplay();
		updateVirtualPixels(lamps);
		millisOfLastPixelUpdate = currentMillis;
	}

	return pixelProcess->status;
}

int stopPixel(struct process * pixelProcess)
{
	pixelProcess->status = PIXEL_OFF;
	return PIXEL_OFF;
}

void pixelStatusMessage(struct process * pixelProcess, char * buffer, int bufferLength)
{
	if(pixelSettings.noOfPixels==0)
	{
		snprintf(buffer, bufferLength, "No pixels connected");
		return;
	}

	switch (pixelProcess->status)
	{
	case PIXEL_OK:
		snprintf(buffer, bufferLength, "PIXEL OK");
		break;
	case PIXEL_OFF:
		snprintf(buffer, bufferLength, "PIXEL OFF");
		break;
	default:
		snprintf(buffer, bufferLength, "Pixel status invalid");
		break;
	}
}


