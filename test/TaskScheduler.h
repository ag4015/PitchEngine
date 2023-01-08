#pragma once
#include <iostream>
#include <mutex>
#include <queue>
#include "PitchEngineTs.h"

std::mutex getNextTaskMutex;

template<typename Task>
class TaskScheduler;

template<typename Task>
void runThread()
{
	std::optional<Task> task;
	do
	{
		task = TaskScheduler<Task>::getInstance()->getNextTask();
		if (task != std::nullopt)
		{
			std::apply(runPitchEngine, task.value());
		}
	} while (task != std::nullopt);

}

// A singleton class to represent the task scheduler
template<typename Task>
class TaskScheduler
{
public:
	static TaskScheduler* instance;
	static TaskScheduler* getInstance(const int numThreads = 1)
	{
		if (!instance)
		{
            instance = new TaskScheduler {numThreads};
		}
		return instance;
	}
	template<typename TaskType>
    void addTask(TaskType task)
    {
		tasks_.push(task);
    }
    std::optional<Task> getNextTask()
    {
		std::lock_guard<std::mutex> getNextTask(getNextTaskMutex);
		Task task;
		if (!tasks_.empty())
		{
			task = std::move(tasks_.front());
			tasks_.pop();
			return task;
		}
		else
		{
			return std::nullopt;
		}
    }
	void run()
	{
		for (int i = 0; i < numThreads_ && i < tasks_.size(); i++)
		{
			threads_.push(std::thread(&runThread<Task>));
		}
		while(threads_.size() > 0)
		{
			threads_.front().join();
			threads_.pop();
		}
	}
private:

	const int numThreads_;
	std::queue<Task> tasks_;
	std::queue<std::thread> threads_;

    TaskScheduler(const int numThreads) : numThreads_(numThreads) {}
    TaskScheduler(TaskScheduler const&) = delete;
    void operator=(TaskScheduler const&) = delete;

};

