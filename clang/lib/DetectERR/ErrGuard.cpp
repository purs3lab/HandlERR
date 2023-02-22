#include "clang/DetectERR/ErrGruard.h"

std::map<GuardLevel, std::string> ErrGuard::GuardLevelLabel = {
    {GuardLevel::Inner, "Inner"},
    {GuardLevel::Outer, "Outer"},
    {GuardLevel::Default, "Default"}};

std::map<HeuristicID, std::string> ErrGuard::HeuristicLabel = {
    {HeuristicID::H02, "H02"},      {HeuristicID::H03, "H03"},
    {HeuristicID::H04, "H04"},      {HeuristicID::H05, "H05"},
    {HeuristicID::H06, "H06"},      {HeuristicID::H07, "H07"},
    {HeuristicID::H08, "H08"},      {HeuristicID::H09, "H09"},
    {HeuristicID::FIFUZZ, "FIFUZZ"}};

std::string ErrGuard::toJsonString() const {
  // {
  //     "FunctionInfo": {
  //         "Name": "bar",
  //         "File": "/home/shank/code/research/HandlERR/clang/tools/detecterr/utils/tests/retnull.c"
  //     },
  //     "ErrConditions": [
  //         {
  //             "File": "/home/shank/code/research/HandlERR/clang/tools/detecterr/utils/tests/retnull.c",
  //             "LineNo": 25,
  //             "ColNo": 3,
  //             "Heuristic": "H04",
  //             "Level": "Inner",
  //             "ErrorLoc": {
  //                 "File": "/home/shank/code/research/HandlERR/clang/tools/detecterr/utils/tests/retnull.c",
  //                 "LineNo": 26,
  //                 "ColNo": 5
  //             }
  //         }
  //     ]
  // }
  return "{\"File\":\"" + GuardLoc.getFileName() +
         "\", \"LineNo\":" + std::to_string(GuardLoc.getLineNo()) +
         ", \"ColNo\":" + std::to_string(GuardLoc.getColSNo()) +
         ", \"Heuristic\":\"" + HeuristicLabel[HID] + "\"" + ", \"Level\":\"" +
         GuardLevelLabel[Level] + "\", \"ErrorLoc\":" + ErrLoc.toJsonString() +
         "}";
}
