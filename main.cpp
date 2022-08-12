#include <thread>
#include <iostream>
#include <mutex>
#include <conio.h>
#include <random>

using namespace std;

constexpr int minimumProduceTime = 5000;
constexpr int maximumProduceTime = 15000;

constexpr int numberOfBeehives = 4;

atomic_bool honeyFull = false;

bool stopProgram = false;

mutex honeyMutex; 

int farmerStartTime = 0;
int farmerGatherTime = 7000;

thread farmerThread;
thread inputThread;

random_device randomDevice;
mt19937 rng(randomDevice());
uniform_int_distribution<std::mt19937::result_type> distributor(minimumProduceTime, maximumProduceTime);

struct Beehive
{
	thread produceThread;
	uint32_t startTime = 0;
	uint32_t produceTime = 0;

};

Beehive beehives[numberOfBeehives];

uint32_t GetTime()
{
	using namespace std::chrono;
	return static_cast<uint32_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
}

int GetRandomProduceTime()
{
	return distributor(rng);
}

void StartTimer(Beehive& beehive)
{
	beehive.produceTime = GetRandomProduceTime();
	beehive.startTime = GetTime();
}

void StartTimerForAllBeehives()
{
	for (int i = 0; i < numberOfBeehives; i++)
	{
		StartTimer(beehives[i]);
	}
}

bool HoneyReady(const Beehive& beehive)
{
	return GetTime() > beehive.startTime + beehive.produceTime;
}

void RunBehive(int index)
{
	while (!stopProgram)
	{
		if (HoneyReady(beehives[index]))
		{
			cout << "Bee waiting to drop off honey: " << index << endl;
			const lock_guard<std::mutex> lock(honeyMutex);

			while (honeyFull) {}

			cout << "Bee dropped off honey: " << index << endl;

			honeyFull = true;
			StartTimer(beehives[index]);
		}
	}
}

bool IsFarmerReady()
{
	return GetTime() > farmerStartTime + farmerGatherTime;
}

void FarmerChecksHoney()
{
	while (!stopProgram)
	{
		if (honeyFull && IsFarmerReady())
		{
			cout << "Farmer took the honey." << endl;
			honeyFull = false;
			farmerStartTime = GetTime();
		}
	}
}

void CreateBeehiveThreads()
{
	for (int i = 0; i < numberOfBeehives; i++)
	{
		beehives[i].produceThread = thread(RunBehive, i);
	}
}

void JoinThreads()
{
	farmerThread.join();

	for (int i = 0; i < numberOfBeehives; i++)
	{
		beehives[i].produceThread.join();
	}

	inputThread.join();
}

void ProcessInput()
{
	while (!stopProgram)
	{
		int stopInt = 0;
		cin >> stopInt;
		stopProgram = true;
	}
}

int main()
{
	farmerThread = thread(FarmerChecksHoney);

	StartTimerForAllBeehives();

	CreateBeehiveThreads();
	inputThread = thread(ProcessInput);

	JoinThreads();

	return 0;
}