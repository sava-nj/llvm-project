; ModuleID = 'tmp/test1.c'
source_filename = "tmp/test1.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 !dbg !10 {
entry:
  %X = alloca i32, align 4
  %Y = alloca i32, align 4
  %Z = alloca i32, align 4
    #dbg_declare(ptr %X, !14, !DIExpression(), !16)
  store i32 21, ptr %X, align 4, !dbg !16
    #dbg_declare(ptr %Y, !17, !DIExpression(), !18)
  store i32 22, ptr %Y, align 4, !dbg !18
    #dbg_declare(ptr %Z, !19, !DIExpression(), !21)
  store i32 23, ptr %Z, align 4, !dbg !21
  %0 = load i32, ptr %X, align 4, !dbg !22
  store i32 %0, ptr %Z, align 4, !dbg !23
  %1 = load i32, ptr %Y, align 4, !dbg !24
  store i32 %1, ptr %X, align 4, !dbg !25
  ret void, !dbg !26
}

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7, !8}
!llvm.ident = !{!9}

!0 = distinct !DICompileUnit(language: DW_LANG_C11, file: !1, producer: "clang version 20.1.8 (https://github.com/llvm/llvm-project.git 87f0227cb60147a26a1eeb4fb06e3b505e9c7261)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "tmp/test1.c", directory: "/home/njego/projects/llvm-project", checksumkind: CSK_MD5, checksum: "9bf41e93e6b5c43074e583ec2b4d26ca")
!2 = !{i32 7, !"Dwarf Version", i32 5}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 8, !"PIC Level", i32 2}
!6 = !{i32 7, !"PIE Level", i32 2}
!7 = !{i32 7, !"uwtable", i32 2}
!8 = !{i32 7, !"frame-pointer", i32 2}
!9 = !{!"clang version 20.1.8 (https://github.com/llvm/llvm-project.git 87f0227cb60147a26a1eeb4fb06e3b505e9c7261)"}
!10 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 1, type: !11, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !13)
!11 = !DISubroutineType(types: !12)
!12 = !{null}
!13 = !{}
!14 = !DILocalVariable(name: "X", scope: !10, file: !1, line: 2, type: !15)
!15 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!16 = !DILocation(line: 2, column: 9, scope: !10)
!17 = !DILocalVariable(name: "Y", scope: !10, file: !1, line: 3, type: !15)
!18 = !DILocation(line: 3, column: 9, scope: !10)
!19 = !DILocalVariable(name: "Z", scope: !20, file: !1, line: 5, type: !15)
!20 = distinct !DILexicalBlock(scope: !10, file: !1, line: 4, column: 5)
!21 = !DILocation(line: 5, column: 11, scope: !20)
!22 = !DILocation(line: 6, column: 11, scope: !20)
!23 = !DILocation(line: 6, column: 9, scope: !20)
!24 = !DILocation(line: 8, column: 9, scope: !10)
!25 = !DILocation(line: 8, column: 7, scope: !10)
!26 = !DILocation(line: 9, column: 1, scope: !10)
