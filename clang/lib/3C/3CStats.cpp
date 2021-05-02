//=--3CStats.cpp--------------------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Implementation of all the methods in 3CStats.h
//===----------------------------------------------------------------------===//

#include "clang/3C/3CStats.h"
#include "clang/3C/Utils.h"
#include <time.h>

void PerformanceStats::startCompileTime() {
  CompileTimeSt = clock();
}

void PerformanceStats::endCompileTime() {
  CompileTime += getTimeSpentInSeconds(CompileTimeSt);
}

void PerformanceStats::startConstraintBuilderTime() {
  ConstraintBuilderTimeSt = clock();
}

void PerformanceStats::endConstraintBuilderTime() {
  ConstraintBuilderTime += getTimeSpentInSeconds(ConstraintBuilderTimeSt);
}

void PerformanceStats::startConstraintSolverTime() {
  ConstraintSolverTimeSt = clock();
}

void PerformanceStats::endConstraintSolverTime() {
  ConstraintSolverTime += getTimeSpentInSeconds(ConstraintSolverTimeSt);
}

void PerformanceStats::startArrayBoundsInferenceTime() {
  ArrayBoundsInferenceTimeSt = clock();
}

void PerformanceStats::endArrayBoundsInferenceTime() {
  ArrayBoundsInferenceTime += getTimeSpentInSeconds(ArrayBoundsInferenceTimeSt);
}

void PerformanceStats::startRewritingTime() {
  RewritingTimeSt = clock();
}

void PerformanceStats::endRewritingTime() {
  RewritingTime += getTimeSpentInSeconds(RewritingTimeSt);
}

void PerformanceStats::startTotalTime() {
  TotalTimeSt = clock();
}

void PerformanceStats::endTotalTime() {
  TotalTime += getTimeSpentInSeconds(TotalTimeSt);
}

void PerformanceStats::incrementNumAssumeBounds() {
  NumAssumeBoundsCasts++;
}
void PerformanceStats::incrementNumCheckedCasts() {
  NumCheckedCasts++;
}

void PerformanceStats::incrementNumWildCasts() {
  NumWildCasts++;
}

void PerformanceStats::incrementNumFixedCasts() {
  NumFixedCasts++;
}

void PerformanceStats::incrementNumITypes() {
  NumITypes++;
}

void PerformanceStats::incrementNumCheckedRegions() {
  NumCheckedRegions++;
}

void PerformanceStats::incrementNumUnCheckedRegions() {
  NumUnCheckedRegions++;
}


void PerformanceStats::printPerformanceStats(llvm::raw_ostream &O,
                                             bool JsonFormat) {
  if (JsonFormat) {
    O << "[";

    O << "{\"TimeStats\": {\"TotalTime\":" << TotalTime;
    O << ", \"ConstraintBuilderTime\":" << ConstraintBuilderTime;
    O << ", \"ConstraintSolverTime\":" << ConstraintSolverTime;
    O << ", \"ArrayBoundsInferenceTime\":" << ArrayBoundsInferenceTime;
    O << ", \"RewritingTime\":" << RewritingTime;
    O << "}},\n";

    O << "{\"ReWriteStats\":{";
    O << "\"NumAssumeBoundsCasts\":" << NumAssumeBoundsCasts;
    O << ", \"NumCheckedCasts\":" << NumCheckedCasts;
    O << ", \"NumWildCasts\":" << NumWildCasts;
    O << ", \"NumFixedCasts\":" << NumFixedCasts;
    O << ", \"NumITypes\":" << NumITypes;
    O << ", \"NumCheckedRegions\":" << NumCheckedRegions;
    O << ", \"NumUnCheckedRegions\":" << NumUnCheckedRegions;
    O << "}}";

    O << "]";
  } else {
    O << "TimeStats\n";
    O << "TotalTime:" << TotalTime << "\n";
    O << "ConstraintBuilderTime:" << ConstraintBuilderTime << "\n";
    O << "ConstraintSolverTime:" << ConstraintSolverTime << "\n";
    O << "ArrayBoundsInferenceTime:" << ArrayBoundsInferenceTime << "\n";
    O << "RewritingTime:" << RewritingTime << "\n";

    O << "ReWriteStats\n";
    O << "NumAssumeBoundsCasts:" << NumAssumeBoundsCasts << "\n";
    O << "NumCheckedCasts:" << NumCheckedCasts << "\n";
    O << "NumWildCasts:" << NumWildCasts << "\n";
    O << "NumFixedCasts:" << NumFixedCasts << "\n";
    O << "NumITypes:" << NumITypes << "\n";
    O << "NumCheckedRegions:" << NumCheckedRegions << "\n";
    O << "NumUnCheckedRegions:" << NumUnCheckedRegions << "\n";

  }
}