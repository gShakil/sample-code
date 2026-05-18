#pragma once
#include <memory>

namespace nsdatabase::nspostgreSQL
{
	struct connectionInfo
	{
		std::string host; // ip
		int port;
		std::string dbname;
		std::string user;
		std::string password;
		int connect_timeout;
	};
	class postgresTester
	{
	public:
		bool connect(connectionInfo coninfo);
		void disconnect();
		bool isconnected() const { return m_pcx && m_pcx->is_open(); }

		bool createDatabase();
		bool dropDatabase();

		bool createTable();
		bool dropTable();

		bool insert_one_item();
		bool insert_five_item();
		bool delete_all_item();

		bool select_one_item();
		bool select_five_item();
		bool select_five_item_stream();
	protected:
		std::string make_dummy_query(int num_rows);

	private:
		void printerr(pqxx::failure const& e);
		std::shared_ptr<pqxx::connection> m_pcx;
	};
};




