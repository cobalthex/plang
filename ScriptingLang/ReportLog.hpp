#pragma once

#include "Array.hpp"
#include "Location.hpp"

namespace Plang
{
	enum class ReportLevel
	{
		Info,
		Message,
		Warning,
		Error,

		Count
	};

	enum class ReportSeverity
	{
		Informational,
		Deprecated,
		Trivial,
		Moderate,
		Severe,
	};

	struct ReportType
	{
		ReportLevel level;
		ReportSeverity severity;
		std::string description; //string that describes how to print a report
								 //arguments: $0 = number of arguments, $1-... are individual arguments
	};

	using ReportTypeId = size_t;

	struct Report
	{
		ReportTypeId type;

		Location location;
		size_t filePos; //file name stored in `location`

		::Array<std::string> arguments; //possibly just inline to two/three args
	};

	class ReportLog
	{
	public:
		static ReportTypeId RegisterType(const ReportType& Type)
		{
            auto id = nextTypeId++;

			types[id] = Type;
			return id;
		}

		static inline const ReportType& GetType(ReportTypeId Id) { return types[Id]; }

		static void AddReport(const Report& Report)
		{
			decltype(auto) type = GetType(Report.type);
			auto& reps = reports[type.level];
			reportTimeline.push_back({ type.level, reps.size() });
			reps.push_back(Report);
		}

		//todo: iterators

	protected:
		static size_t nextTypeId;

		static std::map<ReportTypeId, ReportType> types;

		static std::map<ReportLevel, std::vector<Report>> reports;

		struct ReportIndex
		{
			ReportLevel level;
			size_t index;
		};

		static std::deque<ReportIndex> reportTimeline;
	};
}