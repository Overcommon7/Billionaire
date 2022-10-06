#pragma once
#include "PlayerStore.h"
#include "doublyLinkedlist.h"
#include "Button.h"
#include "Utils.h"

extern DoublyLinkedList<Store> Stores;
extern float playerThreatForJailThread;
extern int numOfStores;

class Player			    
{
public:
	float posX = static_cast<float>(GetMouseX()), posY = static_cast<float>(GetMouseY()), threat = std::stof(GetLine(3, "PlayerStats"));
	Rectangle pointer = { posX, posY, 5.f, 5.f };
	ULL netWorth = std::stoull(GetLine(2, "PlayerStats")), lifeTimeEarning = std::stoull(GetLine(10, "PlayerStats"));
	long long money = std::stoll(GetLine(1, "PlayerStats")), timesClicked = std::stoll(GetLine(9, "PlayerStats"));
	int crystals = std::stoi(GetLine(4, "PlayerStats")), clicker = std::stoi(GetLine(5, "PlayerStats")), tier = std::stoi(GetLine(7, "PlayerStats"));
	short clickerLevel = std::stoi(GetLine(6, "PlayerStats"));
	vector<PlayerStore> stores;

	void CheckForSameBusiness()
	{
		this->stores.erase(std::unique(this->stores.begin(), this->stores.end()), this->stores.end());
		std::sort(this->stores.begin(), this->stores.end());
	}

	Player() 
	{
		int lines = GetNumberOfLines("PlayerStores");
		if (lines == 0)
		{
			stores.push_back(PlayerStore{ "NULL" });
			return;
		}
		stores.reserve(lines);
		for (int i = 1; i <= lines; i++)
		{
			stores.emplace_back(PlayerStore{i});
		}
		CheckForSameBusiness();
	}

	template<typename type>
	void UpdateMoney(type value)
	{
		if (value > 0) this->lifeTimeEarning += value;
		this->money += value;
		this->netWorth += value;
#pragma region DoubleIFs
		if (netWorth >= 1000000000000) {
			if (this->tier == 8)
			{
				this->tier = 9;
				LoadStores(&Stores, numOfStores, this->tier);
			}
		}
		else if (netWorth >= 5000000000) {
			if (this->tier == 7)
			{
				this->tier = 8;
				LoadStores(&Stores, numOfStores, this->tier);
			}
		}
		else if (netWorth >= 750000000) {
			if (this->tier == 6)
			{
				this->tier = 7;
				LoadStores(&Stores, numOfStores, this->tier);
			}
		}
		else if (netWorth >= 300000000) {
			if (this->tier == 5)
			{
				this->tier = 6;
				LoadStores(&Stores, numOfStores, this->tier);
			}
		}
		else if (netWorth >= 50000000) {
			if (this->tier == 4)
			{
				this->tier = 5;
				LoadStores(&Stores, numOfStores, this->tier);
			}
		}
		else if (netWorth >= 1000000) {
			if (this->tier == 3)
			{
				this->tier = 4;
				LoadStores(&Stores, numOfStores, this->tier);
			}
		}
		else if (netWorth >= 100000) {
			if (this->tier == 2)
			{
				this->tier = 3;
				LoadStores(&Stores, numOfStores, this->tier);
			}
		}
		else if (netWorth >= 10000) {
			if (this->tier == 1)
			{
				this->tier = 2;
				LoadStores(&Stores, numOfStores, this->tier);
			}
		}
		else if (tier < 2) this->tier = 1;

#pragma endregion
	}

	void UpdateClicker()
	{
		++this->clickerLevel;
		this->clicker *= 2;
	}

	//long long 

	void UpdatePosition() 
	{
		++this->timesClicked;
		this->posX = static_cast<float>(GetMouseX());
		this->posY = static_cast<float>(GetMouseY());
		this->pointer.x = this->posX;
		this->pointer.y = this->posY;
	}

	bool UpgadeStore(int storeIndex)
	{
#define st this->stores[storeIndex]

		if (this->money - st.upgradePrice < 0) return false;
		this->money -= st.upgradePrice;
		this->threat -= st.threatLevel;
		++st.level;
		if (st.threatLevel >= 0)
		{
			st.upgradePrice *= (1 + static_cast<double>((static_cast<double>(st.level) / 10))) * st.level;
			st.sellPrice = st.upgradePrice * 0.653;
		}
		else st.upgradePrice *= 1.5;
		st.buildTime = std::stod(GetSegmentFromLine("AllStores", GetLineFromWord("AllStores", st.name), 6)) * 3600;
		st.buildTime *= .5 + ((1 + static_cast<double>((static_cast<double>(st.level) / 5))) / 10);
		st.buildTime *= (1 + static_cast<double>((static_cast<double>(st.level) / 5)));
		st.earningPerSec *= (1 + static_cast<double>((static_cast<double>(st.level) / 10)));
		st.maxHold *= (1 + static_cast<double>((static_cast<double>(st.level) / 10)));
		if (st.threatLevel < 0)
		{
			if (st.name == "CommunitySpace") st.threatLevel *= 1.03;
			else if (st.name == "BotanicalGardens") st.threatLevel *= 1.05;
			else if (st.name == "CharityCenter") st.threatLevel *= 1.05;
			else if (st.name == "SearchAndRescue") st.threatLevel *= 1.045;
			else if (st.name == "WildlifeFoundation") st.threatLevel *= 1.0461;
			else if (st.name == "PublicLibrary") st.threatLevel *= 1.041;
			else if (st.name == "MuseumOfScience") st.threatLevel *= 1.0473;
			else if (st.name == "RecyclingMegaplex") st.threatLevel *= 1.32;
			else if (st.name == "BlueEnergyTower") st.threatLevel *= 1.47;
			else if (st.name == "NoahsArk") st.threatLevel *= 1.71;
			else if (st.name == "Eden") st.threatLevel -= 100;
		}
		else st.threatLevel *= 1.08;
		this->threat += st.threatLevel;
		return true;
	}

	void RemoveStore(int storeIndex)
	{
		this->money += st.sellPrice;
		this->netWorth -= st.sellPrice;
		this->lifeTimeEarning += st.sellPrice;
		this->threat -= st.threatLevel;
		playerThreatForJailThread = this->threat;
		this->stores.erase(this->stores.begin() + storeIndex);
		this->stores.shrink_to_fit();
		RemoveLine("PlayerStores", storeIndex + 1);
		if (this->stores.empty()) stores.push_back(PlayerStore{ "NULL" });
	}

	ULL AddStore(const Store& store)
	{
		//Check if the store has already been purchased
		short size = this->stores.size();
		if (size == 1 && this->stores[0].name == "NULL") this->stores.clear();
		else
		{
			for (short i = 0; i < size; i++)
			{
				if (store.name == this->stores[i].name)
				{
					cout << "Already Own!\n";
					return 1;
				}
			}
			if (this->money < store.price)
			{
				cout << "Insufficient Funds";
				return (this->money - store.price) * -1;
			}
		}

		this->stores.push_back(PlayerStore{ store.name });
		this->threat += static_cast<float>(this->stores[stores.size() - 1].threatLevel);
		playerThreatForJailThread = this->threat;
		this->money -= store.price;
		AddLine(GetLine(GetLineFromWord("Allstores", store.name), "AllStores"), "PlayerStores");
		cout << "Build Successful!\n";
		CheckForSameBusiness();
		return 0;
	}

	~Player() {}
};
