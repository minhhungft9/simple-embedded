#ifndef STOCK_H
#define STOCK_H

#include "stock.h"
#endif

Stock::Stock() {
	revenue = 0;
	list = new map<string, Item*>();
}

Stock::~Stock() {
	delete list;
}

void Stock::addItem(string name, float price, int amount) {
	map<string, Item*>::iterator find;
	find = list->find(name);
	if (find != list->end()) {	
		cout << "Stock already has the item." << endl;
	}
	else {
		(*list)[name] = new Item(name, price, amount);
	}
}

void Stock::Sell(string name, int n) {
	map<string, Item*>::iterator find;
	find = list->find(name);
	if (find != list->end()) {
		revenue = revenue + find->second->sell(n);
	}
	else {
		cout << "Can't find item!" << endl; 
	}
}

void Stock::Replenish(string name, int n) {
	map<string, Item*>::iterator find;
	find = list->find(name);
	if (find != list->end()) {
		find->second->add(n);
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
	for (map<string, Item*>::iterator i = list->begin(); i != list->end(); i++) {
		if (i->second->getName().find(n) != string::npos) {
			cout << (*i->second);
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
	cout << "Items in stock:" << endl;
	for (map<string, Item*>::iterator i = list->begin(); i != list->end(); i++) {
		cout << (*i->second);
	}
	cout << "Total revenue: " << revenue << "e" << endl;
}

void Stock::loadList() {
	delete list;
	list = new map<string, Item*>();
	string key, price, amount, sold, money;

	ifstream mykey("key.txt");
	ifstream myprice("price.txt");
	ifstream myamount("amount.txt");
	ifstream mysold("sold.txt");
	ifstream mymoney("money.txt");

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
	

void Stock::saveList() {
	ofstream mykey("key.txt");
	ofstream myprice("price.txt");
	ofstream myamount("amount.txt");
	ofstream mysold("sold.txt");
	ofstream mymoney("money.txt");

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