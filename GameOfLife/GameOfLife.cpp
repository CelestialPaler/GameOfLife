#include <iostream>
#include <vector>
#include <windows.h>
#include <random>
#include <chrono>
#include <thread>
#include <mutex>

// The width and height of the world space
constexpr unsigned int width = 20;
constexpr unsigned int height = 20;
// The data of 
std::vector<std::vector<bool>> worldData;
std::vector<std::vector<bool>> worldDataBuffer;
std::mutex worldDataMutex;
// The number of terations
int iter = 0;
constexpr int density = 5;
int speed = 1;
unsigned int seed = 0;
std::chrono::milliseconds updateTime;
bool enable = true;

// The rules that applied to the world
bool Rule(const int x, const int y)
{
	int count = 0;
	if (worldDataBuffer.at(y).at(x == 0 ? width - 1 : x - 1)) count++;
	if (worldDataBuffer.at(y).at(x == width - 1 ? 0 : x + 1)) count++;
	if (worldDataBuffer.at(y == 0 ? height - 1 : y - 1).at(x == 0 ? width - 1 : x - 1)) count++;
	if (worldDataBuffer.at(y == 0 ? height - 1 : y - 1).at(x)) count++;
	if (worldDataBuffer.at(y == 0 ? height - 1 : y - 1).at(x == width - 1 ? 0 : x + 1)) count++;
	if (worldDataBuffer.at(y == height - 1 ? 0  : y + 1).at(x == 0 ? width - 1 : x - 1)) count++;
	if (worldDataBuffer.at(y == height - 1 ? 0  : y + 1).at(x)) count++;
	if (worldDataBuffer.at(y == height - 1 ? 0  : y + 1).at(x == width - 1 ? 0 : x + 1)) count++;
	if (count == 3) return true;
	else if (count == 2) return worldDataBuffer.at(y).at(x);
	else return false;
}

// Generate the world
void Generate(void)
{
	std::default_random_engine randomEngine;
	seed = std::random_device()();
	randomEngine.seed(seed);
	std::uniform_int_distribution<std::default_random_engine::result_type> random(0, density);
	for (size_t j = 0; j < height; j++)
	{
		std::vector<bool> temp;
		for (size_t i = 0; i < width; i++)
			temp.push_back(random(randomEngine) == density);
		worldData.push_back(temp);
	}
	std::copy(worldData.begin(), worldData.end(), std::back_inserter(worldDataBuffer));
}

// Render the world to the console4
void Render(void)
{
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { 0, 0 });
	std::string displayBuffer;
	{
		std::lock_guard<std::mutex> lock(worldDataMutex);
		for (size_t j = 0; j < height; j++)
		{
			for (size_t i = 0; i < width; i++)
				displayBuffer += (worldData.at(j).at(i) ? "¡ö" : "¡õ");
			displayBuffer += "\n";
		}
	}
	std::cout << displayBuffer << std::endl;
}

void PrintInfo(void)
{
	std::cout << "Seed : " << seed
		<< "   Size : " << width << "x" << height
		<< "   Density : 1/" << density
		<< "   Speed : " << speed
		<< "   Iter : " << iter
		<< "   Update time : " << static_cast<unsigned int>(updateTime.count()) << " ms"
		<< std::endl;
}

void HandleKeyInput(void)
{
	if (GetAsyncKeyState(VK_NUMPAD2) & 0x8000)
	{
		speed++;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	else if (GetAsyncKeyState(VK_NUMPAD1) & 0x8000 && speed > 1)
	{
		speed--;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	else if (GetAsyncKeyState(VK_NUMPAD3) & 0x8000)
		enable = false;
}

// Display all the stuff to the console
void Display(void)
{
	while (enable)
	{
		Render();
		PrintInfo();
		std::this_thread::sleep_for(std::chrono::milliseconds((static_cast<int>(1.0 / 60.0))));
	}
}

// Update the state of all the cells
void Update(void)
{
	while (enable)
	{
		auto startTime = std::chrono::system_clock::now();
		{
			std::lock_guard<std::mutex> lock(worldDataMutex);
			std::swap(worldData, worldDataBuffer);
			for (size_t y = 0; y < height; y++)
				for (size_t x = 0; x < width; x++)
					worldData.at(y).at(x) = Rule(x, y);
		}
		auto endTime = std::chrono::system_clock::now();
		updateTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
		iter++;
		std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(1000.0 / speed)));
	}
}

int main(int argc, char* argv[]) 
{
	Generate();
	std::thread displayThread(Display);
	std::thread updateThread(Update);
	while (enable)
	{
		HandleKeyInput();
		std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(10)));
	}
	displayThread.join();
	updateThread.join();
	system("cls");
	return 0;
}