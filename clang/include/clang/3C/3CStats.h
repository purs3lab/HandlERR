//=--3CStats.h----------------------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// This class contains all the stats related to the conversion computed by 3C.
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_3C_3CSTATS_H
#define LLVM_CLANG_3C_3CSTATS_H

#include "llvm/Support/raw_ostream.h"

class PerformanceStats {
public:
  double CompileTime;
  double ConstraintBuilderTime;
  double ConstraintSolverTime;
  double ArrayBoundsInferenceTime;
  double RewritingTime;
  double TotalTime;

  // Rewrite Stats
  unsigned long NumAssumeBoundsCasts;
  unsigned long NumCheckedCasts;
  unsigned long NumWildCasts;
  unsigned long NumFixedCasts;
  unsigned long NumITypes;
  unsigned long NumCheckedRegions;
  unsigned long NumUnCheckedRegions;

  PerformanceStats() {
    CompileTime = ConstraintBuilderTime = 0;
    ConstraintSolverTime = ArrayBoundsInferenceTime = 0;
    RewritingTime = TotalTime = 0;

    CompileTimeSt = ConstraintBuilderTimeSt = 0;
    ConstraintSolverTimeSt = ArrayBoundsInferenceTimeSt = 0;
    RewritingTimeSt = TotalTimeSt = 0;

    NumAssumeBoundsCasts = NumCheckedCasts = 0;
    NumWildCasts = NumITypes = NumFixedCasts = 0;

    NumCheckedRegions = NumUnCheckedRegions = 0;
  }

  void startCompileTime();
  void endCompileTime();

  void startConstraintBuilderTime();
  void endConstraintBuilderTime();

  void startConstraintSolverTime();
  void endConstraintSolverTime();

  void startArrayBoundsInferenceTime();
  void endArrayBoundsInferenceTime();

  void startRewritingTime();
  void endRewritingTime();

  void startTotalTime();
  void endTotalTime();

  void incrementNumAssumeBounds();
  void incrementNumCheckedCasts();
  void incrementNumWildCasts();
  void incrementNumFixedCasts();
  void incrementNumITypes();
  void incrementNumCheckedRegions();
  void incrementNumUnCheckedRegions();

  void printPerformanceStats(llvm::raw_ostream &O, bool JsonFormat);

private:
  clock_t CompileTimeSt;
  clock_t ConstraintBuilderTimeSt;
  clock_t ConstraintSolverTimeSt;
  clock_t ArrayBoundsInferenceTimeSt;
  clock_t RewritingTimeSt;
  clock_t TotalTimeSt;

};

#endif