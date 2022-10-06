#pragma once
#include "IO.h"
#include "Utils.h"

string buffer = "";
extern bool exitGame, noStores;
extern long long timeElapsed;
short storeCounter = 0;

class PlayerStore
{
public:
	string name;
	long double earningPerSec, currentcapacity = 0; 
	long long maxHold, sellPrice, upgradePrice, tempNum, buildTime;
	short level, tier, order = storeCounter;
	float threatLevel;
	Texture2D skin;

	bool operator<(const PlayerStore& s) { return s.order > this->order; }
	bool operator==(const PlayerStore& s) { return this->name == s.name; }
	
	PlayerStore(string b_name)
	{
		++storeCounter;
		if (b_name == "NULL")
		{
			name = "NULL";
			earningPerSec = 0, currentcapacity = 0;
			maxHold = 0, sellPrice = 0, upgradePrice = 0, tempNum = 0, buildTime = 0;
			threatLevel = 0, level = 0, tier = 0;
			Image Null = LoadImage("PlayerStores/NULL.png");
			noStores = true;
			skin = LoadTextureFromImage(Null);
			UnloadImage(Null);
		}
		else
		{
			std::stringstream ss(GetLine(GetLineFromWord("AllStores", b_name), "AllStores"));
			short counter = 1;

			while (ss >> buffer)
				Initialize(counter);
		}	
	}

	PlayerStore(int lineNumber)
	{	
		++storeCounter;
		short counter = 1;
		std::stringstream ss(GetLine(lineNumber, "PlayerStores"));
		
		while (ss >> buffer)
			Initialize(counter);

		currentcapacity = std::stold(GetSegmentFromLine("PlayerStores", lineNumber, 8)) + (earningPerSec * static_cast<long double>(timeElapsed));
		if (currentcapacity > maxHold) currentcapacity = maxHold;

		long long tempLL = buildTime - (timeElapsed * 360);
		if (tempLL > 0) buildTime = tempLL;
		else buildTime = 0;
	}

	PlayerStore(PlayerStore&& p) noexcept
	{
		cout << "Move Constructor\n";
		this->buildTime = p.buildTime; this->earningPerSec = p.earningPerSec; this->level = p.level; this->maxHold = p.maxHold; 
		this->name = p.name; this->sellPrice = p.sellPrice; this->threatLevel = p.threatLevel; this->upgradePrice = p.upgradePrice;
		this->skin = p.skin; this->currentcapacity = p.currentcapacity;	this->tier = p.tier; this->tempNum = p.tempNum;
	}

	void operator=(const PlayerStore& p)
	{
		this->buildTime = p.buildTime; this->earningPerSec = p.earningPerSec; this->level = p.level; this->maxHold = p.maxHold;
		this->name = p.name; this->sellPrice = p.sellPrice; this->threatLevel = p.threatLevel; this->upgradePrice = p.upgradePrice;
		this->skin = p.skin; this->currentcapacity = p.currentcapacity; this->tier = p.tier; this->tempNum = p.tempNum;
	}

	PlayerStore(const PlayerStore& p)
	{
		this->buildTime = p.buildTime; this->earningPerSec = p.earningPerSec; this->level = p.level; this->maxHold = p.maxHold; 
		this->name = p.name; this->sellPrice = p.sellPrice; this->threatLevel = p.threatLevel; this->upgradePrice = p.upgradePrice;
		this->skin = p.skin; this->currentcapacity = p.currentcapacity;	this->tier = p.tier; this->tempNum = p.tempNum;
	}

	~PlayerStore() 
	{
		if (exitGame)
		{
			UnloadTexture(skin);
		}
	}

	string ConvertDataToString()
	{
		return this->name + ' ' + 
			std::to_string(this->earningPerSec * 60) + ' ' + 
			std::to_string(this->maxHold) + ' ' + 
			std::to_string(this->threatLevel) + ' ' + 
			std::to_string(this->tempNum) + ' ' + 
			std::to_string(this->buildTime / 3600) + ' ' + 
			std::to_string(this->level) + ' ' + 
			std::to_string(this->currentcapacity) + ' ' + 
			std::to_string(this->tier);
	}

	/*friend std::ostream& operator << (std::ostream& os, const PlayerStore& other)
	{
		os << other.name << ' ' << other.buildTime << ' ' << other.level;
		return os;
	}*/

private:
	void Initialize(short& counter)
	{
		if (counter == 1)
		{
			name = buffer;
			if (name == "NULL") noStores = true;
			else noStores = false;
			Image tempIm = LoadImage(("PlayerStores/" + name + ".png").c_str());
			skin = LoadTextureFromImage(tempIm);
			UnloadImage(tempIm);
		}
		else if (counter == 2) earningPerSec = std::stod(buffer) * 0.01666666666666667;
		else if (counter == 3) maxHold = std::stoull(buffer);
		else if (counter == 4) threatLevel = std::stof(buffer);
		else if (counter == 5) tempNum = std::stold(buffer);
		else if (counter == 6) buildTime = std::stoull(buffer) * 3600;
		else if (counter == 7)
		{
			level = std::stoi(buffer);
			if (threatLevel >= 0)
			{
				upgradePrice = tempNum / 2;
				sellPrice = static_cast<long long>(upgradePrice * 0.653);
			}
			else
			{
				upgradePrice = tempNum / 2;
				upgradePrice /= (1 + static_cast<double>((static_cast<double>(level) / 10))) * level;
				sellPrice = 0;
			}
		}
		else if (counter == 9) tier = std::stoi(buffer);
		++counter;
	}
};
