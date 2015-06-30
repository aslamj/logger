#pragma once

#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <vector>

// Logger namespace
namespace Logger
{
#define DEC_VALUE(x) Logger::CUtils::GetUnicodeString(Logger::CUtils::GetDecimalString(x).c_str())
#define HEX_VALUE(x) Logger::CUtils::GetUnicodeString(Logger::CUtils::GetHexadecimalString(x).c_str())
#define WIN32_ERROR(x) Logger::CUtils::GetUnicodeString(Logger::CUtils::GetWin32ErrorString(x).c_str())

	/*
	 Utils class 
	*/
	class CUtils
	{
		friend class CLog;

	public:
		// returns string value of passed in integer
		static std::string GetDecimalString(int n, unsigned int filln = 0)
		{
			std::stringstream str;
			str << std::setfill('0') << std::setw(filln) << n;
			return std::string(str.str());
		}

		// returns hex string value of passed in integer
		static std::string GetHexadecimalString(int n, unsigned int filln = 8)
		{
			std::stringstream str;
			str << "0x" << std::setfill('0') << std::setw(filln) << std::hex << n;
			return std::string(str.str());
		}

		// returns ascii string of passed unicode string
		static std::string GetAsciiString(const wchar_t* const message)
		{
			char* pszBuf = NULL;

			int n = static_cast<int>(wcslen(message) + 1);
			pszBuf = new char[n];

			WideCharToMultiByte(
				CP_ACP,
				0,
				message,
				n,
				pszBuf,
				n,
				NULL,
				NULL);

			std::string ret(pszBuf);

			if (pszBuf)
				delete[] pszBuf;

			return ret;
		}

		// returns unicode string of passed ascii string
		static std::wstring GetUnicodeString(const char* const message)
		{
			wchar_t * pwcsBuf = NULL;

			int n = static_cast<int>(strlen(message) + 1);
			pwcsBuf = new wchar_t[n];

			MultiByteToWideChar(
				CP_ACP,
				0,
				message,
				n,
				pwcsBuf,
				n);

			std::wstring str(pwcsBuf);

			if (pwcsBuf)
				delete[] pwcsBuf;

			return str;
		}

		// return an ascii string for Windows error code
		static std::string GetWin32ErrorString(DWORD dwError)
		{
			std::string strError = "";

			LPVOID lpMsgBuf = NULL;
			const DWORD dwRet = FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				dwError,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				reinterpret_cast<TCHAR*>(&lpMsgBuf),
				0,
				NULL);

			strError += GetHexadecimalString(dwError);
			strError += " (";

			int nErrorMessageLen = 0;

			//if (!dwRet || !lpMsgBuf || !(nErrorMessageLen = static_cast<int>(_tcslen (static_cast<TCHAR*>(lpMsgBuf)))))
			if (!dwRet || !lpMsgBuf || !(nErrorMessageLen = static_cast<int>(wcslen(static_cast<wchar_t *>(lpMsgBuf)))))
			{
				strError += "None.)";
				return strError;
			}

			char* pszErrorMessage = new char[nErrorMessageLen + 1];

			//#if defined (_UNICODE) || defined (UNICODE)
			WideCharToMultiByte(
				CP_ACP,
				0,
				static_cast<TCHAR*>(lpMsgBuf),
				static_cast<int>(wcslen(static_cast<TCHAR*>(lpMsgBuf))) + 1,
				pszErrorMessage,
				nErrorMessageLen + 1,
				NULL,
				NULL);

			//#else
			//    strError += static_cast<char*>(lpMsgBuf);
			//#endif

			if (pszErrorMessage[nErrorMessageLen - 2] == '\r')
				pszErrorMessage[nErrorMessageLen - 2] = '\0';
			if (pszErrorMessage[nErrorMessageLen - 1] == '\n')
				pszErrorMessage[nErrorMessageLen - 1] = '\0';

			strError += pszErrorMessage;
			strError += ")";

			if (pszErrorMessage)
				delete[] pszErrorMessage;
			LocalFree(lpMsgBuf);

