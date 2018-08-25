/* The task:
 There is an application that logs signaling events for the IP telephony calls (~500 signaling events per call).
 Each call is identified by a unique call-id after some initial events have already been logged.
 Until then the calls are identified by a transaction-id (tid).

 Assume there is at least one log entry in the file like 'tid= ttttttt, callid = ccccccc'.
 Write a log analyzer that extracts information for a call-id, in order e.g. for callid = 4882786.
 Describe the algorithm and make assumptions to show how you work with typical log rotation type of log files.
 Write the code in C++ for implementation and explain the design tradeoffs and flexibility considerations for future enhancements.

Algorithm:
----------
1.	Get user input for the following:
    - "callid" to search,
	  - log file name (including path if needed),
    - log file number to start the search with (0-99, where 0 means no numbers - the latest file, e.g. sip.log),
	  - file name to record the results of the search in it (or "d" to display on a screen).

2.  Start the search for the requested "callid" from the log file entered by the user 
    with the log file number entered by the user if such file exists, or with the oldest
    existing log file, which have the biggest number, which is less then the entered by the user.
    Older log files (if exist), which have bigger numbers in their names, are not involved in the search.
    The search will continue through the rest of the log files in sequential order,
	  each time flowing to the next file with the number in the file name decremented by 1,
	  and ending with the most recent log file without number in its name.

3.	Find the 1st line with the requested "callid" and obtain "tid" value from this line.

4.	Pause the search for "callid" and perform the search for the obtained "tid" value 
    starting with the same log file as the search for the "callid" started, 
	  record each found event with matching "tid" into the resulting file until 
    the event with this "tid" and "callid" is reached.

5.	After all files are searched for "tid" up to the file and the line where the matching "callid"
    was found for the 1st time, resume the search for the "callid": record each found event with
	  the matching "callid" into the same output file after the events with matching "tid" (if any).

6.	If no matching events were found (either "callid" was not found or there is no log files
    to look in) - write into the resulting file a statement abot that.

Assumptions:
------------
01. All log files are located in the same directory.

02. The numbering convention of the log files is as follows:
    - the most recent log file is called <Name>.log ;
    - next is <Name>.log.1, next is <Name>.log.2, etc. The highest number - the oldest file is <Name>.log.99 ;
    When the most recent log file, <Name>.log if filled up, the rest of log files are renamed
	  by incrementing the number in their names by 1. If the oldest file, <Name>.99 exists - it gets 
    deleted to allow for the previously named <Name>.log.98 to become <Name>.log.99 ;

03. The events of multiple calls are intermixed, so the events for each call are not grouped together.

04. The events, belonging to a specific call, that do not have "callid", are idenified by "tid",
    which is the same same as the "tid" in the 1st event with the requested "callid".

05. All events with "tid" that do not have "callid" are located in the log files before
    the 1st event that have matching "callid". So, there is no need to search for the events
	  with the obtained "tid" past this 1st found event with matching "callid".

06. All events of the specific call after the 1st event with the "callid" have the same "callid".

07. Each event in the log file always has at least "tid" & "callid" fields in this exact format:
    "tid = tttttttt," and "callid = ," : field name in lowercase, followed by a single space, 
    followed by an "=" sign, followed by a single space, followed by the field's value, folowed 
    by a comma ",".
    Any deviation from this format is considered "invalid entry" and is not getting recorded
    ito the otput file (or displayed on a screen).

08. Each event in log files is represented by a single line.

09. Both, "callid" and "tid", are stored in the log files as ASCII strings.


Design tradeoffs:
-----------------
1.	With more information about how exactly the events with "callid" and the events without "callid"
    are placed in the log files, probably would be possible to limit search ranges: if we definitely
	knew that all events of each call are grouped together and are not intermixed with the events of
	other calls, we could search backwards from the line where the matching "callid" is found for the
	1st time, find all records with matching "tid" and stop the search when the record with some other
	"callid" (of the previous call) is found.
	Similarly, when searching forward for more records with the "callid", stop the search when the line
	without "callid" is found - i.e. this call is finished and the new call is started.
	The search time would be shorter, but this assumption that events of each call are grouped together
	and are not intermixed with the events of other calls does not seem very realistic.



Flexibility considerations for future enhancements:
---------------------------------------------------
1.	Along with the base log file name and the output file, the user is allowed to enter the log 
    file number to start the search: if there is a significant number of log files or/and the log 
    files are very big, and the user knows that the call he/she is looking for is in one of the 
    latest log files - there is no sence to waste time for searching older log files, in which 
    the needed call defiitely cannot be.

2. Currently, the maximum number of log files is assumed 100, i.e. the log file numbers are between 1 and 99.
   If this changes, a single const variable need to be modified with the new value.
*/   

