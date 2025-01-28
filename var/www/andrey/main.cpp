#include "Bureaucrat.hpp"
#include <memory>

int main ()
{
	try
	{
		Bureaucrat o1;
		std::cout << o1 << " is created" << std::endl;
		Bureaucrat o2 ("o2", 75);
		std::cout << o2 << " is created" << std::endl;
		// Bureaucrat* o3 = new Bureaucrat ("o3", 75);
		// std::cout << *o3 << " is created (raw pointer)" << std::endl;
		std::unique_ptr<Bureaucrat> o4 = std::make_unique<Bureaucrat> ("o4", 75);
		std::cout << *o4 << " is created (smart pointer)" << std::endl;
		o1.incrementGrade ();
		std::cout << "After increment: " << o1 << std::endl;
		o2.decrementGrade ();
		std::cout << "After decrement: " << o2 << std::endl;
		// o3->decrementGrade ();
		// std::cout << "After decrement: " << *o3 << std::endl;
		o4->incrementGrade ();
		std::cout << "After increment: " << *o4 << std::endl;
	}
	catch (std::exception const & e)
	{
		std::cerr << e.what () << std::endl;
	}
	std::cout << std::endl
		  << "Failure" << std::endl;
	try
	{
		Bureaucrat o5 ("o5", 0);
	}
	catch (std::exception const & e)
	{
		std::cerr << e.what () << std::endl;
	}
	try
	{
		Bureaucrat o6 ("o6", 151);
	}
	catch (std::exception const & e)
	{
		std::cerr << e.what () << std::endl;
	}
	try
	{
		Bureaucrat o7 ("o7", 150);
		o7.decrementGrade ();
	}
	catch (std::exception const & e)
	{
		std::cerr << e.what () << std::endl;
	}
	try
	{
		Bureaucrat o8 ("o8", 1);
		o8.incrementGrade ();
	}
	catch (std::exception const & e)
	{
		std::cerr << e.what () << std::endl;
	}
}
