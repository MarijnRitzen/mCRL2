// Author(s): Olav Bunte
// Copyright: see the accompanying file COPYING or copy at
// https://github.com/mCRL2org/mCRL2/blob/master/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef PROCESSSYSTEM_H
#define PROCESSSYSTEM_H

#include "filesystem.h"
#include "consoledock.h"

#include <QObject>
#include <QProcess>
#include <QQueue>
#include <QThread>

/**
 * @brief The ProcessThread class defines a thread that makes sure that
 *   processes of certain types (such as verification) happen after each other
 */
class ProcessThread : public QThread
{
  Q_OBJECT

  public:
  /**
   * @brief ProcessThread The constructor
   * @param processQueue The queue this thread needs to take the processes from
   * @param verification Whether this thread is for verification processes
   */
  ProcessThread(QQueue<int>* processQueue, ProcessType processType);

  /**
   * @brief run The body of the thread
   */
  void run() override;

  /**
   * @brief getCurrentProcessId Returns the id of the process this thread is
   *   currently running
   * @return The id of the process this thread is currently running
   */
  int getCurrentProcessId();

  public slots:
  /**
   * @brief newProcessQueued Is called when a new process is added, emits
   *   newProcessInQueue if this process has to be run by this thread
   * @param processType The type of the newly added process
   */
  void newProcessQueued(ProcessType processType);

  /**
   * @brief processFinished Is called when a process has finished, emits
   *   currentProcessFinished if this process is the one that this thread is
   *   running
   * @param processid The id of the process that has finished
   */
  void processFinished(int processid);

  signals:
  /**
   * @brief newProcessInQueue Activates this thread if it is waiting for a new
   *   process
   */
  void newProcessInQueue();

  /**
   * @brief startProcess Tells the process system that a process needs to be
   *   started
   * @param processid The id of the process that needs to be started
   */
  void startProcess(int processid);

  /**
   * @brief currentProcessFinished Activates this thread if it is waiting for a
   *   process to finish
   */
  void currentProcessFinished();

  /**
   * @brief isRunning Is emitted when this thread moves from running to
   *   waiting or vice versa
   * @param running Whether the thread is running (or waiting)
   */
  void isRunning(bool running);

  private:
  QQueue<int>* processQueue;
  ProcessType processType;
  int currentProcessid;
};

/**
 * @brief The ProcessSystem class handles all processes related to mCRL2 tools
 */
class ProcessSystem : public QObject
{
  Q_OBJECT

  public:
  /**
   * @brief ProcessSystem Constructor
   * @param fileSystem The file system
   */
  ProcessSystem(FileSystem* fileSystem);

  /**
   * @brief setConsoleDock Assigns the console dock to the file system for
   *   logging
   * @param consoleDock The console dock
   */
  void setConsoleDock(ConsoleDock* consoleDock);

  /**
   * @brief getProcessThread Returns gets the process thread of type processType
   * @param processType The type of the processThread
   * @return The process thread of type processType
   */
  ProcessThread* getProcessThread(ProcessType processType);

  /**
   * @brief parseSpecification Parses the current specification
   * @return The id of the parsing process
   */
  int parseSpecification();

  /**
   * @brief simulate Simulates the current specification using mcrl22lps and
   *   lpsxsim
   * @return The process id of the simulation process
   */
  int simulate();

  /**
   * @brief createLts Create and visualizes the lts of the current specification
   *   using mcrl22lps, lps2lts, optionally ltsconvert and ltsgraph
   * @param reduction What reduction to apply
   * @return The process id of the lts creation process
   */
  int createLts(LtsReduction reduction);

  /**
   * @brief verifyProperty Verifies a property using mcrl22lps, lps2pbes and
   *   pbes2bool
   * @param property The property to verify
   * @return The process id of the verification process
   */
  int verifyProperty(Property* property);

  /**
   * @brief abortProcess Aborts a process by making the running subprocess
   *   terminate if it is running, else by removing it from the queue
   * @param processid The process id of the process to abort
   */
  void abortProcess(int processid);

  /**
   * @brief abortAllProcesses Aborts all processes of type processType by
   *   clearing the corresponding queue and killing the corresponding currently
   *   running process
   * @param processType The processType to abort all processes of
   */
  void abortAllProcesses(ProcessType processType);

