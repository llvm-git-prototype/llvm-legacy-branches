//===- CocoaConventions.h - Special handling of Cocoa conventions -*- C++ -*--//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements cocoa naming convention analysis. 
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_ANALYSIS_DS_COCOA
#define LLVM_CLANG_ANALYSIS_DS_COCOA

#include "clang/AST/Type.h"

namespace clang {
  
class ObjCMethodDecl;
  
namespace ento {
namespace cocoa {
 
  enum NamingConvention { NoConvention, CreateRule, InitRule };

  NamingConvention deriveNamingConvention(Selector S, const ObjCMethodDecl *MD);

  static inline bool followsFundamentalRule(Selector S, 
                                            const ObjCMethodDecl *MD) {
    return deriveNamingConvention(S, MD) == CreateRule;
  }
  
  bool isRefType(QualType RetTy, llvm::StringRef Prefix,
                 llvm::StringRef Name = llvm::StringRef());
  
  bool isCFObjectRef(QualType T);
  
  bool isCocoaObjectRef(QualType T);

}}}

#endif
