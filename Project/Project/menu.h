#pragma once
#ifndef ITEM_H
#define ITEM_H

#include "Item.h"
#endif

#ifndef STOCK_H
#define STOCK_H

#include "stock.h"
#endif

#include <iostream>
#include <string>
using namespace std;

class Menu {
	private:
		Stock *stock;
	public:
		Menu();
		~Menu();
		void printMenu();
		void handleCommand();
};