#include <array>
#include <chrono>
#include <execution>
#include <iostream>
#include <random>
#include <thread>

constexpr int c_nTrials          = 1000000;
constexpr int c_nMaxThreads      = 20;
constexpr int c_nTrialsPerThread = c_nTrials / c_nMaxThreads;

constexpr int c_nParalysisTurnsNeeded = 177;

struct CParalysisTurns
{
	int nMaxParalysisTurns;
	int nRollsForMaxParalysisTurns;
};

CParalysisTurns SimulateParalysisTurns(int iThread)
{
	constexpr int c_nDieSides = 4;

	// Use std::mt19937 for random number generation
	std::random_device randomDevice;
	std::mt19937 randomNumberGenerator(randomDevice());
	std::uniform_int_distribution<> uniformDistribution(0, c_nDieSides - 1);

	int nRolls = std::min(c_nTrialsPerThread, c_nTrials - iThread * c_nTrialsPerThread);

	CParalysisTurns paralysisTurns{
		.nMaxParalysisTurns         = 0,
		.nRollsForMaxParalysisTurns = nRolls,
	};
	for (int iRoll = 0; iRoll < nRolls; ++iRoll)
	{
		// Generate random draws for all turns
		constexpr int c_nTurns = 231;
		std::array<int, c_nTurns> arr_randomDraws;
		for (int& iRandomDraw : arr_randomDraws)
			iRandomDraw = uniformDistribution(randomNumberGenerator);

		int nParalysisTurns = std::count(arr_randomDraws.begin(), arr_randomDraws.end(), 0);

		if (nParalysisTurns > paralysisTurns.nMaxParalysisTurns)
			paralysisTurns.nMaxParalysisTurns = nParalysisTurns;

		if (nParalysisTurns >= c_nParalysisTurnsNeeded)
		{
			paralysisTurns.nRollsForMaxParalysisTurns = iRoll + 1;
			break;
		}
	}

	return paralysisTurns;
}

void SimulateGraveler()
{
	if (c_nMaxThreads > std::thread::hardware_concurrency())
		throw std::invalid_argument(std::format("Number of threads {} exceeds maximum of {} allowed", c_nMaxThreads, std::thread::hardware_concurrency()));

	std::array<int, c_nMaxThreads> arr_iThreads;
	std::iota(arr_iThreads.begin(), arr_iThreads.end(), 0); // Fill the array with values 0 to 19

	std::array<CParalysisTurns, c_nMaxThreads> arr_resultsPerThread;
	std::for_each(std::execution::par_unseq, arr_iThreads.begin(), arr_iThreads.end(),
				  [&arr_resultsPerThread](int iThread) {
					  CParalysisTurns threadResult  = SimulateParalysisTurns(iThread);
					  arr_resultsPerThread[iThread] = threadResult;
				  });

	int nMaxParalysisTurns = 0;
	int nRolls             = c_nTrials;
	for (int i = 0; i < static_cast<int>(arr_resultsPerThread.size()); ++i)
	{
		int nThreadMaxParalysisTurns = arr_resultsPerThread[i].nMaxParalysisTurns;
		if (nThreadMaxParalysisTurns > nMaxParalysisTurns)
			nMaxParalysisTurns = arr_resultsPerThread[i].nMaxParalysisTurns;

		if (nThreadMaxParalysisTurns >= c_nParalysisTurnsNeeded)
		{
			nRolls = arr_resultsPerThread[i].nRollsForMaxParalysisTurns + c_nTrialsPerThread * i;
			break;
		}
	}
	std::cout << "Highest Ones Roll:" << nMaxParalysisTurns << '\n';
	std::cout << "Number of Roll Sessions: " << nRolls << std::endl;
}

int main()
{
	auto start = std::chrono::high_resolution_clock::now();

	SimulateGraveler();

	auto end                               = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> duration = end - start;

	double dSeconds = duration.count();
	int nMinutes    = static_cast<int>(dSeconds) / 60;
	int nHours      = nMinutes / 60;
	int nDays       = nHours / 24;
	dSeconds -= (static_cast<int>(dSeconds) / 60) * 60;

	std::cout << std::format("Time taken: {} days, {} hours, {} minutes and {:.3f} seconds", nDays, nHours, nMinutes, dSeconds) << std::endl;

	return 0;
}
