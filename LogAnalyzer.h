#ifndef LOG_ANALYZER_H
#define LOG_ANALYZER_H 

#include <fstream>
#include <string>


class LogAnalyzer
{
  private:
    // VARIABLES
    std::string LogFileName_;
    int         startingLogFileNum_;
    int         currentLogFileNum_;
    std::string tid2find_;
    std::string callid2find_;
    std::ofstream& resultFile_;
    int         LogFileNum2ResumeCallidLkp_;
    
    // MANIPILATORS
    bool        parseSingleLogFile();
	  void        setCurrentLogFileNum(int newNumber) {currentLogFileNum_ = newNumber;}
	  void        setLogFileNum2ResumeCallidLkp(int newNumber) {LogFileNum2ResumeCallidLkp_ = newNumber;}
	  void        setTid2Find(std::string const & newTid) {tid2find_ = newTid;}

    // ACESSORS
    std::string getValueOfTid(std::string const & event_str) const;

  public:
    // CREATORS
    LogAnalyzer(std::string const & LogFileName, int const & logFileNum, std::string const & callid2find, std::ofstream& resultFile); // constructor
    ~LogAnalyzer(); // destructor

    // MANPULATORS
    void loopThroughLogfiles() {do { } while(parseSingleLogFile());}

    // ACCESSORS
    static int  getStartingLogFileNum(int const & maxNumLogFile);
};

#endif   // LOG_ANALYZER_H