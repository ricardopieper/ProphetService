#pragma once
#ifndef _UPLOADCHUNKS_HPP_
#define _UPLOADCHUNKS_HPP_
#include <vector>
#include <cassandra.h>
#include "DatabaseSession.hpp"
#include "Upload.hpp"
#include <iostream>
#include <string>
#include <sstream>

class UploadChunks {


public:

	static std::string LoadFile(Upload& upload)
	{

		std::stringstream ss;

		DatabaseSession session;

		try {

			CassError rc = CASS_OK;

			//std::string str = ("SELECT upload_id, model_id, date, processed, result {includeFile} FROM prophet.models {onlyPending}");
			std::string str = "SELECT chunk from prophet.uploadchunks where model_id = ? and upload_id = ?";

			CassStatement* statement = cass_statement_new(str.c_str(), 2);

			cass_statement_bind_uuid(statement, 0, upload.ModelId());
			cass_statement_bind_uuid(statement, 1, upload.UploadId());


			CassFuture * future = cass_session_execute(session.Session(), statement);
			cass_future_wait(future);

			rc = cass_future_error_code(future);

			if (rc != CASS_OK) {

				auto err = std::string("Failed to execute query: ")
					+ CassandraUtils::GetCassandraError(future);

				cass_future_free(future);
				cass_statement_free(statement);
				throw err;
			}
			else {
				const CassResult* result = cass_future_get_result(future);
				CassIterator* iterator = cass_iterator_from_result(result);

				while (cass_iterator_next(iterator)) {

					const CassRow* row = cass_iterator_get_row(iterator);

					std::string buffer;

					CassString::Load(buffer, cass_row_get_column(row, 0));

					ss << buffer;

				}

				cass_result_free(result);
				cass_iterator_free(iterator);

			}

			cass_future_free(future);
			cass_statement_free(statement);

		}
		catch (...) {
			std::exception_ptr e = std::current_exception();
			try {
				if (e) {
					std::rethrow_exception(e);
				}
			}
			catch (const std::exception& ex) {
				std::cout << "Caught exception \"" << ex.what() << "\"\n";
			}
		}
		return ss.str();

	}


	static void ClearDatabase(Upload& upload) {
		DatabaseSession session;

		std::string str = "delete from uploadchunks where model_id = ? and upload_id = ?";
		CassStatement* statement = cass_statement_new(str.c_str(), 2);

		cass_statement_bind_uuid(statement, 0, upload.ModelId());
		cass_statement_bind_uuid(statement, 1, upload.UploadId());

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


};
#endif