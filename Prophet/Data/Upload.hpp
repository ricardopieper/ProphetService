#pragma once
#ifndef _UPLOAD_HPP_
#define _UPLOAD_HPP_
#include <vector>
#include <cassandra.h>
#include "DatabaseSession.hpp"
#include <iostream>
#include <string>

class Upload {
private:
	Guid m_uploadId;
	Guid m_modelId;
	cass_int64_t m_date;
	cass_bool_t m_processed;
	std::string m_result;
	std::string m_file;


	static std::vector<Upload*> loadAllUploads(bool onlyPending = true,
		bool includeFile = false,
		int limit = -1) {

		std::vector<Upload*> uploads;

		DatabaseSession session;

		try {

			CassError rc = CASS_OK;

			//std::string str = ("SELECT upload_id, model_id, date, processed, result {includeFile} FROM prophet.models {onlyPending}");
			std::string str = "SELECT upload_id, model_id, date, processed, result";

			if (includeFile) {
				str += ", file";
			}

			str += " from prophet.uploads";

			if (onlyPending) {
				str += " where processed = false";
			}

			if (limit != -1) {
				str += " limit " + std::to_string(limit);
			}
			if (onlyPending) {
				str += " ALLOW FILTERING";
			}

			CassStatement* statement = cass_statement_new(str.c_str(), 0);
			CassFuture * future = cass_session_execute(session.Session(), statement);
			cass_future_wait(future);

			rc = cass_future_error_code(future);

			if (rc != CASS_OK) {
				throw std::string("Failed to execute query: ")
					+ CassandraUtils::GetCassandraError(future);
			}
			else {
				const CassResult* result = cass_future_get_result(future);
				CassIterator* iterator = cass_iterator_from_result(result);

				while (cass_iterator_next(iterator)) {

					const CassRow* row = cass_iterator_get_row(iterator);

					Upload* upload = new Upload;

					cass_value_get_uuid(cass_row_get_column(row, 0), &upload->m_uploadId);

					cass_value_get_uuid(cass_row_get_column(row, 1), &upload->m_modelId);

					cass_value_get_int64(cass_row_get_column(row, 2), &upload->m_date);

					cass_value_get_bool(cass_row_get_column(row, 3), &upload->m_processed);

					CassString::Load(upload->m_result, cass_row_get_column(row, 4));

					if (includeFile) {
						CassString::Load(upload->m_file, cass_row_get_column(row, 5));
					}

					uploads.push_back(upload);

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
		return uploads;

	}


public:

	Upload() {
	}

	Guid UploadId() {
		return m_uploadId;
	}

	Guid ModelId() {
		return m_modelId;
	}

	cass_int64_t Date() {
		return m_date;
	}

	bool Processed() {
		return m_processed == cass_bool_t::cass_true;
	}

	std::string Result() {
		return m_result;
	}
	std::string File() {
		return m_file;
	}

	const Upload* SetResult(std::string result) {
		m_result = result;
		return this;
	}
	const Upload* SetProcessed(bool processed) {
		m_processed = processed ? cass_bool_t::cass_true : cass_bool_t::cass_false;
		return this;
	}



	static std::vector<Upload*> LoadAllPending(bool includeFile = false) {
		return loadAllUploads(true, includeFile);
	}

	static Upload* GetPending() {

		auto res = loadAllUploads(true, true, 1);

		if (res.size() == 0) {
			return nullptr;
		}
		else return res[0];
	}

	void Save() {

		DatabaseSession session;

		CassStatement* statement = cass_statement_new("update prophet.uploads set processed = ?, result = ? where model_id = ? and upload_id = ?", 4);

		cass_statement_bind_bool(statement, 0, m_processed);
		cass_statement_bind_string(statement, 1, m_result.c_str());
		cass_statement_bind_uuid(statement, 2, m_modelId);
		cass_statement_bind_uuid(statement, 3, m_uploadId);

		CassFuture* future = cass_session_execute(session.Session(), statement);
		cass_future_wait(future);

		CassError rc = cass_future_error_code(future);
	
		cass_future_free(future);
		cass_statement_free(statement);

		if (rc != CASS_OK) {
			throw std::string("Failed to execute query: ")
				+ CassandraUtils::GetCassandraError(future);
		}
	}


};
#endif