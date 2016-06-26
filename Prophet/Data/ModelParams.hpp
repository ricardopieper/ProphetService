#pragma once
#ifndef _MODELPARAMS_HPP_
#define _MODELPARAMS_HPP_
#include <vector>
#include <map>
#include <cassandra.h>
#include <iostream>
#include <string>

class ModelParams {
private:
	Guid m_id;
	Guid m_modelId;
	double* m_data;
	std::vector<int> m_dimensions;
	int m_state;
	std::string m_varname;

public:

	Guid Id() { return m_id; }
	Guid ModelId() { return m_modelId; }
	double* Data() { return m_data; }

	int Rows() { return m_dimensions[0]; }
	int Cols() { return m_dimensions[1]; }

	std::vector<int> Dimension() { return m_dimensions; }
	std::string VarName() { return m_varname; }


	void SetData(double* data) {
		m_data = data;
	}
	void SetVarName(std::string name) {
		m_varname = name;
	}

	void SetDimensions(std::vector<int> dimensions) {
		m_dimensions = dimensions;
	}

	ModelParams() {
	}

	static void Save(Model& model, std::string name, Mtx& data) {


		DatabaseSession session;

		CassStatement* statement = cass_statement_new(
			"insert into prophet.modelparams (modelparams_id, model_id, varname, data, dimensions)"
			" values (uuid(), ?,?,?,?)", 4);

		const cass_byte_t* pData = reinterpret_cast<cass_byte_t*>(data.data());

		cass_statement_bind_uuid(statement, 0, model.ModelId());
		cass_statement_bind_string(statement, 1, name.c_str());
		cass_statement_bind_bytes(statement, 2,
			pData,
			sizeof(double)*data.numCols() * data.numRows());


		CassCollection* dimensions = cass_collection_new(
			CassCollectionType::CASS_COLLECTION_TYPE_LIST, 2);

		cass_collection_append_int32(dimensions, data.numRows());
		cass_collection_append_int32(dimensions, data.numCols());

		cass_statement_bind_collection(statement, 3, dimensions);

		CassFuture* future = cass_session_execute(session.Session(), statement);
		cass_future_wait(future);
		cass_statement_free(statement);

		CassError rc = cass_future_error_code(future);

		cass_collection_free(dimensions);

		if (rc != CASS_OK) {
			cass_future_free(future);

			throw std::string("Failed to execute query: ")
				+ CassandraUtils::GetCassandraError(future);
		}
		cass_future_free(future);

	}

	static void DeleteAll(Model& model) {


		DatabaseSession session;

		CassStatement* statement = cass_statement_new(
			"delete from prophet.modelparams where model_id = ?",1);


		cass_statement_bind_uuid(statement, 0, model.ModelId());

		CassFuture* future = cass_session_execute(session.Session(), statement);
		cass_future_wait(future);
		cass_statement_free(statement);

		CassError rc = cass_future_error_code(future);

		if (rc != CASS_OK) {
			cass_future_free(future);

			throw std::string("Failed to execute query: ")
				+ CassandraUtils::GetCassandraError(future);
		}
		cass_future_free(future);

	}


	static std::vector<ModelParams*> Load(Guid modelId) {

		DatabaseSession dbSession;

		CassStatement* statement = cass_statement_new(
			"select modelparams_id, model_id, varname, data, dimensions "
			"from prophet.modelparams where model_id = ?", 1);

		cass_statement_bind_uuid(statement, 0, modelId);

		CassFuture* future = cass_session_execute(dbSession.Session(), statement);
		cass_future_wait(future);

		CassError rc = cass_future_error_code(future);
		std::vector<ModelParams*> params;

		if (rc != CASS_OK) {
			throw std::string("Failed to execute query")
				+ CassandraUtils::GetCassandraError(future);
		}
		else
		{
			const CassResult* result = cass_future_get_result(future);
			CassIterator* iterator = cass_iterator_from_result(result);

			while (cass_iterator_next(iterator)) {
				const CassRow* row = cass_iterator_get_row(iterator);

				ModelParams* p = new ModelParams();

				cass_value_get_uuid(cass_row_get_column(row, 0), &p->m_id);
				cass_value_get_uuid(cass_row_get_column(row, 1), &p->m_modelId);

				CassString::Load(p->m_varname, cass_row_get_column(row, 2));


				size_t size = 0;
				const cass_byte_t* buffer;
				cass_value_get_bytes(cass_row_get_column(row, 3), &buffer, &size);


				CassIterator* inputvalues_iterator =
					cass_iterator_from_collection(cass_row_get_column(row, 4));

				while (cass_iterator_next(inputvalues_iterator)) {

					int value = 0;
					cass_value_get_int32(cass_iterator_get_value(inputvalues_iterator), &value);
					p->m_dimensions.push_back(value);
				}
				const double* d = reinterpret_cast<const double*>(buffer);
				p->m_data = const_cast<double*>(d);

				cass_iterator_free(inputvalues_iterator);

				params.push_back(p);
			}

			cass_result_free(result);
			cass_iterator_free(iterator);
		}

		cass_future_free(future);
		cass_statement_free(statement);
		return params;

	}

	static std::map<std::string, Mtx*> LoadParameters(Guid modelId)
	{

		auto params = Load(modelId);

		std::map<std::string, Mtx*> mapParams;

		for (auto p : params) {

			Mtx* mtx = new Mtx(FSView(p->Rows(), p->Cols(), p->Data(), p->Rows()));
			mapParams.emplace(p->VarName(), mtx);
			
		}

		return mapParams;

	}

	~ModelParams() {
		delete[] m_data;
	}

};
#endif