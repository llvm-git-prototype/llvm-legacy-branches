//===-- SystemZAsmPrinter.cpp - SystemZ LLVM assembly writer ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to the SystemZ assembly language.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "asm-printer"
#include "SystemZ.h"
#include "SystemZInstrInfo.h"
#include "SystemZTargetMachine.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Module.h"
#include "llvm/Assembly/Writer.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/DwarfWriter.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include "llvm/Target/TargetRegistry.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
using namespace llvm;

STATISTIC(EmittedInsts, "Number of machine instrs printed");

namespace {
  class SystemZAsmPrinter : public AsmPrinter {
  public:
    SystemZAsmPrinter(formatted_raw_ostream &O, TargetMachine &TM,
                      const MCAsmInfo *MAI, bool V)
      : AsmPrinter(O, TM, MAI, V) {}

    virtual const char *getPassName() const {
      return "SystemZ Assembly Printer";
    }

    void printOperand(const MachineInstr *MI, int OpNum,
                      const char* Modifier = 0);
    void printPCRelImmOperand(const MachineInstr *MI, int OpNum);
    void printRIAddrOperand(const MachineInstr *MI, int OpNum,
                            const char* Modifier = 0);
    void printRRIAddrOperand(const MachineInstr *MI, int OpNum,
                             const char* Modifier = 0);
    void printS16ImmOperand(const MachineInstr *MI, int OpNum) {
      O << (int16_t)MI->getOperand(OpNum).getImm();
    }
    void printS32ImmOperand(const MachineInstr *MI, int OpNum) {
      O << (int32_t)MI->getOperand(OpNum).getImm();
    }

    void printInstruction(const MachineInstr *MI);  // autogenerated.
    static const char *getRegisterName(unsigned RegNo);

    void printMachineInstruction(const MachineInstr * MI);

    void emitFunctionHeader(const MachineFunction &MF);
    bool runOnMachineFunction(MachineFunction &F);
    void PrintGlobalVariable(const GlobalVariable* GVar);

    void getAnalysisUsage(AnalysisUsage &AU) const {
      AsmPrinter::getAnalysisUsage(AU);
      AU.setPreservesAll();
    }
  };
} // end of anonymous namespace

#include "SystemZGenAsmWriter.inc"

void SystemZAsmPrinter::emitFunctionHeader(const MachineFunction &MF) {
  unsigned FnAlign = MF.getAlignment();
  const Function *F = MF.getFunction();

  OutStreamer.SwitchSection(getObjFileLowering().SectionForGlobal(F, Mang, TM));

  EmitAlignment(FnAlign, F);

  switch (F->getLinkage()) {
  default: assert(0 && "Unknown linkage type!");
  case Function::InternalLinkage:  // Symbols default to internal.
  case Function::PrivateLinkage:
  case Function::LinkerPrivateLinkage:
    break;
  case Function::ExternalLinkage:
    O << "\t.globl\t";
    CurrentFnSym->print(O, MAI);
    O << '\n';
    break;
  case Function::LinkOnceAnyLinkage:
  case Function::LinkOnceODRLinkage:
  case Function::WeakAnyLinkage:
  case Function::WeakODRLinkage:
    O << "\t.weak\t";
    CurrentFnSym->print(O, MAI);
    O << '\n';
    break;
  }

  printVisibility(CurrentFnSym, F->getVisibility());

  O << "\t.type\t";
  CurrentFnSym->print(O, MAI);
  O << ",@function\n";
  CurrentFnSym->print(O, MAI);
  O << ":\n";
}

bool SystemZAsmPrinter::runOnMachineFunction(MachineFunction &MF) {
  SetupMachineFunction(MF);
  O << "\n\n";

  // Print out constants referenced by the function
  EmitConstantPool(MF.getConstantPool());

  // Print the 'header' of function
  emitFunctionHeader(MF);

  // Print out code for the function.
  for (MachineFunction::const_iterator I = MF.begin(), E = MF.end();
       I != E; ++I) {
    // Print a label for the basic block.
    EmitBasicBlockStart(I);

    for (MachineBasicBlock::const_iterator II = I->begin(), E = I->end();
         II != E; ++II)
      // Print the assembly for the instruction.
      printMachineInstruction(II);
  }

  if (MAI->hasDotTypeDotSizeDirective()) {
    O << "\t.size\t";
    CurrentFnSym->print(O, MAI);
    O << ", .-";
    CurrentFnSym->print(O, MAI);
    O << '\n';
  }

  // Print out jump tables referenced by the function.
  EmitJumpTableInfo(MF.getJumpTableInfo(), MF);

  // We didn't modify anything
  return false;
}

