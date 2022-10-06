#pragma once
#include "pch.h"
#include "IO.h"


extern bool exitGame;

class Button
{
public:
	float posX, posY;
	Rectangle collider;
	Texture2D skin;
	bool multipurpose;

	Button(float x, float y, const char* fileName, bool mp = false) : posX(x), posY(y), multipurpose(mp)
	{
		Image Skin = LoadImage(fileName);
		skin = LoadTextureFromImage(Skin);
		collider = { x, y, static_cast<float>(skin.width), static_cast<float>(skin.height) };
		UnloadImage(Skin);
	}

	void UpdatePosition(float x, float y)
	{
		this->posX += x;
		this->posY += y;
		this->collider.x = this->posX;
		this->collider.y = this->posY;
	}

	void UpdateSkin(Texture2D& newSkin)
	{
		UnloadTexture(this->skin);
		this->skin = newSkin;
	}


	Button(const Button& b)
	{
		this->collider = b.collider; this->multipurpose = b.multipurpose; this->posX = b.posX; this->posY = b.posY; this->skin = b.skin;
	}
	~Button() {
		if (exitGame)
		{
			UnloadTexture(skin);
			cout << "button deleted\n";
		}
		else cout << "Button deleted skin active\n";
	}
};

class Store : public Button 
{
public:
	long long price;
	string name;

	Store(float x, float y, const char* fileName, float price, string name) : Button(x, y, fileName), price(price), name(name) {}
	Store(const Store& s) : Button(s) { this->price = s.price; this->name = s.name;  }
	~Store()
	{
		if (exitGame) UnloadTexture(skin); cout << "button deleted\n";
	}
};
