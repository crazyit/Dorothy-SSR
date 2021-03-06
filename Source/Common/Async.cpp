/* Copyright (c) 2016 Jin Li, http://www.luvfight.me

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "Const/Header.h"
#include "Common/Async.h"

NS_DOROTHY_BEGIN

Async Async::FileIO;
Async Async::Process;

Async::~Async()
{
	if (_thread.isRunning())
	{
		Async::cancel();
		_workerEvent.post("Stop"_slice);
		_workerSemaphore.post();
		_thread.shutdown();
	}
}

void Async::run(function<void*()> worker, function<void(void*)> finisher)
{
	if (!_thread.isRunning())
	{
		_thread.init(Async::work, this);
		SharedDirector.getSystemScheduler()->schedule([this](double deltaTime)
		{
			DORA_UNUSED_PARAM(deltaTime);
			Own<QEvent> event = _finisherEvent.poll();
			if (event)
			{
				Package package;
				void* result;
				EventQueue::retrieve(event, package, result);
				package.second(result);
			}
			return false;
		});
	}
	auto package = std::make_pair(worker, finisher);
	_workerEvent.post("Work"_slice, package);
	_workerSemaphore.post();
}

int Async::work(void* userData)
{
	Async* worker = r_cast<Async*>(userData);
	while (true)
	{
		for (Own<QEvent> event = worker->_workerEvent.poll();
			event != nullptr;
			event = worker->_workerEvent.poll())
		{
			switch (Switch::hash(event->getName()))
			{
				case "Work"_hash:
				{
					Package package;
					EventQueue::retrieve(event, package);
					void* result = package.first();
					worker->_finisherEvent.post(Slice::Empty, package, result);
					break;
				}
				case "Stop"_hash:
				{
					return 0;
				}
			}
		}
		worker->_pauseSemaphore.post();
		worker->_workerSemaphore.wait();
	}
	return 0;
}

void Async::pause()
{
	if (_thread.isRunning())
	{
		for (Own<QEvent> event = _workerEvent.poll();
			event != nullptr;
			event = _workerEvent.poll())
		{
			Package package;
			EventQueue::retrieve(event, package);
			_packages.push_back(package);
		}
		_workerSemaphore.post();
		_pauseSemaphore.wait();
	}
}

void Async::resume()
{
	if (_thread.isRunning() && !_packages.empty())
	{
		for (const auto& package : _packages)
		{
			_workerEvent.post("Work"_slice, package);
		}
		_packages.clear();
		_workerSemaphore.post();
	}
}

void Async::cancel()
{
	for (Own<QEvent> event = _workerEvent.poll();
		event != nullptr;
		event = _workerEvent.poll());
}

NS_DOROTHY_END
