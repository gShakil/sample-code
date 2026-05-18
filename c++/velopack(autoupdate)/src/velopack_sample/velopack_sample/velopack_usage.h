#pragma once
class velopack_usage
{
public:
	void get_entry();
	bool update();
	bool downgrade();

	void printversion();
private:
	void progress_callback(size_t progress);
};