void SystemZAsmPrinter::printMachineInstruction(const MachineInstr *MI) {
  ++EmittedInsts;

  processDebugLoc(MI, true);

  // Call the autogenerated instruction printer routines.
  printInstruction(MI);
  
  if (VerboseAsm)
    EmitComments(*MI);
  O << '\n';

  processDebugLoc(MI, false);
}

void SystemZAsmPrinter::printPCRelImmOperand(const MachineInstr *MI, int OpNum){
  const MachineOperand &MO = MI->getOperand(OpNum);
  switch (MO.getType()) {
  case MachineOperand::MO_Immediate:
    O << MO.getImm();
    return;
  case MachineOperand::MO_MachineBasicBlock:
    GetMBBSymbol(MO.getMBB()->getNumber())->print(O, MAI);
    return;
  case MachineOperand::MO_GlobalAddress: {
    const GlobalValue *GV = MO.getGlobal();
    GetGlobalValueSymbol(GV)->print(O, MAI);

    // Assemble calls via PLT for externally visible symbols if PIC.
    if (TM.getRelocationModel() == Reloc::PIC_ &&
        !GV->hasHiddenVisibility() && !GV->hasProtectedVisibility() &&
        !GV->hasLocalLinkage())
      O << "@PLT";

    printOffset(MO.getOffset());
    return;
  }
  case MachineOperand::MO_ExternalSymbol: {
    std::string Name(MAI->getGlobalPrefix());
    Name += MO.getSymbolName();
    O << Name;

    if (TM.getRelocationModel() == Reloc::PIC_)
      O << "@PLT";

    return;
  }
  default:
    assert(0 && "Not implemented yet!");
  }
}


void SystemZAsmPrinter::printOperand(const MachineInstr *MI, int OpNum,
                                     const char* Modifier) {
  const MachineOperand &MO = MI->getOperand(OpNum);
  switch (MO.getType()) {
  case MachineOperand::MO_Register: {
    assert (TargetRegisterInfo::isPhysicalRegister(MO.getReg()) &&
            "Virtual registers should be already mapped!");
    unsigned Reg = MO.getReg();
    if (Modifier && strncmp(Modifier, "subreg", 6) == 0) {
      if (strncmp(Modifier + 7, "even", 4) == 0)
        Reg = TRI->getSubReg(Reg, SystemZ::SUBREG_EVEN);
      else if (strncmp(Modifier + 7, "odd", 3) == 0)
        Reg = TRI->getSubReg(Reg, SystemZ::SUBREG_ODD);
      else
        assert(0 && "Invalid subreg modifier");
    }

    O << '%' << getRegisterName(Reg);
    return;
  }
  case MachineOperand::MO_Immediate:
    O << MO.getImm();
    return;
  case MachineOperand::MO_MachineBasicBlock:
    GetMBBSymbol(MO.getMBB()->getNumber())->print(O, MAI);
    return;
  case MachineOperand::MO_JumpTableIndex:
    O << MAI->getPrivateGlobalPrefix() << "JTI" << getFunctionNumber() << '_'
      << MO.getIndex();

    return;
  case MachineOperand::MO_ConstantPoolIndex:
    O << MAI->getPrivateGlobalPrefix() << "CPI" << getFunctionNumber() << '_'
      << MO.getIndex();

    printOffset(MO.getOffset());
    break;
  case MachineOperand::MO_GlobalAddress:
    GetGlobalValueSymbol(MO.getGlobal())->print(O, MAI);
    break;
  case MachineOperand::MO_ExternalSymbol: {
    GetExternalSymbolSymbol(MO.getSymbolName())->print(O, MAI);
    break;
  }
  default:
    assert(0 && "Not implemented yet!");
  }

  switch (MO.getTargetFlags()) {
  default:
    llvm_unreachable("Unknown target flag on GV operand");
  case SystemZII::MO_NO_FLAG:
    break;
  case SystemZII::MO_GOTENT:    O << "@GOTENT";    break;
  case SystemZII::MO_PLT:       O << "@PLT";       break;
  }

  printOffset(MO.getOffset());
}

void SystemZAsmPrinter::printRIAddrOperand(const MachineInstr *MI, int OpNum,
                                           const char* Modifier) {
  const MachineOperand &Base = MI->getOperand(OpNum);

  // Print displacement operand.
  printOperand(MI, OpNum+1);

  // Print base operand (if any)
  if (Base.getReg()) {
    O << '(';
    printOperand(MI, OpNum);
    O << ')';
  }
}

