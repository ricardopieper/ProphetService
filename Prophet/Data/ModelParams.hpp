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
	Mtx* m_mtx;
	std::vector<int> m_dimensions;
	int m_state;
	std::string m_varname;

public:

	Guid Id() { return m_id; }
	Guid ModelId() { return m_modelId; }

	int Rows() { return m_dimensions[0]; }
	int Cols() { return m_dimensions[1]; }

	Mtx* GetMtx() {
		return m_mtx;
	}

	std::vector<int> Dimension() { return m_dimensions; }
	std::string VarName() { return m_varname; }


	void SetVarName(std::string name) {
		m_varname = name;
	}

	void SetDimensions(std::vector<int> dimensions) {
		m_dimensions = dimensions;
	}

	ModelParams() {
	}




	static void Save(Model& model, std::string name, Mtx& data) {

		std::cout << "saving " << name << std::endl << data << std::endl;

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
			auto err = std::string("Failed to execute query: ")
				+ CassandraUtils::GetCassandraError(future);
			std::cout << "err " << err << std::endl;
			cass_future_free(future);
			throw err;
		}
		else {
			std::cout << "success " << rc << std::endl;
		}
		cass_future_free(future);

	}

	static void DeleteAll(Model& model) {


		DatabaseSession session;

		CassStatement* statement = cass_statement_new(
			"delete from prophet.modelparams where model_id = ?", 1);


		cass_statement_bind_uuid(statement, 0, model.ModelId());

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

	static void DeleteAll(Model& model, Guid) {


		DatabaseSession session;

		CassStatement* statement = cass_statement_new(
			"delete from prophet.modelparams where model_id = ? and modelparams_id = ?", 1);

		//delete from prophet.modelparams where model_id = 5546d453-8947-4e09-9a1a-ec2f558d6e33 and varname = 'theta3';
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
		//std::vector<ModelParams*>* params = new std::vector<ModelParams*>;
		std::vector<ModelParams*> params;
		if (rc != CASS_OK) {
			std::cout << "Cassandra error! " << rc << std::endl;

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
				std::cout << "loaded from cassandra:" << size << std::endl;
				std::cout << "first: " << (buffer[0]) << std::endl;
				CassIterator* inputvalues_iterator =
					cass_iterator_from_collection(cass_row_get_column(row, 4));

				while (cass_iterator_next(inputvalues_iterator)) {

					int value = 0;
					cass_value_get_int32(cass_iterator_get_value(inputvalues_iterator), &value);
					p->m_dimensions.push_back(value);
				}

				const double* d = reinterpret_cast<const double*>(buffer);

				std::cout << "first d: " << (d[0]) << std::endl;

				double* mtxdata = const_cast<double*>(d);

				std::cout << "first mtx: " << (mtxdata[0]) << std::endl;

				double* newbuf = new double[size / 8];

				std::memcpy(newbuf, mtxdata, size);

				p->m_mtx = new Mtx(FSView(p->Rows(), p->Cols(), newbuf, p->Rows()));

				std::cout << *(p->m_mtx) << std::endl;

				cass_iterator_free(inputvalues_iterator);
				params.push_back(p);
				//params->push_back(p);
			}

			cass_result_free(result);
			cass_iterator_free(iterator);
		}

		cass_future_free(future);
		cass_statement_free(statement);

		std::cout << "Finished loading params." << std::endl;

		return params;

	}

	static std::map<std::string, ModelParams*> LoadParameters(Guid modelId)
	{

		std::cout << "Loading params..." << std::endl;

		std::vector<ModelParams*> params = Load(modelId);

		std::cout << "params loaded " << std::endl;

		std::cout << "num theta:" << params.size() << std::endl;

		std::map<std::string, ModelParams*> mapParams;

		std::cout << "instance of map created" << std::endl;


		for (ModelParams* parm : params) {

			std::cout << (parm) << std::endl;
			std::cout << "Dereferencing last address" << std::endl;

			ModelParams p = *parm;
			std::cout << "Dereferenced!" << std::endl;

			std::cout << p.VarName() << std::endl;

			std::cout << "Rows: " << p.Rows() << std::endl;
			std::cout << "Cols: " << p.Cols() << std::endl;

			mapParams.emplace(p.VarName(), parm);

		}

		return mapParams;

	}

	~ModelParams() {
	}

};
#endif