  /**
   * @brief getResult Gets the result of a process
   *   for a verification process, the result is either "" (in case of error),
   *   "false" or "true"
   * @param processid The id of the process to get the result from
   * @return The result of the process
   */
  QString getResult(int processid);

  signals:
  /**
   * @brief newProcessQueued Is emitted when a new process is added to a queue
   * @param processtype The type of the new process
   */
  void newProcessQueued(ProcessType processType);

  /**
   * @brief processFinished Is emitted when a process is finished
   * @param processid The id of the process that has finished
   */
  void processFinished(int processid);

  private:
  FileSystem* fileSystem;
  ConsoleDock* consoleDock;
  int pid;
  std::map<int, std::vector<QProcess*>> processes;
  std::map<int, ProcessType> processTypes;
  std::map<int, QString> results;
  std::map<ProcessType, QQueue<int>*> processQueues;
  std::map<ProcessType, ProcessThread*> processThreads;

  /**
   * @brief createMcrl22lpsProcess Creates a process to execute mcrl22lps on the
   *   current specification
   * @param processType Determines what console dock tab to use to log to
   * @return The mcrl22lps process
   */
  QProcess* createMcrl22lpsProcess(ProcessType processType);

  /**
   * @brief createMcrl2ParsingProcess Creates a process to parse the current
   *   mCRL2 specification using mcrl22lps
   * @return The parsing process
   */
  QProcess* createMcrl2ParsingProcess();

  /**
   * @brief createLpsxsimProcess Creates a process to execute lpsxsim on the lps
   *   that corresponds to the current specification
   * @return The lpsxsim process
   */
  QProcess* createLpsxsimProcess();

  /**
   * @brief createLps2ltsProcess Creates a process to execute lps2lts on the lps
   *   that corresponds to the current specification
   * @return The lps2lts process
   */
  QProcess* createLps2ltsProcess();

  /**
   * @brief createLtsconvertProcess Creates a process to execute ltsconvert on
   *   the lts that corresponds to the current specification
   * @param reduction The reduction to apply
   * @return The ltsconvert process
   */
  QProcess* createLtsconvertProcess(LtsReduction reduction);

  /**
   * @brief createLtsgraphProcess Creates a process to execute ltsgraph on the
   *   lts that corresponds to the current specification and the given reduction
   * @param reduction The reduction that was applied
   * @return The ltsgraph process
   */
  QProcess* createLtsgraphProcess(LtsReduction reduction);

  /**
   * @brief createLps2pbesProcess Creates a process to execute lps2pbes on the
   *   lps that corresponds to the current specification and a given property
   * @param propertyName The name of the property to create a pbes of
   * @return The lps2pbes process
   */
  QProcess* createLps2pbesProcess(QString propertyName);

  /**
   * @brief createPbes2boolProcess Creates a process to execute pbes2bool on the
   *   pbes that corresponds to the current specification and a given property
   * @param propertyName The name of the property that corresponds to the pbes
   * @return The pbes2bool process
   */
  QProcess* createPbes2boolProcess(QString propertyName);

  private slots:

  /**
   * @brief startProcess Starts a process
   * @param processid The id of the process to run
   */
  void startProcess(int processid);

  /**
   * @brief parseMcrl2 Parses an mCRL2 specification
   * @param processid The id of the process to run
   */
  void parseMcrl2(int processid);


  /**
   * @brief parsingResult Handles the result of parsing a specification
   */
  void mcrl2ParsingResult();

  /**
   * @brief createLps The first step of any process, creating the lps
   * @param processid The id of the process to run
   */
  void createLps(int processid);

  /**
   * @brief simulateLps The last step of simulation, visualizing the simulation
   */
  void simulateLps();

  /**
   * @brief createLts The second step of lts creation, creating the lts
   */
  void createLts();

  /**
   * @brief reduceLts An optional step of lts creation, reducing the lts
   */
  void reduceLts();

  /**
   * @brief showLts The last step of lts creation, visualizing the lts
   */
  void showLts();

  /**
   * @brief createPbes The second step of verification, creating the pbes
   */
  void createPbes();

  /**
   * @brief solvePbes The third step of verification, solving the pbes
   */
  void solvePbes();

  /**
   * @brief verificationResult Extracts and stores the result of the
   *   verification
   */
  void verificationResult();
};

#endif // PROCESSSYSTEM_H
