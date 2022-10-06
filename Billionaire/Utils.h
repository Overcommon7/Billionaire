#pragma once
#include "pch.h"
#include "Button.h"
#include "doublyLinkedlist.h"

extern bool newTier;
bool firstLoad = true;

void AddSpace(string& str)
{
    const int length = str.length();
    for (short i = 2; i < length; i++)
    {
        if (str[i - 1] != ' ' && str[i] == std::toupper(str[i]))
        {
            str.insert(str.begin() + (i), ' ');
            if (i + 1 < length) ++i;
            else return;
        }
    }
}

short Random(short num1 = 100, short num2 = 0)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	short t = gen();
	if (t < 0) t *= -1;
	return (t % num1) + num2;
}

void delay(short delay = 1000)
{
	clock_t start_time = clock();
	while (clock() < start_time + delay);
}

void GetStoreSkinFromName(string StoreName, DoublyLinkedList<Store>& sto, Image& skin)
{
	for (short i = 0; i < sto.size(); i++)
	{
		if (StoreName == sto[i].name)
		{
			skin = LoadImageFromTexture(sto[i].skin);
			return;
		}
	}
}

long long GetElapsedTime()
{
    time_t then;
    std::fstream File("time.txt");
    File >> then;
    File.close();
    time_t now = time(0);
    return static_cast<long long>((now - then));
}

template<typename type>
string ConvertToTime(type frames, bool inFrames = true)
{
	type buildSeconds;
	if (inFrames) buildSeconds = (frames / 60);
	else buildSeconds = frames;

	long minutes = 0;
	int hours = 0;
	short days = 0;

	while (buildSeconds > 59)
	{
		++minutes;
		buildSeconds -= 60;
	}

	while (minutes > 59)
	{
		++hours;
		minutes -= 60;
	}

	while (hours > 23)
	{
		++days;
		hours -= 24;
	}

	string returnString = "";
	if (days > 0) returnString += std::to_string(days) + "D ";
	if (hours > 0) returnString += std::to_string(hours) + "Hr ";
	if (minutes > 0) returnString += std::to_string(minutes) + "Min ";
	returnString += std::to_string(buildSeconds) + "Sec";

	return returnString;
}

vector<vector<int>> GetNumberOfStorePerTier(int tier)
{
	vector<vector<int>> retvec;
	const int numberOfLines = GetNumberOfLines("AllStores");
	int temp = 1, num = 0, total = 0, y = 266;
	for (short i = 1; i <= numberOfLines; i++)
	{
		if (temp < std::stoi(GetSegmentFromLine("AllStores", i, 9)))
		{
			int oz = 0;
		 	retvec.push_back({num, y});
			if (num % 2 == 1) oz = 1;
			y = 266 + ((total / 2) + oz) * 270;
			num = 0;
			if (temp < tier) ++temp;
			else return retvec;
		}
		++num;
		++total;
	}
	return retvec;
}

void LoadStores(DoublyLinkedList<Store>* str, int& numOfStores, int tier)
{
	if (firstLoad) firstLoad = false;
	else newTier = true;
	str->clear();
	float tempY = 266, tempX = 75;
	int tempTier = 1, temp = 0;
	numOfStores = 0;
	string tempName = "", filePath = "";
	const int numberOfLines = GetNumberOfLines("AllStores");
	vector<vector<int>>	repeats = GetNumberOfStorePerTier(tier);

	for (short j = 0; j < repeats.size(); j++)
	{
		for (short i = 0; i < repeats[j][0]; i++)
		{
			++numOfStores;			
			tempName = GetSegmentFromLine("Allstores", numOfStores, 1);
			filePath = ("Stores/" + tempName + ".png");
			str->push_back(Store{tempX, tempY, filePath.c_str(), std::stof(GetSegmentFromLine("Allstores", numOfStores, 5)), tempName});
			tempY += 270;
			if (i == (repeats[j][0] / 2) - 1)
			{
				tempY = repeats[j][1];
				tempX += 300;
			}	
		}
		tempX = 75;
	}
}
