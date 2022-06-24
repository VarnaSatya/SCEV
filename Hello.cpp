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
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include <list>
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
      //errs() << "Hello: ";
      //errs().write_escaped(F.getName()) << '\n';
      //Module *M=F.getParent();
      //IRBuilder<> IR(M->getContext());
      int key;
      //auto &SE = getAnalysis<ScalarEvolutionWrapperPass>().getSE();
      std::list<Instruction*> instrList;
      std::map<int, std::list<Instruction*> > assgnInstrs;
      IRBuilder<> IR(M->getContext());
      for (BasicBlock &BB : F)
      {
        //errs() << "\nBasic block (name=" << BB.getName() << ") has "<< BB.size() << " instructions.\nPredecessors:\n";
        
        if(BB.getName()=="for.body")
        {
          key=0;
          for (Instruction &I : BB)
          {
            //errs()<<"\nInstructions:\n"<<I;
            Value *instruction= &I;

            instrList.push_back(&I);
            //errs() << "\nOpernads number:"<<I.getNumOperands();
            if (auto *AI = dyn_cast<LoadInst>(instruction)) 
            {
              //errs() << "\nLoad Instrction:"<<&AI<<I;             
            }
            if (I.getNumOperands()==2)
            {
              if (ConstantInt* CI = dyn_cast<ConstantInt>(I.getOperand(1))) 
              {
                  if (CI->getBitWidth() <= 32) 
                  {
                    int constIntValue = CI->getSExtValue();
                    errs() <<"\nOperand2: "<< constIntValue<<"\n";
                    key=constIntValue;
                  }
              }
            } 
             if (auto *AI = dyn_cast<StoreInst>(instruction)) 
            {
              //errs() << "\nStore Instrction:"<<*AI<<I;
              //errs() << "\nInstruction list size:"<<instrList.size()<<"\n"<<*instrList.front();
              assgnInstrs[key]=instrList;
              key=0;
              instrList.clear();
            }
            //errs()<<"\n";
  
          }
            
        }
      }
        //errs()<<"\nMAP: "<<assgnInstrs<<"\n";
        //errs() << "\tKEY\tELEMENT\n";
        std::map<int, std::list<Instruction*> >::iterator itr;
        Instruction* prev;
        
        for (itr = assgnInstrs.begin(); itr != assgnInstrs.end(); ++itr) 
        {          
          //errs() << '\t' << itr->first << '\t' << *itr->second.front() << '\n';
          key=itr->first;
          errs() << "\nkey: " << key <<"\n";
          if(key==0)
          {  
            prev=itr->second.back();
            errs() << *prev << "\n";
            continue;
          }
          else
          {
            //std::list<Instruction*>::iterator it;
            //errs() << "\n"<<*itr->second.front();

            for(const auto& line : itr->second)
            {
              //errs() <<"PREV:" <<*prev <<"\n";
              errs() << *line <<"\n";
              //line->removeFromParent();
              line->moveAfter(prev);
              prev=line;
              //break;
              //errs() << *prev << "\n";

            }
          }
          
        }

      return false;
    }

    // We don't modify the program, so we preserve all analyses.
    void getAnalysisUsage(AnalysisUsage &AU) const override 
    {
      //AU.setPreservesAll();      
      auto *SE = getAnalysisIfAvailable<ScalarEvolutionWrapperPass>();
      //AU.addRequired<LoopStandardAnalysisResults>();
      AU.addRequired<ScalarEvolutionWrapperPass>();
    }
    /*bool runOnLoop(Loop *L, LPPassManager &LPM) override 
    {
      auto *SE = getAnalysisIfAvailable<ScalarEvolutionWrapperPass>();
    }*/
  };
}

char Hello2::ID = 0;
static RegisterPass<Hello2>
Y("hello2", "Hello World Pass (with getAnalysisUsage implemented)");
