#include <ArduinoJson.h>

enum DistanceUnit
{
  METERS,
  CM,
};

struct TopicHCSR04
{
  long distance;
  DistanceUnit unit = METERS;
  TopicHCSR04(long distance)
  {
    this->distance = distance;
  }

  TopicHCSR04(long distance, DistanceUnit unit)
  {
    this->distance = distance;
    this->unit = unit;
  }

  String intoJSON()
  {
    JsonDocument doc;
    doc["distance"] = this->distance;
    doc["unit"] = this->unit;
    String buffer;
    serializeJson(doc,buffer);
    return buffer;
  }
};
