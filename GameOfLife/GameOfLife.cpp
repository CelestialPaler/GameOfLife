// A simple implementation of the Game of Life.
// All the parameters can be changed in the code, and the speed can be adjusted using Numpad.
#include <iostream>
#include <vector>
#include <windows.h>
#include <random>
#include <chrono>
#include <thread>
#include <mutex>

// The width and height of the world space.
constexpr unsigned int width{ 20 };
constexpr unsigned int height{ 20 };
// The density of the world generation.
constexpr float density{ 0.5 };
// The data that stores the status of all the cells.
std::vector<std::vector<bool>> worldData;
std::vector<std::vector<bool>> worldDataBuffer;
// The mutex that makes sure that the data was accessed exclusively.
std::mutex worldDataMutex;
// The number of iterations.
int iteration{ 0 };
// The speed that the cells get updates.
int speed{ 1 };
// The seed that the world generation based on.
unsigned int seed{ 0 };
// The time comsuption of updating the cells.
std::chrono::milliseconds updateTime{ 0 };
// The main kill switch of the program.
bool enable = true;

// The rules that applied to the world.
bool Rule(const int x, const int y)
{
	int count = 0;
	// Calculate the number of alive neighbors.
	if (worldDataBuffer.at(y).at(x == 0 ? width - 1 : x - 1)) count++;
	if (worldDataBuffer.at(y).at(x == width - 1 ? 0 : x + 1)) count++;
	if (worldDataBuffer.at(y == 0 ? height - 1 : y - 1).at(x == 0 ? width - 1 : x - 1)) count++;
	if (worldDataBuffer.at(y == 0 ? height - 1 : y - 1).at(x)) count++;
	if (worldDataBuffer.at(y == 0 ? height - 1 : y - 1).at(x == width - 1 ? 0 : x + 1)) count++;
	if (worldDataBuffer.at(y == height - 1 ? 0 : y + 1).at(x == 0 ? width - 1 : x - 1)) count++;
	if (worldDataBuffer.at(y == height - 1 ? 0 : y + 1).at(x)) count++;
	if (worldDataBuffer.at(y == height - 1 ? 0 : y + 1).at(x == width - 1 ? 0 : x + 1)) count++;
	switch (count)
	{
	case 2: return worldDataBuffer.at(y).at(x); break;
	case 3: return true; break;
	default: return false; break;
	}
}

// Generate the world.
void Generate(void)
{
	seed = std::random_device()();
	std::default_random_engine randomEngine(seed);
	std::uniform_int_distribution<std::default_random_engine::result_type> random(0, 100);
	{
		std::lock_guard<std::mutex> lock(worldDataMutex);
		for (size_t y = 0; y < height; y++)
		{
			std::vector<bool> temp;
			for (size_t x = 0; x < width; x++)
				temp.push_back(random(randomEngine) < (density * 100));
			worldData.push_back(temp);
		}
		std::copy(worldData.begin(), worldData.end(), std::back_inserter(worldDataBuffer));
	}
}

// Render the world to the console.
void Render(void)
{
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { 0, 0 });
	std::string displayBuffer;
	{
		std::lock_guard<std::mutex> lock(worldDataMutex);
		for (size_t y = 0; y < height; y++)
		{
			for (size_t x = 0; x < width; x++)
				displayBuffer += (worldData.at(y).at(x) ? "¡ö" : "¡õ");
			displayBuffer += "\n";
		}
	}
	std::cout << displayBuffer << std::endl;
}

// Print the world info to the console.
void PrintInfo(void)
{
	std::cout << "Seed : " << seed
		<< "   Size : " << width << "x" << height
		<< "   Density : " << density
		<< "   Speed : " << speed
		<< "   Iter : " << iteration
		<< "   Update time : " << static_cast<unsigned int>(updateTime.count()) << " ms   "
		<< std::endl;
}

// Handle the keyboard events.
void HandleKeyInput(void)
{
	if (GetAsyncKeyState(VK_NUMPAD2) & 0x8000)
	{
		speed++;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	if (GetAsyncKeyState(VK_NUMPAD1) & 0x8000 && speed > 1)
	{
		speed--;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	if (GetAsyncKeyState(VK_NUMPAD3) & 0x8000)
		enable = false;
}

// Display all the stuff to the console.
void Display(void)
{
	while (enable)
	{
		Render();
		PrintInfo();
		std::this_thread::sleep_for(std::chrono::milliseconds((static_cast<int>(1.0 / 60.0))));
	}
}

// Update the state of all the cells.
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
		iteration++;
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