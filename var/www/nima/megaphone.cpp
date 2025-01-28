#include <cctype>
#include <iostream>
#include <string>

std::string removeLeadingSpace(std::string str)
{
	unsigned int index = 0;

	while(index < str.length() && str[index] == ' ')
		index++;
	if (index == str.length())
		return ("");
	return (str.substr(index));
}

std::string removeTrailingSpace(std::string str)
{
	int index = str.length() - 1;

	while(index >= 0 && str[index] == ' ')
		index--;
	if (index == -1)
		return ("");
	return (str.substr(0, index + 1));
}

int main(int argc, char **argv) {
  if (argc < 2)
    std::cout << "* LOUD AND UNBEARABLE FEEDBACK NOISE *";
  else {
    for (int index = 1; index < argc; index++) {
      std::string str = argv[index];
	  str = removeLeadingSpace(str);
	  str = removeTrailingSpace(str);
	  int len = static_cast<int>(str.length());
      for (int i = 0; i < len; i++) {
		if (!isalpha(str[i]))
			std::cout << str[i];
		else
		{
			char c = static_cast<unsigned char>(str[i]);
			c = std::toupper(c);
			std::cout << c;
		}
      }
	  if (index < argc -1)
        std::cout << " ";
    }
  }
  std::cout << std::endl;
  return 0;
}
