#include "pch.h"
#include "postgresTester.h"
#include <string>

namespace nsdatabase::nspostgreSQL
{
	bool postgresTester::connect(connectionInfo coninfo)
	{
		try
		{
			if (!m_pcx)
			{
				std::string constring = std::format("host={0} port={1} connect_timeout={2} user={3} password={4} dbname='{5}'"
					, coninfo.host
					, coninfo.port
					, coninfo.connect_timeout
					, coninfo.user
					, coninfo.password
					, coninfo.dbname
				);
				m_pcx = std::make_shared<pqxx::connection>(constring.c_str());
				m_pcx->set_client_encoding("EUC_KR"); // 없으면 printerr에서 한글이 깨져서 출력된다.
				//m_pcx = std::make_shared<pqxx::connection>("host=localhost port=5432 connect_timeout=10 user=postgres password=vhtmsus12!");
			}
		}
		catch (pqxx::failure& e)
		{
			printerr(e);
			return false;
		}

		return m_pcx->is_open();
	}

	void postgresTester::disconnect()
	{
		try
		{
			if (m_pcx)
				m_pcx->close();
			m_pcx.reset();
		}
		catch (pqxx::failure & e)
		{
			printerr(e);
		}
	}
	bool postgresTester::createDatabase()
	{
		if (!isconnected())
			return false;
		try
		{
			pqxx::nontransaction tx{ *m_pcx };
			tx.exec("CREATE DATABASE sampledatabase WITH OWNER = postgres ENCODING = 'UTF8' LC_COLLATE = 'Korean_Korea.949' LC_CTYPE = 'Korean_Korea.949' LOCALE_PROVIDER = 'libc' TABLESPACE = pg_default CONNECTION LIMIT = -1 IS_TEMPLATE = False;");
			//tx.commit(); // nontransaction 은 필요없음.

			return true;
		}
		catch (pqxx::failure & e)
		{
			printerr(e);
		}


		return false;
	}
	bool postgresTester::dropDatabase()
	{
		try
		{
			pqxx::nontransaction tx{ *m_pcx };
			tx.exec("DROP DATABASE IF EXISTS sampledatabase WITH (FORCE);"); // WITH (FORCE);가 있다면 다른 세션이 접속해있더라도 모든 세션을 종료하고 db를 삭제한다.
			return true;
		}
		catch(pqxx::failure& e)
		{
			printerr(e);
		}
		return false;
	}
	bool postgresTester::createTable()
	{
		try
		{
			if (isconnected())
			{
				pqxx::work tx{ *m_pcx };
				tx.exec("CREATE TABLE IF NOT EXISTS sampletable (name varchar, salary integer);");
				tx.commit();
				return true;
			}
		}
		catch (pqxx::failure& e)
		{
			printerr(e);
		}

		return false;
	}
	bool postgresTester::dropTable()
	{
		try
		{
			if (isconnected())
			{
				pqxx::work tx{ *m_pcx };
				tx.exec("DROP TABLE IF EXISTS sampletable");
				tx.commit();
				return true;
			}
		}
		catch (pqxx::failure& e)
		{
			printerr(e);
		}

		return false;
	}
	bool postgresTester::insert_one_item()
	{
		try
		{
			if (isconnected())
			{
				pqxx::work tx{ *m_pcx };
				tx.exec("INSERT INTO sampletable(name, salary) VALUES ('Someone0', 0)");
				tx.commit();
				return true;
			}
		}
		catch (pqxx::failure& e)
		{
			printerr(e);
		}

		return false;
	}
	bool postgresTester::insert_five_item()
	{
		try
		{
			if (isconnected())
			{
				pqxx::work tx{ *m_pcx };
				tx.exec("INSERT INTO sampletable(name, salary) VALUES \
					('Someone1', 1),\
					('Someone2', 2),\
					('Someone3', 3),\
					('Someone4', 4),\
					('Someone5', 5)");
				tx.commit();
				return true;
			}
		}
		catch (pqxx::failure& e)
		{
			printerr(e);
		}
		return false;
	}
	bool postgresTester::delete_all_item()
	{
		try
		{
			if (isconnected())
			{
				pqxx::work tx{ *m_pcx };
				tx.exec("delete from sampletable");
				tx.commit();
				return true;
			}
		}
		catch (pqxx::failure& e)
		{
			printerr(e);
		}
		return false;
	}
	bool postgresTester::select_one_item()
	{
		try
		{
			pqxx::work tx{ *m_pcx };
			pqxx::result r{ tx.exec("SELECT name, salary FROM sampletable where salary=1") };
			for (auto row : r)
			{
				TRACE("[pqxx Error] name: %s - (column by name)\n", row["name"].c_str());
				TRACE("[pqxx Error] salary: %d - (column by index)\n", row[1].as<int>());
			}
			// Not really needed, since we made no changes, but good habit to be
			// explicit about when the transaction is done.
			tx.commit();
			return true;
		}
		catch (pqxx::failure& e)
		{
			printerr(e);
		}


		return false;
	}
	bool postgresTester::select_five_item()
	{
		try
		{
			pqxx::work tx{ *m_pcx };
			pqxx::result r{ tx.exec("SELECT name, salary FROM sampletable where salary<=5") };
			for (auto row : r)
			{
				TRACE("[pqxx Error] name: %s - (column by name)\n", row["name"].as<std::string_view>().data()); // std::string_view로 읽으면 메모리 할당이 더 적음.
				TRACE("[pqxx Error] salary: %d - (column by index)\n", row[1].as<int>());
			}

			TRACE("=============================================================\n");

			r.for_each([](std::string_view name, int number) {
				TRACE("[pqxx Error] name: %s - (column by name)\n", name.data()); // std::string_view로 읽으면 메모리 할당이 더 적음.
				TRACE("[pqxx Error] salary: %d - (column by index)\n", number);
				});

			// Not really needed, since we made no changes, but good habit to be
			// explicit about when the transaction is done.
			tx.commit();
			return true;
		}
		catch (pqxx::failure& e)
		{
			printerr(e);
		}
		return false;
	}
	bool postgresTester::select_five_item_stream()
	{
		// But all these are just the first way of querying data.  It reads all the
		// result data from the database server and returns an object representing
		// all that data.
		//
		// There is another way: _streaming._  This does not work for all queries;
		// it does not accept parameters, for instance.  Due to some constant
		// overhead it's also likely to be a bit _slower_ for small result sets.
		// But it gets much faster for larger result sets.  The actual numbers
		// depend on your individual use-case, so when performance is crucial,
		// measure what works best for you.
		//
		// Why does streaming tend to be faster?  There are several reasons:
		// 1. You can start processing the first rows before the query even
		// finishes.
		// 2. It bypasses calls to the underlying C library, libpq.
		// 3. Fewer memory allocations and deallocations are needed.
		// 4. Encoding support in libpqxx has very little overhead.
		//
		// When streaming a query, you specify as what types you want to read the
		// respective columns, as template arguments.

		try
		{
			pqxx::work tx{ *m_pcx };
			for (auto const [name, number] :
				tx.stream<std::string_view, int>(make_dummy_query(20)))
			{
				TRACE("[pqxx Error] name: %s - (column by name)\n", name.data());
				TRACE("[pqxx Error] salary: %d - (column by index)\n", number);
			}

			TRACE("=============================================================\n");

			tx.for_stream(
				make_dummy_query(20), [](std::string_view name, int number) {
					TRACE("[pqxx Error] name: %s - (column by name)\n", name.data());
					TRACE("[pqxx Error] salary: %d - (column by index)\n", number);
				});

			return true;
		}
		catch (pqxx::failure& e)
		{
			printerr(e);
		}
		return false;
	}
	std::string postgresTester::make_dummy_query(int num_rows)
	{
		/// Generate a query that produces `num_rows` rows of data.
		/** Each row will consist of a string and an integer.
		*/

		return std::format(
			"SELECT ('name' || n), n FROM generate_series(1, {}) n", num_rows);
	}
	void postgresTester::printerr(pqxx::failure const& e)
	{
		TRACE("\n[pqxx Error] %s: %s", e.name().data(), e.what());
		TRACE("\n[pqxx Error] Happened in %s", pqxx::source_loc(e.location()).c_str());
		if (e.query().empty() == false)
			TRACE("\n[pqxx Error] query is : %s", e.query().data());
		if(e.sqlstate().empty() == false)
			TRACE("\n[pqxx Error] SQLSTATE is : %s", e.sqlstate().data());
	}
}















