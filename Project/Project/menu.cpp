#ifndef MENU_H
#define MENU_H

#include "menu.h"
#endif

Menu::Menu() {
	stock = new Stock();
}

Menu::~Menu() {
	delete stock;
}

void Menu::printMenu() {
	cout << "-----------------------------" << endl;
	cout << "Stock / Choose option:" << endl;
	cout << "	1. Add new item" << endl;
	cout << "	2. Sell an item" << endl;
	cout << "	3. Load list of item" << endl;
	cout << "	4. Save list of item" << endl;
	cout << "	5. Replenish stock" << endl;
	cout << "	6. Find items" << endl;
	cout << "	7. Print items and revenue" << endl;
	cout << "	8. Clear list" << endl;
	cout << "	x. Exit" << endl;
	cout << "-----------------------------" << endl;
}

void Menu::handleCommand() {
	while (true) {
		string command;
		cout << "Enter command: ";
		cin >> command;
		cin.ignore();
		if (command == "1") {
			string item;
			cout << "What item: ";
			getline(cin, item);
			cout << "Enter price: ";
			float price;
			cin >> price;
			cout << "Enter amount: ";
			int amount;
			cin >> amount;
			stock->addItem(item, price, amount);
			this->printMenu();
		}
		else if (command == "2") {
			cout << "What item: ";
			string item;
			getline(cin, item);
			cout << "Enter amount you want to sell: ";
			int number;
			cin >> number;
			stock->Sell(item, number);
			this->printMenu();
		}
		else if (command == "3") {
			stock->loadList();
			this->printMenu();
		}
		else if (command == "4") {
			stock->saveList();
			this->printMenu();
		}
		else if (command == "5") {
			cout << "What item: ";
			string item;
			getline(cin, item);
			cout << "Enter amount you want to replenish: ";
			int number;
			cin >> number;
			stock->Replenish(item, number);
			this->printMenu();
		}
		else if (command == "6") {
			string item;
			cout << "Enter keyword: ";
			getline(cin, item);
			stock->Find(item);
			this->printMenu();
		}
		else if (command == "7") {
			stock->printList();
			this->printMenu();
		}
		else if (command == "8") {
			stock->Clear();
			cout << "All items are cleared." << endl;
			this->printMenu();
		}
		else if (command == "x") {
			break;
		}
	}
}