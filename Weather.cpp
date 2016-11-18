//
// Created by ziyang on 11/17/16.
//
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <curl.h>
#include "pugixml.hpp"


#include "Weather.h"

size_t AppendDataToStringCurlCallback(void *ptr, size_t size, size_t nmemb, void *vstring) {
	std::string *pstring = (std::string *) vstring;
	pstring->append((char *)ptr, size * nmemb);
	return size * nmemb;
}

bool Weather::GetWeatherFromNatWeatherService() {

	CURL *curl_handle;
	const std::string url_ann_arbor = "http://forecast.weather.gov/MapClick.php?textField1=42.28&textField2=-83.74&FcstType=dwml";

	curl_global_init(CURL_GLOBAL_ALL);

	//init the curl session 
	curl_handle = curl_easy_init();

	//set URL to get here 
	curl_easy_setopt(curl_handle, CURLOPT_URL, url_ann_arbor.c_str());

	//Switch on full protocol/debug output while testing 
	curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 0L);

	//disable progress meter, set to 0L to enable and disable debug output 
	curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0L);

	//set user agent 
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "Mozilla/5.0");

	//write data to string 
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, AppendDataToStringCurlCallback);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &body_);

	//get it! 
	curl_easy_perform(curl_handle);

	//cleanup curl stuff 
	curl_easy_cleanup(curl_handle);

	return true;
}

bool Weather::ParseXml() {

	//load string into XML DOM
	pugi::xml_parse_result result = xmldoc_.load(body_.c_str());

	//check if there are errors
	if (result) {
		std::cout << "XML parsed without errors" << std::endl;
	}
	else {
		std::cout << "XML parsed with errors" << std::endl;
		std::cout << "Error description: " << result.description() << "\n";
	}

	return true;
}

bool Weather::GetCurrentWeather() {

	//this is the top level node
	pugi::xml_node rootnode = xmldoc_.child("dwml");

	//search all child for current observations
	for (pugi::xml_node it = rootnode.first_child(); it; it = it.next_sibling()) {

		if (strcmp(it.attribute("type").as_string(), "current observations") == 0) {

			//read current temperature
			ReadXmlChildValue(weather_data_.temperature, it.child("parameters").child("temperature").child("value"));

			//read current weather conditions
			weather_data_.weather_conditions = it.child("parameters")
				.child("weather").child("weather-conditions")
				.attribute("weather-summary").as_string();

			//read sustained wind speed
			ReadXmlChildValue(weather_data_.wind_speed, it.child("parameters")
				.child("wind-speed").next_sibling().child("value"));

			//done reading
			break;
		}
	}

	return true;
}

bool Weather::GetForecastWeather()
{
	//this is the top level node
	pugi::xml_node rootnode = xmldoc_.child("dwml");

	//search all child for forecast
	for (pugi::xml_node it = rootnode.first_child(); it; it = it.next_sibling()) {

		if (strcmp(it.attribute("type").as_string(), "forecast") == 0) {

			//search for max and min temperature from identical <temperature> tag
			for (pugi::xml_node it2 = it.child("parameters").child("temperature"); it2; it2 = it2.next_sibling())
			{
				//read the first value for min temp for the day
				if (strcmp(it2.attribute("type").as_string(), "minimum") == 0)
					ReadXmlChildValue(weather_data_.min_temperature, it2.child("value"));

				//read the first value for max temp for the day
				if (strcmp(it2.attribute("type").as_string(), "maximum") == 0)
					ReadXmlChildValue(weather_data_.max_temperature, it2.child("value"));
			}
		}
	}

	return true;
}

bool Weather::PrintCurrentWeather() {
	std::cout << weather_data_.temperature << std::endl
		<< weather_data_.weather_conditions << std::endl
		<< weather_data_.wind_speed << std::endl
		<< weather_data_.min_temperature << std::endl
		<< weather_data_.max_temperature << std::endl;
}