# Building U8.EXE

One command rebuilds Ultima 8's `U8.EXE` from the decompiled source and compares it with the
shipped file, part by part.

```sh
python3 projects/U8/build.py
```

It takes about two minutes. The result is `out/U8.EXE`. It runs: in DOSBox-X it plays the intro, just
like the original.

## Prerequisites

| What | Default location | Option |
|---|---|---|
| Python 3.9 or later | `python3` | |
| DOSBox-X (Homebrew `dosbox-x`) | on `PATH` | `--dosbox` |
| Borland C++ 3.1 (`BIN`, `INCLUDE`) | `/Users/kitrinx/workspace/Ultima/borland/bcpp31/BCPP31` | `--borland` |
| Phar Lap 286 SDK 3.0 (`disk1/BIN/GORUN286.EXE`, `disk1/INC`) | `references/pharlap-286-dos-extender-sdk/extracted` | `--pharlap` |
| Original `U8.EXE`, for the comparison only | `references/ULTIMA8-game/U8.EXE` | `--original`, `--no-compare` |

From Borland the build uses `BCC`, `TASM`, `TLINK`, `TLIB`, `IMPLIB`, the DPMI files `BCC` needs,
and the headers. Everything else it links comes from `lib/`.

## What happens

```
source/**.C, .CPP  --BCC 3.1 -ml + each file's "// flags:" line----->  out/dos/OBJ
source/**.ASM, lib/*.ASM  --TASM 3.1 /ml /m5 + "; flags:"------------->  out/dos/OBJ
objects (all but 10)  --through TLIB libraries and back (63 with /0)-->  out/dos/OBJ
lib/CL.LIB + lib/TMPL.LIB  --TLIB /0, Phar Lap's MKLIB steps----------->  BCL286.LIB
lib/AILXMI.DEF  --IMPLIB---------------------------------------------->  AILXMI.LIB
objects in lib/U8.LNK order + AILXMI, PHAPI, BCL286 + lib/U8.DEF
                --TLINK /c /C /v /s /P=2048, MARKPHAR--------------------->  NE image (GORUN286 stub)
lib/RUN286B.EXE + NE image  --bind------------------------------------->  out/U8.EXE
out/U8.EXE vs original  --compare-------------------------------------->  out/compare.txt
```

- **Compiling.** Each C module is compiled from the original build folder, `source/U8`, under the
  name its `// name:` line gives (`..\ITEM\ITEM.C`). BCC lower-cases a file name typed on its
  command line, `__FILE__` included, so it compiles a one-line stub, `out/dos/STUB/ITEM.C`, that
  includes the source under that name. The stub's name sets the code segment name (`ITEM_TEXT`).
  The flags lines carry the original debug options too: `-y` (line numbers) in the 10 C modules
  that have them in the shipped debug data, `/zd` in the 43 assembly modules that do. `-y` can
  change code, so grading uses the same flags.
- **Libraries.** The shipped debug data names most modules after their object file and lists 63
  not at all: those objects went through TLIB libraries, the 63 through one built with `/0`, which
  drops all comment and line records. `build.py` does the same, in batches of 40 (TLIB runs out of
  memory on more). BCL286 is built with `tlib /0` too, which keeps Phar Lap's modules out of the
  debug data as in the original. SETREGS is assembled with TASM `/o`, whose loader-resolved offsets
  give the two NE relocations the original has.
- **Clock and file times.** The DOS clock is pinned. Source files get the times (to the two-second
  step DOS keeps) recorded in the original debug data (`lib/SRCTIME.TSV`), or 1993-01-01. `MAIN.C` stores `__DATE__`/`__TIME__`, so
  the build sets the clock to 10 Feb 1995 16:02:44 for it, checks the object and retries.
- **Link order.** `lib/U8.LNK` is the TLINK response file. Its object order is the original's,
  taken from the segment layout and the debug data's module list. `/P=2048` packs code segments the
  way the original is packed. `/C` keeps import names case-sensitive (`_DosRealIntr`,
  `_XMI_describe_driver`). Phar Lap's `MARKPHAR` then sets the NE header's target-system byte.
- **Binding.** The SDK has no bindable run-time extender, and its `BIND286` refuses the one in
  U8.EXE (OEM serial WA1 against the SDK's W1). `lib/RUN286B.EXE` is Origin's bound extender (the
  first 225,136 bytes of U8.EXE). `build.py` puts it in front of the linked NE image, as BIND286
  did, and moves the image's file offsets by the same whole number of 512-byte sectors.

## Output

`out/` holds `U8.EXE`, `U8.MAP` (TLINK's detailed map), `compile.log`, `link.log` and
`compare.txt`. `out/dos/` is the DOS working tree (sources, stubs, response files, objects);
`out/tools/` is the toolchain drive.

## How it compares with the original

`compare.txt` splits both files into multiple parts (extender,
NE tables, each segment, each segment's relocations, debug data). 290 of 291 parts are
identical: the extender, NE tables, all 147 code segments, all relocation tables, DGROUP and the
stack segment. The one that differs:

| Part | Why it differs |
|---|---|
| debug data (TDINFO) | The module list matches except `GRAPHICS\CVRESTO.C` and `COLLIDE.C`, which have no code or data in U8.EXE and no source here. Line records follow the decompiled layout, which is not a target. Listed C modules compile through an include stub, so each names the stub (`D:\STUB\ITEM.C`) as its first file. No object is rewritten to hide either. |

For a closer look at one segment, `U8.MAP` lists every module's place, and the split tool's
`segment_map.tsv` and `source_map.tsv` give the original's.
