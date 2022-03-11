#include "clang/DetectERR/ErrGruard.h"

std::map<HeuristicID, std::string> ErrGuard::HeuristicLabel = {
    {HeuristicID::H02, "H02"}, {HeuristicID::H03, "H03"},
    {HeuristicID::H04, "H04"}, {HeuristicID::H05, "H05"},
    {HeuristicID::H06, "H06"}, {HeuristicID::H07, "H07"},
    {HeuristicID::H08, "H08"},
};

std::string ErrGuard::toJsonString() const {
  return "{\"File\":\"" + PSL.getFileName() +
         "\", \"LineNo\":" + std::to_string(PSL.getLineNo()) +
         ", \"ColNo\":" + std::to_string(PSL.getColSNo()) +
         ", \"Heuristic\":\"" + HeuristicLabel[HID] + "\"" + "}";
}
