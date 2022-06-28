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
      
      auto &SE = getAnalysis<ScalarEvolutionWrapperPass>().getSE();
      int key;
      std::list<Instruction*> instrList;
      std::map<int, StoreInst* > assgnInstrs;
      //std::map<int, std::list<Value*> > assgnInstrs;
      IRBuilder<> IR(M->getContext());
      for (BasicBlock &BB : F)
      {
        
        if(BB.getName()=="for.body")
        {
          key=0;
          for (Instruction &I : BB)
          {
            Value *instruction= &I;

            if (auto *AI = dyn_cast<LoadInst>(instruction)) 
            {
              //errs() << "\nLoad Instrction:"<<&AI<<I;
              //instrList.push_back(AI->getPointerOperand()); 
              //const SCEV *BaseExpr = SE.getSCEV(AI->getPointerOperand());              
              //errs() << "\nLoad: "<<*BaseExpr;              
              //errs() << "\nLoad2: "<<*SE.getPointerBase(BaseExpr)<<" "<<*SE.getMinusSCEV(SE.getPointerBase(BaseExpr),SE.getPointerBase(BaseExpr));
              //instrList.push_back(&I);            
            }
            else if (auto *AI = dyn_cast<StoreInst>(instruction)) 
            {
              Value *v=AI->getPointerOperand();
              errs() << "\nStore Instrction:"<<*v;
              instrList.push_back(&I);
              assgnInstrs[key]=AI;
              key=0;
              instrList.clear();
            }
            else
            {
              Value *val = dyn_cast<Value>(&I);
              instrList.push_back(&I);
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
            } 
             
            //errs()<<"\n";
  
          }
            
        }
      }
        //errs()<<"\nMAP: "<<assgnInstrs<<"\n";
        //errs() << "\tKEY\tELEMENT\n";
        std::map<int, StoreInst* >::iterator itr;
        StoreInst *prev;
        //Value* prev;
        
        for (itr = assgnInstrs.begin(); itr != assgnInstrs.end(); ++itr) 
        {          
          //errs() << '\t' << itr->first << '\t' << *itr->second.front() << '\n';
          key=itr->first;
          errs() << "\nkey: " << key;
          if(key==0)
          {  
            prev=itr->second;
            errs() <<"\n"<<*prev ;
            continue;
          }
          else
          {
            //std::list<Instruction*>::iterator it;
            //errs() << "\n"<<*itr->second.front();
            //for(const auto& line : itr->second)
            //{
            //errs() <<"PREV:" <<*prev <<"\n";
            errs() << *itr->second <<"\n";
            const SCEV *BaseExpr = SE.getSCEV(itr->second->getPointerOperand());
            //line->removeFromParent();
            //line->moveAfter(prev);
            errs() << "\nSCEV Minus\n"<<*BaseExpr<<"\n"<<*SE.getPointerBase(BaseExpr)<<"\n"<<*itr->second->getPointerOperand()<<"\n"<<*SE.getMinusSCEV(SE.getPointerBase(BaseExpr),BaseExpr)<<"\n";
            //prev=line;
            //break;
            //errs() << *prev << "\n";
            //}
          }

          
          
        }

      //F.dump();
      return false;
    }

    // We don't modify the program, so we preserve all analyses.
    void getAnalysisUsage(AnalysisUsage &AU) const override 
    {
      AU.setPreservesAll();      
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
