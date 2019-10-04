#include "Average.h"
#include <Arduino.h>

void Average::setNoOfValuesToAverage(int no)
{
	NoOfValuesToAverage = no;
	restartAverage();
}

bool Average::addToAverage(float value)
{
	if (NoOfValuesToAverage == averageCount)
		return false;

	total += value;
	averageCount++;

	return true;
}

bool Average::averageReady()
{
	return NoOfValuesToAverage == averageCount;
}

bool Average::getAverage(float *result)
{
	if (!averageReady())
		return false;

	*result = total / averageCount;

	restartAverage();

	return true;
}

void Average::restartAverage()
{
	total = 0;
	averageCount = 0;
}

Average::Average(int NoOfValuesToAverage)
{
	setNoOfValuesToAverage(NoOfValuesToAverage);
}

Average::~Average()
{
}
