#ifndef STOCK_H
#define STOCK_H

#include "stock.h"
#endif

#include <iomanip>

Stock::Stock() {
	revenue = 0;
	list = new map<string, Item>();
}

Stock::~Stock() {
	delete list;
}

void Stock::addItem(string name, float price, int amount) {
	Item i;
	map<string, Item>::iterator find;
	find = list->find(name);
	if (find != list->end()) {	
		cout << "Stock already has the item." << endl;
	}
	else {
		i.setName(name);
		i.setPrice(price);
		i.setAmount(amount);
		(*list)[name] = i;
	}
}

void Stock::Sell(string name, int n) {
	map<string, Item>::iterator find;
	find = list->find(name);
	if (find != list->end()) {
		revenue = revenue + find->second.sell(n);
	}
	else {
		cout << "Can't find item!" << endl; 
	}
}

void Stock::Replenish(string name, int n) {
	map<string, Item>::iterator find;
	find = list->find(name);
	if (find != list->end()) {
		find->second.add(n);
	}
	else {
		cout << "Can't find item!" << endl;
	}
}

void Stock::Clear() {
	list->clear();
}

void Stock::Find(string n) {
	cout << "Result:" << endl;
	int result = 0;
	cout << left << setw(20) << "ITEM" << left << setw(15) << "PRICE(euro)" << left << setw(15) << "AMOUNT" << left << setw(15) << "SOLD\n" << endl;
	for (map<string, Item>::iterator i = list->begin(); i != list->end(); i++) {
		if (i->second.getName().find(n) != string::npos) {
			i->second.printItem();
			++result;
		}
	}
	if (result == 0)
	{
		cout << "Not found!" << endl;
	}
	result = 0;
}

void Stock::printList() {
	cout << left << setw(20) << "ITEM" << left << setw(15) << "PRICE(euro)" << left << setw(15) << "AMOUNT" << left << setw(15) << "SOLD\n" << endl;
	for (map<string, Item>::iterator i = list->begin(); i != list->end(); i++) {
		i->second.printItem();
	}
	cout << "\nTotal revenue: " << revenue << "e" << endl;
}

void Stock::loadMoney() {
	ifstream mymoney("money.txt");
	string money;
	if (mymoney.is_open()) {
		getline(mymoney, money);
		if (money.empty()) {
			revenue = 0;
		}
		else
		{
			revenue = stof(money);
		}
	}
	/*delete list;
	list = new map<string, Item*>();
	string key, price, amount, sold, money;

	ifstream mykey("key.txt");
	ifstream myprice("price.txt");
	ifstream myamount("amount.txt");
	ifstream mysold("sold.txt");
	ifstream mymoney("money.txt");

	if (mykey.is_open() && myprice.is_open() && myamount.is_open() && mysold.is_open() && mymoney.is_open()) {
		getline(mymoney, money);
		if (money.empty()) {
			revenue = 0;
		}
		else
		{
			revenue = stof(money);
		}

		while (getline(mykey, key), getline(myprice, price), getline(myamount, amount), getline(mysold, sold))
		{
			(*list)[key] = new Item(key, stof(price), stoi(amount), stoi(sold));
		}

		cout << "Loaded list from disk." << endl;
	}
	else {
		cout << "Unable to open file." << endl;
	}*/	

	/*
	delete list;
	list = new map<string, Item>();
	Item i;
	ifstream myStock("myStock.txt");
	if (myStock.is_open()) {
		while (myStock >> i) {
			(*list)[i.getName()] = i;
		}
		myStock.close();	
		cout << "List is loaded from disk." << endl;
	}
	else {
		cout << "Unable to open file";
	}*/
}

void Stock::saveMoney() {
	ofstream mymoney("money.txt");
	mymoney << revenue;
	
	/*
	ofstream myStock("myStock.txt");
	if (myStock.is_open()) {
		for (map<string, Item>::iterator i = list->begin(); i != list->end(); i++) {
			myStock << i->second;
		}
		myStock.close();
		cout << "List is saved to disk." << endl;
	}
	else {
		cout << "Unable to open file";
	}*/

	/*ofstream mykey("key.txt");
	ofstream myprice("price.txt");
	ofstream myamount("amount.txt");
	ofstream mysold("sold.txt");
	ofstream mymoney("money.txt");
	if (mykey.is_open() && myprice.is_open() && myamount.is_open() && mysold.is_open() && mymoney.is_open()) {
		for (map<string, Item*>::iterator i = list->begin(); i != list->end(); i++) {
			mykey << i->first << endl;
		}

		for (map<string, Item*>::iterator i = list->begin(); i != list->end(); i++) {
			myprice << i->second->getPrice() << endl;
		}

		for (map<string, Item*>::iterator i = list->begin(); i != list->end(); i++) {
			myamount << i->second->getAmount() << endl;
		}

		for (map<string, Item*>::iterator i = list->begin(); i != list->end(); i++) {
			mysold << i->second->getSold() << endl;
		}

		mymoney << revenue;
		cout << "Saved list to disk." << endl;
	}
	else {
		cout << "Unable to open file." << endl;
	}*/
}

ostream &operator<<(ostream& out, const Stock& s) {
	for (map<string, Item>::iterator i = s.list->begin(); i != s.list->end(); i++) {
		out << i->second;
	}
	return out;
}

istream &operator >> (istream& in, Stock& s) {
	Item i;
	delete s.list;
	s.list = new map<string, Item>();
	while (in >> i) {
		(*s.list)[i.getName()] = i;
	}
	return in;
}