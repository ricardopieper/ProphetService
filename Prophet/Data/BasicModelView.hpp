#pragma once
#ifndef _BASICMODELVIEW_HPP_
#define _BASICMODELVIEW_HPP_
#include <vector>
#include <map>
#include <cassandra.h>
#include <iostream>
#include <string>
#include "DatabaseSession.hpp"

class BasicModelView {
private:	
	Guid m_modelId;
	std::map<std::string, double> m_averages;
public:

	Guid Id() { return m_modelId; }
	std::map<std::string, double> Averages() { return m_averages; }


	void SetId(Guid id) {
		m_modelId = id;
	}
	void SetAverages(std::map<std::string, double> val) {
		m_averages = val;
	}

	BasicModelView() {

	}

	void Insert() 
	{
		DatabaseSession session;

		CassStatement* statement = cass_statement_new(
			"insert into prophet.basicmodelview (model_id, averages)"
			" values (?,?)", 2);

		cass_statement_bind_uuid(statement, 0, m_modelId);

		CassCollection* map = cass_collection_new(CassCollectionType::CASS_COLLECTION_TYPE_MAP, 
			m_averages.size());

		for (auto pair : m_averages) {
			cass_collection_append_string(map, pair.first.c_str());
			cass_collection_append_double(map, pair.second);
		}

		cass_statement_bind_collection(statement, 1, map);
		cass_collection_free(map);

		CassFuture* future = cass_session_execute(session.Session(), statement);
		cass_future_wait(future);
		cass_statement_free(statement);

		CassError rc = cass_future_error_code(future);
		if (rc != CASS_OK) {
			
			std::string err = std::string("Failed to execute query: ")
				+ CassandraUtils::GetCassandraError(future);
			
			cass_future_free(future);

			throw err;
		}
		cass_future_free(future);
	}
	
	void Update() {
		DatabaseSession session;

		CassStatement* statement = cass_statement_new(
			"update prophet.basicmodelview set averages = ? where model_id = ?", 2);

		cass_statement_bind_uuid(statement, 1, m_modelId);
		CassCollection* map = cass_collection_new(CassCollectionType::CASS_COLLECTION_TYPE_MAP,
			m_averages.size());

		for (auto pair : m_averages) {
			cass_collection_append_string(map, pair.first.c_str());
			cass_collection_append_double(map, pair.second);
		}

		cass_statement_bind_collection(statement, 0, map);
		cass_collection_free(map);

		CassFuture* future = cass_session_execute(session.Session(), statement);
		cass_future_wait(future);
		cass_statement_free(statement);

		CassError rc = cass_future_error_code(future);
		if (rc != CASS_OK) {

			std::string err = std::string("Failed to execute query: ")
				+ CassandraUtils::GetCassandraError(future);

			cass_future_free(future);

			throw err;
		}
		cass_future_free(future);
	
	}

	static BasicModelView* Get(Guid id) {
		DatabaseSession session;

		CassStatement* statement = cass_statement_new(
			"select model_id from prophet.basicmodelview " 
			"where model_id = ? limit 1", 1);

		cass_statement_bind_uuid(statement, 0, id);

		CassFuture* future = cass_session_execute(session.Session(), statement);
		cass_future_wait(future);

		CassError rc = cass_future_error_code(future);
		BasicModelView* modelview = nullptr;

		if (rc != CASS_OK) {
			std::string err = std::string("Failed to execute query")
				+ CassandraUtils::GetCassandraError(future);

			cass_future_free(future);

			throw err;
		}
		else
		{
			const CassResult* result = cass_future_get_result(future);
			CassIterator* iterator = cass_iterator_from_result(result);

			if (cass_iterator_next(iterator)) {
				const CassRow* row = cass_iterator_get_row(iterator);

				modelview = new BasicModelView();

				cass_value_get_uuid(cass_row_get_column(row, 0), &modelview->m_modelId);
			}

			cass_result_free(result);
			cass_iterator_free(iterator);
		}

		cass_future_free(future);
		cass_statement_free(statement);
		return modelview;

	}

};
#endif