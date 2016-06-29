#ifndef _DATABASESESSION_CPP_
#define _DATABASESESSION_CPP_
#include "DatabaseSession.hpp"
#include <iostream>
#include <exception>
#include <stdexcept>
#include <sstream>

CassCluster* DatabaseSession::CreateCluster(char* host) {
	CassCluster* cluster = cass_cluster_new();
	cass_cluster_set_contact_points(cluster, host);
	return cluster;
}

CassError DatabaseSession::ConnectSession(CassSession* session, const CassCluster* cluster) {
	CassError rc = CASS_OK;
	CassFuture* future = cass_session_connect(session, cluster);

	cass_future_wait(future);
	rc = cass_future_error_code(future);
	cass_future_free(future);

	return rc;
}

DatabaseSession::DatabaseSession(char* host) {

	m_cluster = cass_cluster_new();
	m_session = cass_session_new();

	/* Add contact points */
	cass_cluster_set_contact_points(m_cluster, host);

	/* Provide the cluster object as configuration to connect the session */
	CassFuture* connect_future = cass_session_connect(m_session, m_cluster);

	CassError rc = cass_future_error_code(connect_future);

	if (rc != CASS_OK) {
		cass_cluster_free(m_cluster);
		cass_session_free(m_session);

		std::string err = "Couldn't connect to the Cassandra Database: "
			+ CassandraUtils::GetCassandraError(connect_future);

		cass_future_free(connect_future);

		throw err;

	}
/*

	m_host = host;
	m_cluster = CreateCluster(host);
	m_session = cass_session_new();

	CassFuture* future = cass_session_connect(m_session, m_cluster);

	CassError rc = cass_future_error_code(future);
	cass_future_free(future);


	if (rc != CASS_OK) {
		cass_cluster_free(m_cluster);
		cass_session_free(m_session);
		cass_future_free(future);
		throw "Couldn't connect to the Cassandra Database"
			+ CassandraUtils::GetCassandraError(future);
	}
	cass_future_free(future);

	*/
}

DatabaseSession::DatabaseSession() : DatabaseSession(DatabaseSession::DefaultSession) { }

char* DatabaseSession::DefaultSession = nullptr;

CassCluster* DatabaseSession::Cluster() {
	return m_cluster;
}

CassSession* DatabaseSession::Session() {
	return m_session;
}

DatabaseSession::~DatabaseSession() {
	cass_cluster_free(m_cluster);
	cass_session_free(m_session);
}
#endif
