/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCTestVC.h"

#include "cmCTest.h"
#include "cmSystemTools.h"
#include "cmXMLSafe.h"

#include <cmsys/Process.h>

//----------------------------------------------------------------------------
cmCTestVC::cmCTestVC(cmCTest* ct, std::ostream& log): CTest(ct), Log(log)
{
  this->PathCount[PathUpdated] = 0;
  this->PathCount[PathModified] = 0;
  this->PathCount[PathConflicting] = 0;
  this->Unknown.Date = "Unknown";
  this->Unknown.Author = "Unknown";
  this->Unknown.Rev = "Unknown";
}

//----------------------------------------------------------------------------
cmCTestVC::~cmCTestVC()
{
}

//----------------------------------------------------------------------------
void cmCTestVC::SetCommandLineTool(std::string const& tool)
{
  this->CommandLineTool = tool;
}

//----------------------------------------------------------------------------
void cmCTestVC::SetSourceDirectory(std::string const& dir)
{
  this->SourceDirectory = dir;
}

//----------------------------------------------------------------------------
bool cmCTestVC::InitialCheckout(const char* command)
{
  cmCTestLog(this->CTest, HANDLER_OUTPUT,
             "   First perform the initial checkout: " << command << "\n");

  // Make the parent directory in which to perform the checkout.
  std::string parent = cmSystemTools::GetFilenamePath(this->SourceDirectory);
  cmCTestLog(this->CTest, HANDLER_OUTPUT,
             "   Perform checkout in directory: " << parent << "\n");
  if(!cmSystemTools::MakeDirectory(parent.c_str()))
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Cannot create directory: " << parent << std::endl);
    return false;
    }

  // Construct the initial checkout command line.
  std::vector<cmStdString> args = cmSystemTools::ParseArguments(command);
  std::vector<char const*> vc_co;
  for(std::vector<cmStdString>::const_iterator ai = args.begin();
      ai != args.end(); ++ai)
    {
    vc_co.push_back(ai->c_str());
    }
  vc_co.push_back(0);

  // Run the initial checkout command and log its output.
  this->Log << "--- Begin Initial Checkout ---\n";
  OutputLogger out(this->Log, "co-out> ");
  OutputLogger err(this->Log, "co-err> ");
  bool result = this->RunChild(&vc_co[0], &out, &err, parent.c_str());
  this->Log << "--- End Initial Checkout ---\n";
  if(!result)
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Initial checkout failed!" << std::endl);
    }
  return result;
}

//----------------------------------------------------------------------------
bool cmCTestVC::RunChild(char const* const* cmd, OutputParser* out,
                         OutputParser* err, const char* workDir)
{
  this->Log << this->ComputeCommandLine(cmd) << "\n";

  cmsysProcess* cp = cmsysProcess_New();
  cmsysProcess_SetCommand(cp, cmd);
  workDir = workDir? workDir : this->SourceDirectory.c_str();
  cmsysProcess_SetWorkingDirectory(cp, workDir);
  this->RunProcess(cp, out, err);
  int result = cmsysProcess_GetExitValue(cp);
  cmsysProcess_Delete(cp);
  return result == 0;
}

//----------------------------------------------------------------------------
std::string cmCTestVC::ComputeCommandLine(char const* const* cmd)
{
  cmOStringStream line;
  const char* sep = "";
  for(const char* const* arg = cmd; *arg; ++arg)
    {
    line << sep << "\"" << *arg << "\"";
    sep = " ";
    }
  return line.str();
}

//----------------------------------------------------------------------------
bool cmCTestVC::RunUpdateCommand(char const* const* cmd,
                                 OutputParser* out, OutputParser* err)
{
  // Report the command line.
  this->UpdateCommandLine = this->ComputeCommandLine(cmd);
  if(this->CTest->GetShowOnly())
    {
    this->Log << this->UpdateCommandLine << "\n";
    return true;
    }

  // Run the command.
  return this->RunChild(cmd, out, err);
}

//----------------------------------------------------------------------------
std::string cmCTestVC::GetNightlyTime()
{
  // Get the nightly start time corresponding to the current dau.
  struct tm* t = this->CTest->GetNightlyTime(
    this->CTest->GetCTestConfiguration("NightlyStartTime"),
    this->CTest->GetTomorrowTag());
  char current_time[1024];
  sprintf(current_time, "%04d-%02d-%02d %02d:%02d:%02d",
          t->tm_year + 1900,
          t->tm_mon + 1,
          t->tm_mday,
          t->tm_hour,
          t->tm_min,
          t->tm_sec);
  return std::string(current_time);
}

//----------------------------------------------------------------------------
void cmCTestVC::Cleanup()
{
  this->Log << "--- Begin Cleanup ---\n";
  this->CleanupImpl();
  this->Log << "--- End Cleanup ---\n";
}

//----------------------------------------------------------------------------
void cmCTestVC::CleanupImpl()
{
  // We do no cleanup by default.
}

//----------------------------------------------------------------------------
bool cmCTestVC::Update()
{
  this->NoteOldRevision();
  this->Log << "--- Begin Update ---\n";
  bool result = this->UpdateImpl();
  this->Log << "--- End Update ---\n";
  this->NoteNewRevision();
  return result;
}

//----------------------------------------------------------------------------
void cmCTestVC::NoteOldRevision()
{
  // We do nothing by default.
}

//----------------------------------------------------------------------------
void cmCTestVC::NoteNewRevision()
{
  // We do nothing by default.
}

//----------------------------------------------------------------------------
bool cmCTestVC::UpdateImpl()
{
  cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
             "* Unknown VCS tool, not updating!" << std::endl);
  return true;
}

//----------------------------------------------------------------------------
bool cmCTestVC::WriteXML(std::ostream& xml)
{
  this->Log << "--- Begin Revisions ---\n";
  bool result = this->WriteXMLUpdates(xml);
  this->Log << "--- End Revisions ---\n";
  return result;
}

//----------------------------------------------------------------------------
bool cmCTestVC::WriteXMLUpdates(std::ostream&)
{
  cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
             "* CTest cannot extract updates for this VCS tool.\n");
  return true;
}

//----------------------------------------------------------------------------
void cmCTestVC::WriteXMLEntry(std::ostream& xml,
                              std::string const& path,
                              std::string const& name,
                              std::string const& full,
                              File const& f)
{
  static const char* desc[3] = { "Updated", "Modified", "Conflicting"};
  Revision const& rev = f.Rev? *f.Rev : this->Unknown;
  std::string prior = f.PriorRev? f.PriorRev->Rev : std::string("Unknown");
  xml << "\t\t<" << desc[f.Status] << ">\n"
      << "\t\t\t<File>" << cmXMLSafe(name) << "</File>\n"
      << "\t\t\t<Directory>" << cmXMLSafe(path) << "</Directory>\n"
      << "\t\t\t<FullName>" << cmXMLSafe(full) << "</FullName>\n"
      << "\t\t\t<CheckinDate>" << cmXMLSafe(rev.Date) << "</CheckinDate>\n"
      << "\t\t\t<Author>" << cmXMLSafe(rev.Author) << "</Author>\n"
      << "\t\t\t<Email>" << cmXMLSafe(rev.EMail) << "</Email>\n"
      << "\t\t\t<Log>" << cmXMLSafe(rev.Log) << "</Log>\n"
      << "\t\t\t<Revision>" << cmXMLSafe(rev.Rev) << "</Revision>\n"
      << "\t\t\t<PriorRevision>" << cmXMLSafe(prior) << "</PriorRevision>\n"
      << "\t\t</" << desc[f.Status] << ">\n";
  ++this->PathCount[f.Status];
}
