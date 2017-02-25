#ifndef ITEM_H
#define ITEM_H

#include "Item.h"
#endif

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

string Item::getName() const {
	return m_name;
}

float Item::getPrice() const {
	return m_price;
}

int Item::getAmount() const {
	return m_amount;
}

int Item::getSold() const {
	return m_sold;
}

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

ostream &operator<<(ostream& out, const Item& i) {
	out << i.m_name << ", price: " << i.m_price << "e, amount: " << i.m_amount << ", sold: " << i.m_sold << endl;
	return out;
}