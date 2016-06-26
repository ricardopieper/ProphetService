#pragma once
#pragma once
#ifndef _MODELPREDICTIONS_HPP_
#define _MODELPREDICTIONS_HPP_
#include <vector>
#include <map>
#include <cassandra.h>
#include <iostream>
#include <string>
#include "DatabaseSession.hpp"
#include "Model.hpp"
#include "../ML/FlensDefs.hpp"

class ModelPrediction {
private:
	Guid m_id;
	Guid m_modelId;
	std::map<std::string, double> m_inputvalues;
	std::string m_inputvar;
	double m_fromValue;
	double m_toValue;
	int m_amountPredictions;
public:

	Guid Id() { return m_id; }
	Guid ModelId() { return m_modelId; }
	std::map<std::string, double> InputValues() { return m_inputvalues; }
	std::string InputVar() { return m_inputvar; }
	double FromValue() { return m_fromValue; }
	double ToValue() { return m_toValue; }
	int AmountOfPredictions() { return m_amountPredictions; }

	static ModelPrediction* GetPending() {

		DatabaseSession dbSession;

		CassStatement* statement = cass_statement_new(
			"select prediction_id, model_id, inputvalues, inputvar, fromValue, "
			"toValue, amountPredictions from prophet.modelpredictions "
			"where state = 0 limit 1 ALLOW FILTERING;", 0);

		CassFuture* future = cass_session_execute(dbSession.Session(), statement);
		cass_future_wait(future);

		CassError rc = cass_future_error_code(future);
		ModelPrediction* pendingPrediction = nullptr;

		if (rc != CASS_OK) {
			throw std::string("Failed to execute query")
				+ CassandraUtils::GetCassandraError(future);
		}
		else
		{
			const CassResult* result = cass_future_get_result(future);
			CassIterator* iterator = cass_iterator_from_result(result);

			if (cass_iterator_next(iterator)) {
				const CassRow* row = cass_iterator_get_row(iterator);

				pendingPrediction = new ModelPrediction();

				cass_value_get_uuid(cass_row_get_column(row, 0), &pendingPrediction->m_id);
				cass_value_get_uuid(cass_row_get_column(row, 1), &pendingPrediction->m_modelId);

				CassIterator* inputvalues_iterator =
					cass_iterator_from_map(cass_row_get_column(row, 2));

				while (cass_iterator_next(inputvalues_iterator)) {
					std::string val;
					CassString::Load(val, cass_iterator_get_map_key(inputvalues_iterator));

					double value = 0;
					cass_value_get_double(cass_iterator_get_map_value(inputvalues_iterator), &value);

					pendingPrediction->m_inputvalues.emplace(val, value);
				}

				CassString::Load(pendingPrediction->m_inputvar, cass_row_get_column(row, 3));

				cass_value_get_double(cass_row_get_column(row, 4), &pendingPrediction->m_fromValue);
				cass_value_get_double(cass_row_get_column(row, 5), &pendingPrediction->m_toValue);
				cass_value_get_int32(cass_row_get_column(row, 6), &pendingPrediction->m_amountPredictions);

			}

			cass_result_free(result);
			cass_iterator_free(iterator);
		}

		cass_future_free(future);
		cass_statement_free(statement);
		return pendingPrediction;
	}
	
	void SaveResult(std::string result) {

		DatabaseSession session;

		CassStatement* statement = cass_statement_new(
			"update prophet.modelpredictions set state = 1, result = ? where model_id = ? and prediction_id = ?", 3);

		cass_statement_bind_string(statement, 0, result.c_str());

		cass_statement_bind_uuid(statement, 1, m_modelId);

		cass_statement_bind_uuid(statement, 2, m_id);


		CassFuture* future = cass_session_execute(session.Session(), statement);
		cass_future_wait(future);

		CassError rc = cass_future_error_code(future);
		if (rc != CASS_OK) {
			cass_statement_free(statement);
			std::string err = std::string("Failed to execute query: ")
								+ CassandraUtils::GetCassandraError(future);
			cass_future_free(future);
			throw err;

		}
		cass_statement_free(statement);
		cass_future_free(future);
	}

};
#endif