			return strError;
		}

	private:
		// used internally to get local system time
		static SYSTEMTIME GetLocalSystemTime()
		{
			SYSTEMTIME tTime = { 0 };
			GetLocalTime(&tTime);
			return tTime;
		}

		// used internally to get the local time string
		static std::string GetTimeString(const SYSTEMTIME& tTime)
		{
			const char pszDays[][8] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
			const char pszMonths[][8] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

			std::string str("");

			// Thu Nov 04 2004
			//str += pszDays[tTime.wDayOfWeek];
			//str += " ";
			//str += pszMonths[tTime.wMonth - 1];
			//str += " ";
			//str += GetDecimalString(tTime.wDay, 2);
			//str += " ";
			//str += GetDecimalString(tTime.wYear, 4);
			//str += " ";

			// 11/04/2004 
			//str += pszDays[tTime.wDayOfWeek];
			//str += " ";
			str += GetDecimalString(tTime.wMonth, 2);
			str += "/";
			str += GetDecimalString(tTime.wDay, 2);
			str += "/";
			str += GetDecimalString(tTime.wYear, 4);
			str += " ";

			// 13:29:11.243
			str += GetDecimalString(tTime.wHour, 2);
			str += ":";
			str += GetDecimalString(tTime.wMinute, 2);
			str += ":";
			str += GetDecimalString(tTime.wSecond, 2);
			str += ".";
			str += GetDecimalString(tTime.wMilliseconds, 3);

			return str;
		}

		// used internally by logger to prepare log header
		static std::string GetLogHeaderString()
		{
			// mm/dd/yyyy hh:mm:ss.ms, [sid.pid.tid process_name]
			std::string str("");

			str += GetTimeString(GetLocalSystemTime());
			str += ", ";
			str += "[";
			str += GetSessionIDProcessIDThreadIDString(GetCurrentProcessId(), GetCurrentThreadId());
			str += " ";
			str += GetAsciiString(GetProcessName().c_str());
			str += "]";

			return str;
		}

		// used internally within logging header
		static std::wstring GetProcessName(bool bAbsPath = false)
		{
			wchar_t wcsModulePath[MAX_PATH] = { 0 };
			GetModuleFileName(NULL, wcsModulePath, MAX_PATH);

			std::wstring wstrModulePath = (0 == wcslen(wcsModulePath)) ? L"<None>" : wcsModulePath;

			if (false == bAbsPath)
			{
				size_t idx = wstrModulePath.rfind('\\');
				if (idx > 0)
					wstrModulePath.erase(0, idx + 1);
			}

			size_t idx = wstrModulePath.rfind('.');
			if (idx > 0)
				wstrModulePath.erase(idx, wstrModulePath.size());

			return wstrModulePath;
		}

		// used internally by logger to print bytes (hex representation with its pritable characters)
		static std::string GetBytesToHexString(const BYTE* const pData, const size_t pDataSize, const bool bUseSpaces = false)
		{
			std::string str;
			const BYTE aHex[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

			for (unsigned int i = 0; i < pDataSize; i++)
			{
				const BYTE ch = pData[i];
				// add first nibble
				str += aHex[(ch & 0xF0) >> 4];
				// add second nibble
				str += aHex[ch & 0x0F];
				if (bUseSpaces)
					str += " ";
			}
			return str;
		}

		// used internally for logging
		static std::string GetSessionIDProcessIDThreadIDString(unsigned long nProcessID, unsigned long nThreadID)
		{
			std::string str("");

			DWORD dwSessionID = -1;
			if (TRUE == ProcessIdToSessionId(nProcessID, &dwSessionID))
				str += GetDecimalString(dwSessionID);
			str += ".";
			str += GetDecimalString(nProcessID);
			str += ".";
			str += GetDecimalString(nThreadID);

			return str;
		}
	};

	/*
	 Time Diff util class
	*/
	class CTimeDiff
	{
	private:
		DWORD m_dwStart;
		DWORD m_dwEnd;

	public:
		CTimeDiff() :
			m_dwStart(0),
			m_dwEnd(0)
		{
			start();
		}

