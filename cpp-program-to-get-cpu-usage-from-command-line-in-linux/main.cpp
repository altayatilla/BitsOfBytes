#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

const int NUM_CPU_STATES = 10;

enum CPUStates
{
	S_USER = 0,
	S_NICE,
	S_SYSTEM,
	S_IDLE,
	S_IOWAIT,
	S_IRQ,
	S_SOFTIRQ,
	S_STEAL,
	S_GUEST,
	S_GUEST_NICE
};

typedef struct CPUData
{
	std::string cpu;
	size_t times[NUM_CPU_STATES];
} CPUData;

void ReadStatsCPU(std::vector<CPUData> & entries);

size_t GetIdleTime(const CPUData & e1, const CPUData & e2);
size_t GetActiveTime(const CPUData & e1, const CPUData & e2);

void PrintStats(const std::vector<CPUData> & entries1, const std::vector<CPUData> & entries2);

int main(int argc, char * argv[])
{
	std::vector<CPUData> entries1;
	std::vector<CPUData> entries2;

	// snapshot 1
	ReadStatsCPU(entries1);

	// 100ms pause
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	// snapshot 2
	ReadStatsCPU(entries2);

	// print output
	PrintStats(entries1, entries2);

	return 0;
}

void ReadStatsCPU(std::vector<CPUData> & entries)
{
	std::ifstream fileStat("/proc/stat");

	std::string line;

	const std::string STR_CPU("cpu");
	const std::size_t LEN_STR_CPU = STR_CPU.size();
	const std::string STR_TOT("tot");

	while(std::getline(fileStat, line))
	{
		// cpu stats line found
		if(!line.compare(0, LEN_STR_CPU, STR_CPU))
		{
			std::istringstream ss(line);

			// store entry
			entries.emplace_back(CPUData());
			CPUData & entry = entries.back();

			// read cpu label
			ss >> entry.cpu;

			// remove "cpu" from the label when it's a processor number
			if(entry.cpu.size() > LEN_STR_CPU)
				entry.cpu.erase(0, LEN_STR_CPU);
			// replace "cpu" with "tot" when it's total values
			else
				entry.cpu = STR_TOT;

			// read times
			for(int i = 0; i < NUM_CPU_STATES; ++i)
				ss >> entry.times[i];
		}
	}
}

size_t GetIdleTime(const CPUData & e1, const CPUData & e2)
{
	return	(e2.times[S_IDLE] - e1.times[S_IDLE]) +
			(e2.times[S_IOWAIT] - e1.times[S_IOWAIT]);
}

size_t GetActiveTime(const CPUData & e1, const CPUData & e2)
{
	return	(e2.times[S_USER] - e1.times[S_USER]) +
			(e2.times[S_NICE] - e1.times[S_NICE]) +
			(e2.times[S_SYSTEM] - e1.times[S_SYSTEM]) +
			(e2.times[S_IRQ] - e1.times[S_IRQ]) +
			(e2.times[S_SOFTIRQ] - e1.times[S_SOFTIRQ]) +
			(e2.times[S_STEAL] - e1.times[S_STEAL]) +
			(e2.times[S_GUEST] - e1.times[S_GUEST]) +
			(e2.times[S_GUEST_NICE] - e1.times[S_GUEST_NICE]);
}

void PrintStats(const std::vector<CPUData> & entries1, const std::vector<CPUData> & entries2)
{
	const size_t NUM_ENTRIES = entries1.size();

	for(size_t i = 0; i < NUM_ENTRIES; ++i)
	{
		const CPUData & e1 = entries1[i];
		const CPUData & e2 = entries2[i];

		std::cout << e1.cpu << std::endl;

		const float ACTIVE_TIME		= static_cast<float>(GetActiveTime(e1, e2));
		const float IDLE_TIME		= static_cast<float>(GetIdleTime(e1, e2));
		const float TOTAL_TIME		= ACTIVE_TIME + IDLE_TIME;

		std::cout << "active: ";
		std::cout.setf(std::ios::fixed, std::ios::floatfield);
		std::cout.width(6);
		std::cout.precision(2);
		std::cout << (100.f * ACTIVE_TIME / TOTAL_TIME) << "%";

		std::cout << " - idle: ";
		std::cout.setf(std::ios::fixed, std::ios::floatfield);
		std::cout.width(6);
		std::cout.precision(2);
		std::cout << (100.f * IDLE_TIME / TOTAL_TIME) << "%" << std::endl;

		std::cout << std::endl;
	}
}
