; RUN: llc -show-mc-encoding -march=arm -mcpu=cortex-a8 -mattr=+neon < %s | FileCheck %s

define <8 x i8> @vnegs8(<8 x i8>* %A) nounwind {
	%tmp1 = load <8 x i8>* %A
; CHECK: vneg.s8	d16, d16                @ encoding: [0xa0,0x03,0xf1,0xf3]
	%tmp2 = sub <8 x i8> zeroinitializer, %tmp1
	ret <8 x i8> %tmp2
}

define <4 x i16> @vnegs16(<4 x i16>* %A) nounwind {
	%tmp1 = load <4 x i16>* %A
; CHECK: vneg.s16	d16, d16        @ encoding: [0xa0,0x03,0xf5,0xf3
	%tmp2 = sub <4 x i16> zeroinitializer, %tmp1
	ret <4 x i16> %tmp2
}

define <2 x i32> @vnegs32(<2 x i32>* %A) nounwind {
	%tmp1 = load <2 x i32>* %A
; CHECK: vneg.s32	d16, d16        @ encoding: [0xa0,0x03,0xf9,0xf3]
	%tmp2 = sub <2 x i32> zeroinitializer, %tmp1
	ret <2 x i32> %tmp2
}

define <2 x float> @vnegf32(<2 x float>* %A) nounwind {
	%tmp1 = load <2 x float>* %A
; CHECK: vneg.f32	d16, d16        @ encoding: [0xa0,0x07,0xf9,0xf3]
	%tmp2 = fsub <2 x float> < float -0.000000e+00, float -0.000000e+00 >, %tmp1
	ret <2 x float> %tmp2
}

define <16 x i8> @vnegQs8(<16 x i8>* %A) nounwind {
	%tmp1 = load <16 x i8>* %A
; CHECK: vneg.s8	q8, q8                  @ encoding: [0xe0,0x03,0xf1,0xf3]
	%tmp2 = sub <16 x i8> zeroinitializer, %tmp1
	ret <16 x i8> %tmp2
}

define <8 x i16> @vnegQs16(<8 x i16>* %A) nounwind {
	%tmp1 = load <8 x i16>* %A
; CHECK: vneg.s16	q8, q8          @ encoding: [0xe0,0x03,0xf5,0xf3]
	%tmp2 = sub <8 x i16> zeroinitializer, %tmp1
	ret <8 x i16> %tmp2
}

define <4 x i32> @vnegQs32(<4 x i32>* %A) nounwind {
	%tmp1 = load <4 x i32>* %A
; CHECK: vneg.s32	q8, q8          @ encoding: [0xe0,0x03,0xf9,0xf3]
	%tmp2 = sub <4 x i32> zeroinitializer, %tmp1
	ret <4 x i32> %tmp2
}

define <4 x float> @vnegQf32(<4 x float>* %A) nounwind {
	%tmp1 = load <4 x float>* %A
; CHECK: vneg.f32	q8, q8          @ encoding: [0xe0,0x07,0xf9,0xf3]
	%tmp2 = fsub <4 x float> < float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00 >, %tmp1
	ret <4 x float> %tmp2
}

define <8 x i8> @vqnegs8(<8 x i8>* %A) nounwind {
	%tmp1 = load <8 x i8>* %A
; CHECK: vqneg.s8	d16, d16        @ encoding: [0xa0,0x07,0xf0,0xf3]
	%tmp2 = call <8 x i8> @llvm.arm.neon.vqneg.v8i8(<8 x i8> %tmp1)
	ret <8 x i8> %tmp2
}

define <4 x i16> @vqnegs16(<4 x i16>* %A) nounwind {
	%tmp1 = load <4 x i16>* %A
; CHECK: vqneg.s16	d16, d16        @ encoding: [0xa0,0x07,0xf4,0xf3]
	%tmp2 = call <4 x i16> @llvm.arm.neon.vqneg.v4i16(<4 x i16> %tmp1)
	ret <4 x i16> %tmp2
}

define <2 x i32> @vqnegs32(<2 x i32>* %A) nounwind {
	%tmp1 = load <2 x i32>* %A
; CHECK: vqneg.s32	d16, d16        @ encoding: [0xa0,0x07,0xf8,0xf3]
	%tmp2 = call <2 x i32> @llvm.arm.neon.vqneg.v2i32(<2 x i32> %tmp1)
	ret <2 x i32> %tmp2
}

define <16 x i8> @vqnegQs8(<16 x i8>* %A) nounwind {
	%tmp1 = load <16 x i8>* %A
; CHECK: vqneg.s8	q8, q8          @ encoding: [0xe0,0x07,0xf0,0xf3]
	%tmp2 = call <16 x i8> @llvm.arm.neon.vqneg.v16i8(<16 x i8> %tmp1)
	ret <16 x i8> %tmp2
}

define <8 x i16> @vqnegQs16(<8 x i16>* %A) nounwind {
	%tmp1 = load <8 x i16>* %A
; CHECK: vqneg.s16	q8, q8          @ encoding: [0xe0,0x07,0xf4,0xf3]
	%tmp2 = call <8 x i16> @llvm.arm.neon.vqneg.v8i16(<8 x i16> %tmp1)
	ret <8 x i16> %tmp2
}

define <4 x i32> @vqnegQs32(<4 x i32>* %A) nounwind {
	%tmp1 = load <4 x i32>* %A
; CHECK: vqneg.s32	q8, q8          @ encoding: [0xe0,0x07,0xf8,0xf3]
	%tmp2 = call <4 x i32> @llvm.arm.neon.vqneg.v4i32(<4 x i32> %tmp1)
	ret <4 x i32> %tmp2
}

declare <8 x i8>  @llvm.arm.neon.vqneg.v8i8(<8 x i8>) nounwind readnone
declare <4 x i16> @llvm.arm.neon.vqneg.v4i16(<4 x i16>) nounwind readnone
declare <2 x i32> @llvm.arm.neon.vqneg.v2i32(<2 x i32>) nounwind readnone

declare <16 x i8> @llvm.arm.neon.vqneg.v16i8(<16 x i8>) nounwind readnone
declare <8 x i16> @llvm.arm.neon.vqneg.v8i16(<8 x i16>) nounwind readnone
declare <4 x i32> @llvm.arm.neon.vqneg.v4i32(<4 x i32>) nounwind readnone
