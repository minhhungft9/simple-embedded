#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>      // std::stringstream

using namespace std;

int main()
{
	string line;
	ifstream file("Text.txt");
	if (file.is_open()) {
		while (getline(file, line))
		{
			cout << line << endl;

			string token;

			vector<double> tokens;
			double readNumber;
			double sum = 0;

			istringstream tokenStream(line);
			while (getline(tokenStream, token, ','))
			{
				readNumber = stof(token);
				tokens.push_back(readNumber);
			}
			vector<double>::size_type tokensSize = tokens.size();
			for (unsigned i = 0; i < tokensSize; i++) {
				sum = sum + tokens[i];
			}
			double average = sum / tokensSize;
			cout << "Average: " << average << endl;
			sum = 0;
			average = 0;
		}
		file.close();
	}
	else
	{
		cout << "Can not open the file." << endl;
	}

	cin.get();
	return 0;
}