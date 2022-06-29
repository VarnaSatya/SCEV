//===- Hello.cpp - Example code from "Writing an LLVM Pass" ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements two versions of the LLVM "Hello World" pass described
// in docs/WritingAnLLVMPass.html
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/DependenceAnalysis.h"


#include <list>
#include<vector>
#include <map>
using namespace llvm;

#define DEBUG_TYPE "hello"

static LLVMContext Context;


STATISTIC(HelloCounter, "Counts number of functions greeted");

namespace {
  // Hello - The first implementation, without getAnalysisUsage.
  struct Hello : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    Hello() : FunctionPass(ID) {}

    bool runOnFunction(Function &F) override {
      ++HelloCounter;
      errs() << "Hello: ";
      errs().write_escaped(F.getName()) << '\n';
      return false;
    }
  };
}

char Hello::ID = 0;
static RegisterPass<Hello> X("hello", "Hello World Pass");

namespace {
  // Hello2 - The second implementation with getAnalysisUsage implemented.
  struct Hello2 : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    Hello2() : FunctionPass(ID) {}

    bool runOnFunction(Function &F) override {

      Module *M=F.getParent();
      ++HelloCounter;
      
      auto &SE = getAnalysis<ScalarEvolutionWrapperPass>().getSE();
      int cur=0,prev=0,flag=0;
      std::map<int,Instruction*> storeStuff;
      Instruction *last;

      for (BasicBlock &BB : F)      
      {        
        if(BB.getName()=="for.body")
        {
          for (Instruction &I : BB)
          {
            Value *instruction= &I;
            if (auto *AI = dyn_cast<StoreInst>(instruction)) 
            { 
              const SCEV *s=SE.getSCEV(AI->getPointerOperand());
              const SCEV *p=SE.getMinusSCEV(s,SE.getPointerBase(s));

              errs()<<"\nStuff:"<<SE.getUnsignedRange(s)<<"\t"<<SE.getUnsignedRange(p);
              
              //cur=p->first addition operand

              if (cur<prev)
                flag=1;

              storeStuff[cur]=&I;              
            }             
            last=&I;
          }            
        }
      }

      if(flag==1) //if order of access is not in ascending order
      {
        std::map<int, Instruction* >::iterator itr;
        Instruction *l=last;

        for (itr = storeStuff.begin(); itr != storeStuff.end(); ++itr) 
        {
          itr->second->moveAfter(last);
          last=itr->second;
        }
        l->moveAfter(last);
      }

      errs()<<"\n\n";
      F.dump();

      return false;
    }

    // We don't modify the program, so we preserve all analyses.
    void getAnalysisUsage(AnalysisUsage &AU) const override 
    {
      AU.setPreservesAll();      
      AU.addRequired<ScalarEvolutionWrapperPass>();
    }
  };
}

char Hello2::ID = 0;
static RegisterPass<Hello2>
Y("hello2", "Hello World Pass (with getAnalysisUsage implemented)");
