#pragma once

#ifndef _DATABASESESSION_HPP_
#define _DATABASESESSION_HPP_
#include <cassandra.h>
#include <string>
#include <iostream>

typedef CassUuid Guid;

class CassString {
private:
	const char* text;
	size_t length = 0;
public:
	void Load(const CassValue* val) {
		cass_value_get_string(val, &text, &length);
	}
	static void Load(std::string& str, const CassValue* val) {
		CassString cstr;
		cstr.Load(val);
		str.assign(cstr.text, cstr.length);
	}
};


class CassandraUtils {
public:
	static std::string GetCassandraError(CassFuture* future) {
		const char* message = NULL;
		size_t message_length = 0;
		cass_future_error_message(future, &message, &message_length);

		if (message == nullptr || message_length == 0) {
			return "";
		}
		else {
			std::string stringMsg;

			stringMsg.assign(message, message_length);

			return stringMsg;
		}
	}
};
class DatabaseSession {

private:
	CassCluster* CreateCluster(char* host);
	CassError ConnectSession(CassSession* session, const CassCluster* cluster);
	CassCluster* m_cluster;
	CassSession* m_session;
	char* m_host;

public:
	DatabaseSession(char* host);
	DatabaseSession();
	CassCluster* Cluster();
	CassSession* Session();
	~DatabaseSession();
	static char* DefaultSession;

};

#endif