#include "LogAnalyzer.h"
#include <fstream>
#include <string>
#include <iostream>

using namespace std;

int main()
{
  const int maxNumLogFile = 99;

  // The user shall provide the callid to search and he log file to start he search wih:
  cout << "Enter callid: ";
  string callid2Find;
  cin >> callid2Find;

  string logFileName;
  cout << endl << "Enter log(s) file name (including path if needed," << endl;
  cout << "   e.g. sip.log, or ./logs/sip.log) to search in: ";
  cin >> logFileName;
  cout << endl;

  // Get from the user the log file number to start the lookup
  int logFileNum = LogAnalyzer::getStartingLogFileNum(maxNumLogFile);

  // Find the oldest present log file (with the biggest number) based on the user's input
  string tempLogFileName;
  int i;
  for (i = logFileNum; i >= 0 ; i--)
  {
    tempLogFileName = logFileName + "." + to_string(i);
    ifstream f(tempLogFileName.c_str());
	  if (f.good())
	  {
	    // God file is found with the largest number (from 99 down)
      logFileNum = i;
	  break;
	  }
  }


  cout << endl;
  string resultFileName;
  ofstream resultFile;
  bool display_only = false;
  char temp_choice = '?';
  while (true)
  {
	  cout << "Enter the file name for the results of the search (d - to display on screen): ";
	  cin >> resultFileName;
	  if (resultFileName != "d")
	  {
	    // Check whether a file with this name already exists and warn user
	    ifstream f(resultFileName.c_str());
		  if (f.good())
	    {
		    cout << "A file with the name " << resultFileName << " already exists!" << endl;
		    while ((temp_choice != '1' ) && (temp_choice != '2'))
	 	    {
		      cout << "Do you wan to overwrite it (1) or to choose a different name (2) ? : ";
			    cin >> temp_choice;
		    }
		    if (temp_choice == '1')
		    {
		  	  // Open the ofstream in trunc mode and break the asking loop
	        ofstream resultFile(resultFileName.c_str(), ios::app);
	        break;
		    }
		    else
		    {
		  	  // Re-ask to enter the output file name
		    }
		  }
		  else
		  {
		    // The file with the entered name does not exist - we'll use it
		    break;
		  }
    }
    else
    {
      display_only = true;
      /* The output file name entered is "d" - display on a screen
      ** If such file exists - overwrite its content with new results
		  ** and delete after after displaying
		  */
      ofstream resultFile(resultFileName.c_str());
      break;
    }
  }

  resultFile.open(resultFileName.c_str());

  cout << endl << "  Search for the events with the callid = " << callid2Find << endl;
  cout << "  will start with the log file " << logFileName  << "." << logFileNum << endl;
  if (display_only)
  {
    cout << "  The results will be displayed on a scren" << endl << endl;
  }
  else
  {
    cout << "  The results will be recorded in the file " << resultFileName << endl << endl;
  }


  LogAnalyzer logAnalyzer(logFileName, logFileNum, callid2Find, resultFile);
  logAnalyzer.loopThroughLogfiles();

  // If nothing is found - write into the resulting file "Nothing was found"
  resultFile.seekp(0, resultFile.end);
  int length = resultFile.tellp();
  if (length == 0)
  {
	  resultFile << "No records were found for the callid = " << callid2Find << endl;
  }

  resultFile.close();
  if (display_only)
  {
    cout << ifstream(resultFileName.c_str()).rdbuf() << '\n';
    if (remove(resultFileName.c_str()) !=0)
    {
      cout << "Remove operation for the file" << resultFileName << " failed!!" << endl;
    }
  }

  return(0);
}

