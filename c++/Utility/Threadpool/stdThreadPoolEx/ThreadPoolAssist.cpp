#include "pch.h"
#include "ThreadPoolAssist.h"

namespace __ENF
{
	namespace Threadpool
	{
		ThreadPoolAssist::ThreadPoolAssist(size_t num_threads)
			: num_threads_(num_threads), stop_all(false) {
			worker_threads_.reserve(num_threads_);
			for (size_t i = 0; i < num_threads_; ++i) {
				worker_threads_.emplace_back([this]() { this->WorkerThread(); });
			}
		}

		void ThreadPoolAssist::WorkerThread() {
			while (true) {
				std::unique_lock<std::mutex> lock(m_job_q_);
				cv_job_q_.wait(lock, [this]() { return !this->jobs_.empty() || stop_all; });
				if (stop_all && this->jobs_.empty()) {
					return;
				}

				std::function<void()> job = std::move(jobs_.front());
				jobs_.pop();
				lock.unlock();

				job();
			}
		}

		ThreadPoolAssist::~ThreadPoolAssist() {
			stop_all = true;
			cv_job_q_.notify_all();

			for (auto& t : worker_threads_) {
				t.join();
			}
		}
	}
}