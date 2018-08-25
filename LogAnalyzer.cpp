#include "LogAnalyzer.h"
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <cctype>
#include <locale>
#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <fstream>
#include <string>
using namespace std;


LogAnalyzer::LogAnalyzer(std::string const & LogFileName, int const & logFileNum, std::string const & callid2find, std::ofstream& resultFile)
  : LogFileName_(LogFileName)
  , startingLogFileNum_(logFileNum)
  , currentLogFileNum_(logFileNum)
  , tid2find_()
  , callid2find_(callid2find)
  , resultFile_(resultFile)
{
}

LogAnalyzer::~LogAnalyzer() {}


/* ============================================================================ 
**  The search a specific log file for tid or for callid.
** ============================================================================ */
bool LogAnalyzer::parseSingleLogFile()
{
  auto const done = string{"done"};
  bool continue_looping_log_files = true;

  ifstream log_file;
  string  LogFileName; // Construct the name of thie log file from the NAME and current log file number
  if (currentLogFileNum_ == 0)
    {
      LogFileName = LogFileName_;
  	}
  else // currentLogFileNum > 1
    {
	  LogFileName = LogFileName_ + "." + to_string(currentLogFileNum_);
  	}

  
  log_file.open(LogFileName.c_str());
  if (log_file.is_open())
  {
  auto event_str = string{};
  auto get_next_logfile_num = bool{true};

	while (getline(log_file, event_str))
    {
	  // Check whether the requested callid is present in this line:
   	  string callid2search = "callid = " + callid2find_ + ',';
   	  bool callid_found = (event_str.find(callid2search)) != string::npos;
      if ((tid2find_.empty)() || (tid2find_ == done))
      {
	    /* This is lookup for callid, either preliminary - to get the tid to look for
		   before continue searchung for the callid, or post-tid lookup for callid.
		*/
		if (callid_found)
		{
		  // Requested callid found!
		  if (tid2find_.empty())
		  {
		  	/* This is the preliminary lookup for callid (to get the tid for the
			** tid search), and this is the 1st time when the requested callid is
			** found. Get the tid on this line and start search from the the log
			** file entered by the user for the tid found in this event:
		    */
		    setTid2Find(getValueOfTid(event_str));
		    setLogFileNum2ResumeCallidLkp(currentLogFileNum_);
		    log_file.close();
		    // Remember current log file number to resume call id lookup later with is
	        setCurrentLogFileNum(startingLogFileNum_);

		    loopThroughLogfiles(); 
		  }
		  else
		  {
		    /* callid matches and we are looking for only callid:
		    ** - add the whole line to the output File of events for this call
            */
		    resultFile_ << event_str << endl;
		  }
	 	} // Requested callid is found
      }
	  else
	  {
        // This is a search for tid 

      	string tid2search = "tid = " + tid2find_ + ','; // tid always has a comma ',' right after its value

        if (event_str.find(tid2search) != string::npos)
	    {
          // This line has the tid value we are looking for - record it
		  resultFile_ << event_str << endl;

		  if (callid_found)
		  {
		    /* callid matches and the tid2search matches - this is that first event
	           with the requested callid: set tid2Find to "done to indicate that 
	           from now on this method will be resume and finish callid search
		    */
		    setTid2Find(done);
	        setCurrentLogFileNum(LogFileNum2ResumeCallidLkp_);
            get_next_logfile_num = false;
		    continue_looping_log_files = false;
	 	  }
        }
      } 
    } 
    log_file.close();

	if (get_next_logfile_num)
	{
      if (currentLogFileNum_ == 0)
      {
	    // No more log files left to search
        continue_looping_log_files = false;
  	  }
  	  else
  	  {
        setCurrentLogFileNum(currentLogFileNum_ - 1);
  	  }
	}
  } // log_fle.is_open
  else
  {
  	if (!tid2find_.empty())
	  {
      cout << "Unable to open file " << LogFileName << " : finishing search for tid=" << tid2find_ << endl;
	  }
	  else
	  {
      cout << "Unable to open file " << LogFileName << " : finishing search for callid=" << callid2find_ << endl;
	  }
	continue_looping_log_files = false; // Stop loopng through log files
  }
  return (continue_looping_log_files);
}


/* ============================================================================ 
** Obtain the log file number from the user. 
** If the number enterd is biger than the number of the oldest log file - 
** use the number of that oldest log file as the starting point for the lookup.
** ============================================================================ */
int LogAnalyzer::getStartingLogFileNum(int const & maxNumLogFile) 
{
  string logFileNum_str = "bad";
  int logFileNum = maxNumLogFile + 1, i = 0;
  while (true)
	{
    cout << "Enter the number of the log file to start the search with:" << endl;
    cout << "   0 - to search only the most recent log file that has no number " << endl;
	  cout << "   1.." <<  maxNumLogFile << " - to start search with this specific log file" << endl;
	  cout << "   99 - to search all log files with provided name " << endl;
	  cout << " : " ;
    cin >> logFileNum_str;
    for (i = 0; i < logFileNum_str.length(); i++)
	  {
	    if (!isdigit(logFileNum_str[i]))
	    {
	   	  //Ths input suppose to have only decimal digits: just re-ask to enter
	      break;
	    }
	  }

	  if (i == logFileNum_str.length())
	  {
      logFileNum = atoi(logFileNum_str.c_str());
	  	// Input was all digits - check whether the range is valid
	  	if ((logFileNum >= 0) &&  (logFileNum <= maxNumLogFile))
	  	{
	  	  // Valid range - break the 'while' loop for rntering log file number
	  	  break;
	  	}
	  }
  }
  return(logFileNum);
}


/* ============================================================================ 
**  Extract the value of the "tid" from the event line
** ============================================================================ */
string LogAnalyzer::getValueOfTid(string const & event_str) const
{
  string tid2search = "tid = ";
  size_t char_pos = event_str.find(tid2search);
  string tid_str_val;
  if (char_pos != string::npos)
  {
    tid_str_val = event_str.substr(char_pos + sizeof("tid = ") -1);

    // The obtained value can be at the end of the line, or can end with a comma ','
    char_pos = tid_str_val.find(',', 0);

    // If the comma ',' after the id value is found - limit the value until before the comma.
    // If comma was not found - this is the end of the line: nothing to do.
    if (char_pos != string::npos)
    {
      tid_str_val = tid_str_val.substr(0, char_pos);
    }
  }

  return(tid_str_val);
}
