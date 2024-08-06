#pragma once
class ThreadPoolUsage
{
public:
	void SampleExec_Type1(const size_t& nThreadCount, const size_t& nRepeatCount);
	void SampleExec_Type2(const size_t& nThreadCount, const size_t& nRepeatCount);
	void SampleExec_Type3(const size_t& nThreadCount, const size_t& nRepeatCount);

protected:
	int WorkFunction(int nId, int nSec);
private:

};

