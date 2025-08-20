// ConsoleApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <Magick++.h>

int main(int argc, char** argv)
{
	printf("Some text\n");

	Magick::InitializeMagick("");
	Magick::Image magickImage;

	try {
		magickImage.read("G:\\test\\test.psd");
	}
	catch (std::exception &ex){
		return E_FAIL;
	}

	magickImage.strip();
	try {
		magickImage.write("G:\\test\\test1.png");
	}
	catch (std::exception& ex) {
		return E_FAIL;
	}
	return S_OK;
}