		DWORD start()
		{
			m_dwStart = GetTickCount();
			return m_dwStart;
		}

		DWORD end()
		{
			m_dwEnd = GetTickCount();
			return m_dwEnd;
		}

		DWORD diff()
		{
			if (m_dwStart && (m_dwEnd == 0))
				end();
			const DWORD dwDiff = m_dwEnd - m_dwStart;
			m_dwEnd = m_dwStart = 0;
			return dwDiff;
		}
	};

	/*
	 Base class for log context
	*/
	class CLogContext
	{
	public:
		virtual void logMessage(const std::wstring& message) const = 0;
	};

	/*
	 File log context: prints in the specified file used for logging (tail it to see the progress)
	*/
	class CFileLogContext :
		public CLogContext
	{
	public:
		CFileLogContext(const wchar_t * pwcsLogFileName = L"c:\\logging.log") :
			m_logFileName(pwcsLogFileName)
		{
		}

		virtual void logMessage(const std::wstring& message) const
		{
			std::fstream logFile;
			logFile.open(m_logFileName.c_str(), std::ios_base::app);

			logFile << CUtils::GetAsciiString(message.c_str()).c_str();
		}

	private:
		std::wstring m_logFileName;
	};

	/*
	 Console (stdout) log context: prints in the output console
	*/
	class CStdOutLogContext :
		public CLogContext
	{
	public:
		virtual void logMessage(const std::wstring& message) const
		{
			std::cout << CUtils::GetAsciiString(message.c_str());
		}
	};


	/*
	 Debug log context: prints in the debug (uses Window's OutputDebugString())
	*/
	class CDebugLogContext :
		public CLogContext
	{
	public:
		virtual void logMessage(const std::wstring& message) const
		{
			OutputDebugString(message.c_str());
		}
	};

	/*
	 Log class
	*/
	class CLog
	{
	private:
		CLogContext & m_rLogCtx;

		std::wstring m_funcName;
		CTimeDiff m_td;

		std::wstring m_logMessage;
		BYTE * m_pTempData;

	public:
		// ctor()
		CLog(CLogContext & logCtx, std::wstring funcName) :
			m_rLogCtx(logCtx),
			m_funcName(funcName),
			m_logMessage(),
			m_pTempData(NULL)
		{
			*this << std::endl;
			*this << L"Entered " << m_funcName << L"()" << std::endl;
			m_td.start();
		}

		// dtor()
		~CLog()
		{
			*this << L"Exiting from " << m_funcName << L"(), it took " << m_td.diff() << L" milliseconds" << std::endl;
			*this << std::endl;
		}

		// logs string (ascii)
		CLog& operator << (const std::string & message)
		{
			m_logMessage += CUtils::GetUnicodeString(message.c_str());
			return *this;
		}

		// logs wstring (unicode)
		CLog& operator << (const std::wstring & message)
		{
			m_logMessage += message;
			return *this;
		}

		// logs char* 
		CLog& operator << (const char * message)
		{
			m_logMessage += CUtils::GetUnicodeString(message);
			return *this;
		}

		// logs wchar_t*
		CLog& operator << (const wchar_t * message)
		{
			m_logMessage += message;
			return *this;
		}

		// logs the BYTES in passed vector<unsigned char>
		CLog& operator << (const std::vector<BYTE> & blob)
		{
			logByteValues(&blob[0], blob.size());
			return *this;
		}

		// logs the BYTES pointed by the unsigned char* (like pbData)
		CLog& operator << (const BYTE * pData)
		{
			if (NULL == m_pTempData && NULL != pData)
				m_pTempData = const_cast<BYTE*>(pData);

			return *this;
		}