void SystemZAsmPrinter::printRRIAddrOperand(const MachineInstr *MI, int OpNum,
                                            const char* Modifier) {
  const MachineOperand &Base = MI->getOperand(OpNum);
  const MachineOperand &Index = MI->getOperand(OpNum+2);

  // Print displacement operand.
  printOperand(MI, OpNum+1);

  // Print base operand (if any)
  if (Base.getReg()) {
    O << '(';
    printOperand(MI, OpNum);
    if (Index.getReg()) {
      O << ',';
      printOperand(MI, OpNum+2);
    }
    O << ')';
  } else
    assert(!Index.getReg() && "Should allocate base register first!");
}

void SystemZAsmPrinter::PrintGlobalVariable(const GlobalVariable* GVar) {
  const TargetData *TD = TM.getTargetData();

  if (!GVar->hasInitializer())
    return;   // External global require no code

  // Check to see if this is a special global used by LLVM, if so, emit it.
  if (EmitSpecialLLVMGlobal(GVar))
    return;

  MCSymbol *GVarSym = GetGlobalValueSymbol(GVar);
  Constant *C = GVar->getInitializer();
  unsigned Size = TD->getTypeAllocSize(C->getType());
  unsigned Align = std::max(1U, TD->getPreferredAlignmentLog(GVar));

  printVisibility(GVarSym, GVar->getVisibility());

  O << "\t.type\t";
  GVarSym->print(O, MAI);
  O << ",@object\n";

  OutStreamer.SwitchSection(getObjFileLowering().SectionForGlobal(GVar, Mang,
                                                                  TM));

  if (C->isNullValue() && !GVar->hasSection() &&
      !GVar->isThreadLocal() &&
      (GVar->hasLocalLinkage() || GVar->isWeakForLinker())) {

    if (Size == 0) Size = 1;   // .comm Foo, 0 is undefined, avoid it.

    if (GVar->hasLocalLinkage()) {
      O << "\t.local\t";
      GVarSym->print(O, MAI);
      O << '\n';
    }

    O << MAI->getCOMMDirective();
    GVarSym->print(O, MAI);
    O << ',' << Size;
    if (MAI->getCOMMDirectiveTakesAlignment())
      O << ',' << (MAI->getAlignmentIsInBytes() ? (1 << Align) : Align);

    if (VerboseAsm) {
      O << "\t\t" << MAI->getCommentString() << ' ';
      WriteAsOperand(O, GVar, /*PrintType=*/false, GVar->getParent());
    }
    O << '\n';
    return;
  }

  switch (GVar->getLinkage()) {
  case GlobalValue::CommonLinkage:
  case GlobalValue::LinkOnceAnyLinkage:
  case GlobalValue::LinkOnceODRLinkage:
  case GlobalValue::WeakAnyLinkage:
  case GlobalValue::WeakODRLinkage:
    O << "\t.weak\t";
    GVarSym->print(O, MAI);
    O << '\n';
    break;
  case GlobalValue::DLLExportLinkage:
  case GlobalValue::AppendingLinkage:
    // FIXME: appending linkage variables should go into a section of
    // their name or something.  For now, just emit them as external.
  case GlobalValue::ExternalLinkage:
    // If external or appending, declare as a global symbol
    O << "\t.globl ";
    GVarSym->print(O, MAI);
    O << '\n';
    // FALL THROUGH
  case GlobalValue::PrivateLinkage:
  case GlobalValue::LinkerPrivateLinkage:
  case GlobalValue::InternalLinkage:
     break;
  default:
    assert(0 && "Unknown linkage type!");
  }

  // Use 16-bit alignment by default to simplify bunch of stuff
  EmitAlignment(Align, GVar, 1);
  GVarSym->print(O, MAI);
  O << ":";
  if (VerboseAsm) {
    O << "\t\t\t\t" << MAI->getCommentString() << ' ';
    WriteAsOperand(O, GVar, /*PrintType=*/false, GVar->getParent());
  }
  O << '\n';
  if (MAI->hasDotTypeDotSizeDirective()) {
    O << "\t.size\t";
    GVarSym->print(O, MAI);
    O << ", " << Size << '\n';
  }

  EmitGlobalConstant(C);
}

// Force static initialization.
extern "C" void LLVMInitializeSystemZAsmPrinter() {
  RegisterAsmPrinter<SystemZAsmPrinter> X(TheSystemZTarget);
}
