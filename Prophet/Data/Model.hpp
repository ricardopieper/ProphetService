#pragma once
#ifndef _MODEL_HPP_
#define _MODEL_HPP_

#include <vector>
#include <cassandra.h>
#include "DatabaseSession.hpp"
#include "../ML/FlensDefs.hpp"

enum ModelState {
	Pending = 0,
	Training = 1,
	Trained = 2,
	TrainingFailed = 3
};


class Model {
private:
	Guid m_modelId,
		m_digesterId,
		m_engineId;

	std::vector<std::string> m_inputVars;

	std::string m_outputVar, m_name;

	ModelState m_state;

	int m_millisecondsTraining;
public:

	Model(const CassRow* row) {

		cass_value_get_uuid(cass_row_get_column(row, 0), &m_modelId);
		cass_value_get_uuid(cass_row_get_column(row, 1), &m_digesterId);
		cass_value_get_uuid(cass_row_get_column(row, 2), &m_engineId);

		CassString::Load(m_name, cass_row_get_column(row, 4));
		CassString::Load(m_outputVar, cass_row_get_column(row, 5));

		cass_int32_t state = 0;
		cass_value_get_int32(cass_row_get_column(row, 6), &state);

		m_state = (ModelState)state;

		CassIterator* items_iterator = nullptr;

		const CassValue* value = cass_row_get_column(row, 3);
		items_iterator = cass_iterator_from_collection(value);

		while (cass_iterator_next(items_iterator)) {
			std::string val;
			CassString::Load(val, cass_iterator_get_value(items_iterator));
			m_inputVars.push_back(val);
		}
		cass_iterator_free(items_iterator);
	}



	Mtx* GetDataset() {
		DatabaseSession session;


		std::string insert = "select ";
		std::string fields = "";

		for (auto str : m_inputVars) {
			fields += str + ", ";
		}
		insert += fields;

		insert += m_outputVar;


		insert += " from prophet.modeldatasets where model_id = ?";

		CassStatement* statement = cass_statement_new(insert.c_str(), 1);
		cass_statement_bind_uuid(statement, 0, m_modelId);

		cass_statement_set_paging_size(statement, 100000);

		std::vector<std::vector<double>> mtxBuf;
		size_t columns = m_inputVars.size() + 1;

		cass_bool_t hasMorePages = cass_true;
		while (hasMorePages)
		{
			std::cout << "fetching dataset..." << std::endl;
			CassFuture * future = cass_session_execute(session.Session(), statement);
			cass_future_wait(future);

			CassError rc = cass_future_error_code(future);

			if (rc != CASS_OK) {
				std::string str = std::string("Failed to execute query: ")
					+ CassandraUtils::GetCassandraError(future);

				cass_future_free(future);
				throw str;
			}
			else
			{
				const CassResult* result = cass_future_get_result(future);
				CassIterator* iterator = cass_iterator_from_result(result);

				while (cass_iterator_next(iterator)) {

					const CassRow* row = cass_iterator_get_row(iterator);

					std::vector<double> line;

					for (size_t i = 0; i < columns; i++) {

						double* d = new double;

						cass_value_get_double(cass_row_get_column(row, i), d);

						line.push_back(*d);

					}
					mtxBuf.push_back(line);
				}

				hasMorePages = cass_result_has_more_pages(result);
				if (hasMorePages) {
					cass_statement_set_paging_state(statement, result);
				}
				cass_result_free(result);
				cass_iterator_free(iterator);

			}
			cass_future_free(future);
		}

		if (mtxBuf.size()) {

			Mtx* m = new Mtx(mtxBuf.size(), columns);

			for (size_t i = 1; i <= mtxBuf.size(); i++) {

				auto line = mtxBuf[i - 1];

				for (size_t j = 1; j <= columns; j++) {

					double v = line[j - 1];

					(*m)(i, j) = v;
				}
			}

			cass_statement_free(statement);
			return m;
		}
		else return nullptr;
	}


	Guid ModelId() {
		return m_modelId;
	}
	Guid DigesterId() {
		return m_digesterId;
	}
	Guid EngineId() {
		return m_engineId;
	}
	std::vector<std::string> InputVariables() {
		return m_inputVars;
	}
	std::string OutputVariable() {
		return m_outputVar;
	}
	std::string Name() {
		return m_name;
	}
	ModelState State() {
		return m_state;
	}