		// logs the size of above pbData
		CLog& operator << (const size_t nDataSize)
		{
			if (NULL != m_pTempData && nDataSize > 0)
			{
				logByteValues(m_pTempData, nDataSize);
				m_pTempData = NULL;
			}
			else
			{
				// disable warning messages 4267
#pragma warning(push)
#pragma warning(disable : 4267)
				// disabling warning C4267 which is happening for 64 bits application flag /Wp64, if specified:
				// warning C4267: 'argument' : conversion from 'size_t' to 'unsigned int', possible loss of data
				m_logMessage += CUtils::GetUnicodeString(CUtils::GetDecimalString(nDataSize).c_str());
#pragma warning(pop)
			}

			return *this;
		}

		// for std::endl, so that the log line can be printed
		CLog& operator << (std::wostream& (__cdecl* func) (std::wostream&))
		{
			m_logMessage += L"\n";

			std::wstring str;
			str += CUtils::GetUnicodeString(CUtils::GetLogHeaderString().c_str());
			str += L" => ";
			str += m_logMessage.c_str();

			m_rLogCtx.logMessage(str);

			m_logMessage.clear();
			return *this;
		}

	private:
		// internal function which is actually doing all the logging
		void logByteValues(const BYTE* pData, const size_t nDataSize)
		{
			if (NULL == pData && 0 <= nDataSize)
				return;

			std::string strMessage;
			strMessage += ",\n";

			const size_t z = CUtils::GetLogHeaderString().length() + strlen(" => "); // grab this for space count
			// add spaces
			for (size_t y = 0; y < z; y++)
				strMessage += " ";

			strMessage += "cbData: ";
			strMessage += CUtils::GetDecimalString(static_cast<unsigned int>(nDataSize));
			strMessage += " bytes, pbData:";
			strMessage += "\n";

			for (size_t i = 0; i < nDataSize; i += 16)
			{
				std::string strTemp;

				size_t n = 16;
				if ((nDataSize - i) < 16)
					n = nDataSize - i;

				// add spaces
				for (unsigned int m = 0; m < z; m++)
					strMessage += " ";

				// add address
				// disable warning messages 4311
#pragma warning(push)
#pragma warning(disable : 4311)
				// disabling warning C4311 which is happening for 64 bits application flag /Wp64, if specified:
				// warning C4311: 'reinterpret_cast' : pointer truncation from 'BYTE *' to 'unsigned int'
				strMessage += CUtils::GetHexadecimalString(reinterpret_cast<unsigned int>(const_cast<BYTE*>(pData)+i));
#pragma warning(pop)
				strMessage += ": ";

				// add hex string
				strTemp = CUtils::GetBytesToHexString(pData + i, n, true);
				strMessage += strTemp;

				// add spaces
				if (n != 16)
				{
					for (unsigned int j = 0; j < 16 - n; j++)
						strMessage += "   ";
				}
				strMessage += "   ";

				// add char string
				for (size_t j = i; j < i + n; j++)
				{
					const BYTE ch = static_cast<BYTE>(pData[j]);

					if (ch == 0)
						strMessage += ".";	// for non-printable chars
					else if (ch >= 1 && ch <= 6)
						strMessage += ch;
					else if (ch >= 7 && ch <= 10)
						strMessage += ".";	// for non-printable chars
					else if (ch >= 11 && ch <= 12)
						strMessage += "."; // wordpad sucks for these two
					else if (ch == 13)
						strMessage += ".";	// for non-printable chars
					else if (ch >= 14 && ch <= 31)
						strMessage += ch;
					else if (ch == 32)
						strMessage += ".";	// for non-printable chars
					else if (ch >= 33 && ch <= 254)
						strMessage += ch;
					// for wordpad to look good, who cares
					//else if (ch >= 33 && ch <= 159)
					//	strMessage += ch;
					//else if (ch == 160)
					//	strMessage += ".";	// for non-printable chars
					//else if (ch >= 161 && ch <= 174)
					//	strMessage += ch;
					//else if (ch == 175)
					//	strMessage += ".";	// for non-printable chars
					//else if (ch >= 176 && ch <= 254)
					//	strMessage += ch;
					else if (ch == 255)
						strMessage += ".";	// for non-printable chars
				}

				strMessage += "\n";
			}
			m_logMessage += CUtils::GetUnicodeString(strMessage.c_str());
		}
	};

}
