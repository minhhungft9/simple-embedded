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
	cout << "	3. Replenish stock" << endl;
	cout << "	4. Find items" << endl;
	cout << "	5. Load list of item" << endl;
	cout << "	6. Save list of item" << endl;
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
			float price;	
			int amount;
			while (true)
			{
				cout << "Enter price (float): ";
				cin >> price;
				if (cin.fail()) {
					cout << "Price must be a float!" << endl;
					cin.clear();
					cin.ignore(numeric_limits<streamsize>::max(), '\n');
				}
				else {
					break;
				}
			}
			while (true)
			{
				cout << "Enter amount (integer): ";
				cin >> amount;
				if (cin.fail()) {
					cout << "Amount must be an integer!" << endl;
					cin.clear();
					cin.ignore(numeric_limits<streamsize>::max(), '\n');
				}
				else {
					break;
				}
			}
			stock->addItem(item, price, amount);
			this->printMenu();
		}

		else if (command == "2") {
			cout << "What item: ";
			string item;
			getline(cin, item);
			int number;
			while (true)
			{
				cout << "Enter amount you want to sell: ";
				cin >> number;
				if (cin.fail()) {
					cout << "Amount must be an integer!" << endl;
					cin.clear();
					cin.ignore(numeric_limits<streamsize>::max(), '\n');
				}
				else {
					break;
				}
			}
			stock->Sell(item, number);
			this->printMenu();
		}

		else if (command == "5") {
			//stock->loadList();
			stock->loadMoney();
			ifstream myStock("myStock.txt");
			if (myStock.is_open()) {
				myStock >> *stock;
				myStock.close();
				cout << "List is loaded from disk." << endl;
			}
			else {
				cout << "Unable to open file";
			}
			this->printMenu();
		}

		else if (command == "6") {
			//stock->saveList();
			stock->saveMoney();
			ofstream myStock("myStock.txt");
			if (myStock.is_open()) {
				myStock << *stock;
				myStock.close();
				cout << "List is saved to disk." << endl;
			}
			else {
				cout << "Unable to open file";
			}
			this->printMenu();
		}

		else if (command == "3") {
			cout << "What item: ";
			string item;
			getline(cin, item);
			int number;
			while (true)
			{
				cout << "Enter amount you want to replenish: ";
				cin >> number;
				if (cin.fail()) {
					cout << "Amount must be an integer!" << endl;
					cin.clear();
					cin.ignore(numeric_limits<streamsize>::max(), '\n');
				}
				else {
					break;
				}
			}
			stock->Replenish(item, number);
			this->printMenu();
		}

		else if (command == "4") {
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