#ifndef ITEM_H
#define ITEM_H

#include "Item.h"
#endif

#include <iomanip>

Item::Item(string name, float price, int amount, int sold) {
	m_name = name;
	m_price = price;
	m_amount = amount;
	m_sold = sold;
}

Item::Item(const Item& i) {
	m_name = i.m_name;
	m_price = i.m_price;
	m_amount = i.m_amount;
	m_sold = i.m_sold;
}

string Item::getName() const {return m_name;}

float Item::getPrice() const {return m_price;}

int Item::getAmount() const {return m_amount;}

int Item::getSold() const {return m_sold;}

float Item::sell(int n) {
	if (n > m_amount) {
		cout << "Not enough amount for sale!" << endl;
		return 0;
	}
	else {
		m_amount = m_amount - n;
		m_sold = m_sold + n;
		float money = m_price * n;
		cout << n << " " << m_name << " are sold." << endl;
		return money;
	}
}

void Item::add(int n) {
	if (n < 0) {
		cout << "Replenish amount must bigger than 0!" << endl;
	}
	else {
		m_amount = m_amount + n;
		cout << n << " " << m_name << " are added to the stock." << endl;
	}
}

void Item::printItem() {
	cout << setw(20) << m_name << setw(15) << m_price << setw(15) << m_amount << setw(15) << m_sold << endl;
}

void Item::setName(string name) {m_name = name;}

void Item::setPrice(float price) {m_price = price;}

void Item::setAmount(int amount) {m_amount = amount;}

void Item::setSold(int sold) {m_sold = sold;}

ostream &operator<<(ostream& out, const Item& i) {
	out << i.m_name << endl << i.m_price << endl << i.m_amount << endl << i.m_sold << endl;
	return out;
}

istream &operator >> (istream& in, Item& i) {
	in >> i.m_name;
	in >> i.m_price;
	in >> i.m_amount;
	in >> i.m_sold;
	//in.ignore(1024, '\n');
	return in;
}