	void SetIntColumn(std::string col, int value) {
		DatabaseSession session;

		CassStatement* statement = cass_statement_new(
			("update prophet.models set state = 2, " + col + " = ? where model_id = ?").c_str(), 2);

		cass_statement_bind_int32(statement, 0, value);

		cass_statement_bind_uuid(statement, 1, m_modelId);


		CassFuture* future = cass_session_execute(session.Session(), statement);
		cass_future_wait(future);

		CassError rc = cass_future_error_code(future);
		if (rc != CASS_OK) {

			std::string err = std::string("Failed to execute query: ")
				+ CassandraUtils::GetCassandraError(future);
			cass_statement_free(statement);
			cass_future_free(future);
			throw err;
		}
		cass_statement_free(statement);
		cass_future_free(future);
	}


	void SetProcessed(int millisecondsTraining) {
		SetIntColumn("millisecondstraining", millisecondsTraining);
	}

	static std::vector<Model*> LoadAll() {

		DatabaseSession dbSession;

		CassError rc = CASS_OK;

		CassStatement* statement = cass_statement_new(
			"SELECT model_id, digester_id, engine_id, inputvars, "
			"name, outputvar, state FROM prophet.models", 0);

		CassFuture* future = cass_session_execute(dbSession.Session(), statement);
		cass_future_wait(future);

		rc = cass_future_error_code(future);

		std::vector<Model*> models;

		if (rc != CASS_OK) {
			throw std::string("Failed to execute query: ")
				+ CassandraUtils::GetCassandraError(future);
		}
		else {
			const CassResult* result = cass_future_get_result(future);
			CassIterator* iterator = cass_iterator_from_result(result);

			while (cass_iterator_next(iterator)) {

				const CassRow* row = cass_iterator_get_row(iterator);

				models.push_back(new Model(row));

			}

			cass_result_free(result);
			cass_iterator_free(iterator);

		}

		cass_future_free(future);
		cass_statement_free(statement);

		return models;

	}

	static Model* Load(Guid modelId) {

		DatabaseSession dbSession;

		CassError rc = CASS_OK;

		CassStatement* statement = cass_statement_new(
			"SELECT model_id, digester_id, engine_id, inputvars, "
			"name, outputvar, state FROM prophet.models where model_id = ?", 1);

		cass_statement_bind_uuid(statement, 0, modelId);

		CassFuture* future = cass_session_execute(dbSession.Session(), statement);
		cass_future_wait(future);

		rc = cass_future_error_code(future);
		Model* modelResult = nullptr;

		if (rc != CASS_OK) {
			throw std::string("Failed to execute query")
				+ CassandraUtils::GetCassandraError(future);
		}
		else
		{
			const CassResult* result = cass_future_get_result(future);
			CassIterator* iterator = cass_iterator_from_result(result);

			if (cass_iterator_next(iterator)) {

				modelResult = new Model(cass_iterator_get_row(iterator));

			}

			cass_result_free(result);
			cass_iterator_free(iterator);
		}

		cass_future_free(future);
		cass_statement_free(statement);

		return modelResult;

	}

	static Model* GetPending() {

		DatabaseSession dbSession;

		CassError rc = CASS_OK;

		CassStatement* statement = cass_statement_new(
			"SELECT model_id, digester_id, engine_id, inputvars, "
			"name, outputvar, state FROM prophet.models where state = 1 limit 1 ALLOW FILTERING", 0);

		CassFuture* future = cass_session_execute(dbSession.Session(), statement);
		cass_future_wait(future);

		rc = cass_future_error_code(future);
		Model* modelResult = nullptr;

		if (rc != CASS_OK) {
			throw std::string("Failed to execute query")
				+ CassandraUtils::GetCassandraError(future);
		}
		else
		{
			const CassResult* result = cass_future_get_result(future);
			CassIterator* iterator = cass_iterator_from_result(result);

			if (cass_iterator_next(iterator)) {
				modelResult = new Model(cass_iterator_get_row(iterator));
			}

			cass_result_free(result);
			cass_iterator_free(iterator);
		}

		cass_future_free(future);
		cass_statement_free(statement);

		return modelResult;

	}

};

#endif