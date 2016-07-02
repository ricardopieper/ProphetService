#pragma once
#ifndef _MODELDATASETS_HPP_
#define _MODELDATASETS_HPP_
#include <vector>
#include <cassandra.h>
#include "DatabaseSession.hpp"
#include "Model.hpp"
#include <iostream>
#include <string>
#include "../ML/FlensDefs.hpp"

class ModelDatasets {

public:

	ModelDatasets() {
	}


	static void Save(Model* model, std::vector<std::string> headers, Mtx* mtx) {

		DatabaseSession session;

	
		int batchSize = 200;
		int remainingRows = mtx->numRows();


		while (remainingRows > 0){

			std::string insert = "insert into prophet.modeldatasets (model_id, row_id, ";
			std::string fields = "";
			std::string params = "?, uuid(), ";
			{
				int i = 0;
				int max = headers.size();
				for (auto str : headers) {
					fields += str;
					params += "?";
					if (i < max - 1) {
						fields += ", ";
						params += ", ";
					}
					i++;
				}
			}
			insert += fields + ") values (" + params + ")";
	
			CassFuture* future = cass_session_prepare(session.Session(), insert.c_str());
	
			cass_future_wait(future);

			CassError rc = cass_future_error_code(future);	
			if (rc != CASS_OK) {
				throw std::string("Failed to execute query: ")
				+ CassandraUtils::GetCassandraError(future);
			}
			else {

				const CassPrepared* prepared = cass_future_get_prepared(future);

				CassBatch* batch = cass_batch_new(CASS_BATCH_TYPE_LOGGED);


				int cols = mtx->numCols();

				int rowsToInsert = std::min(batchSize, remainingRows);
				std::cout<<"Inserting "<<rowsToInsert<<" rows"<<std::endl;
				for (int row = 0; row < rowsToInsert; row++) {
					CassStatement* statement = cass_prepared_bind(prepared);

					cass_statement_bind_uuid(statement, 0, model->ModelId());

					for (int j = 0; j < cols; j++) {

						cass_double_t d = (*mtx)(row + 1, j + 1);

						cass_statement_bind_double(statement, j + 1, d);

					}
					cass_batch_add_statement(batch, statement);
					cass_statement_free(statement);


				}

				future = cass_session_execute_batch(session.Session(), batch);
				cass_future_wait(future);

				rc = cass_future_error_code(future);
				if (rc != CASS_OK) {

					cass_batch_free(batch);
					std::string err = std::string("Failed to execute query: ")
						+ CassandraUtils::GetCassandraError(future);

					cass_future_free(future);
					throw err;
				}
				cass_future_free(future);
				cass_batch_free(batch);
				remainingRows -= rowsToInsert;
				std::cout<<"Inserted "<<rowsToInsert<<"/"<<(mtx->numRows())<< " ("<<remainingRows << " remaining)"<<std::endl;
			}
		}

	}


};
#endif
