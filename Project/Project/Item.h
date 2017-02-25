#pragma once
#include <iostream>
#include <string>
using namespace std;

class Item {
	private:
		string m_name;
		float m_price;
		int m_amount;
		int m_sold;
	public:
		Item(string name = "", float price = 0.0, int amount = 0, int sold = 0);
		Item(const Item& i);
		string getName() const;
		float getPrice() const;
		int getAmount() const;
		int getSold() const;
		float sell(int n);
		void add(int n);
		friend ostream &operator<<(ostream& out, const Item& i);
};