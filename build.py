#!/usr/bin/env python3
"""Build U8.EXE from source and compare it with the original, part by part.

    python3 build.py [--borland DIR] [--pharlap DIR] [--original U8.EXE] [--no-compare]

Everything DOS runs inside a headless DOSBox-X:

    source/  --BCC 3.1 / TASM 3.1-->  out/dos/OBJ/*.OBJ
    lib/U8.LNK + lib/U8.DEF  --TLINK 5.1-->  out/dos/U8.EXE (NE image, GORUN286 stub)
    lib/RUN286B.EXE + NE image  --bind-->  out/U8.EXE

C: is the toolchain (out/tools, copied from the Borland and Phar Lap installs); D: is out/dos
(the source tree, include stubs, response files and objects). The DOS clock and file times are
pinned, so a build is repeatable.
"""
import argparse, csv, os, re, shutil, struct, subprocess, sys, time
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
SOURCE = HERE / 'source'
LIB = HERE / 'lib'
BORLAND = Path('/Users/kitrinx/workspace/Ultima/borland/bcpp31/BCPP31')
PHARLAP = ROOT / 'references/pharlap-286-dos-extender-sdk/extracted'
ORIGINAL = ROOT / 'references/ULTIMA8-game/U8.EXE'

BORLAND_BIN = ('BCC.EXE', 'DPMI16BI.OVL', 'DPMILOAD.EXE', 'DPMIMEM.DLL', 'TLINK.EXE', 'TLIB.EXE', 'TASM.EXE',
               'IMPLIB.EXE')
# Folder names too long for DOS.
DOS_DIRS = {'_lib': '_LIB'}
CC_FLAGS = ('-c', '-ml', '-DDOSX286')
ASM_FLAGS = ('/ml', '/m5')

# MAIN.C records __DATE__ and __TIME__: Feb 10 1995 16:02:44. DOSBox-X reads a set time back about
# a second early and the clock runs on while BCC starts, so the stamp is checked and retried.
BUILD_CLOCK = {'MAIN': ('02/10/1995', '16:02:45', b'Feb 10 1995\x0016:02:44')}
# Debug records as the original debug data shows them. Only these C modules were compiled with line
# numbers (`-y` in their flags line); the others reached the link through a TLIB library, which
# names a module after its file, and these through one built with /0, which drops every
# comment and line record, so the linker lists them nowhere in the debug data.
LINE_MODULES = {'DESKIP', 'COLBUF', 'INIT', 'MAIN', 'MOTRAK', 'U8DIR', 'COMBIN', 'ITEM', 'ITEMCACH', 'NPC'}
STRIPPED = {
	'STATREST', 'RANDINT', 'URANDOM', 'REFSTORE', 'GAMETIME', 'WAIT', 'PROCMON', 'HARDWARE', 'HANDLES', 'APPDEBUG',
	'GRAVITY', 'CONTRECT', 'MISSILE', 'MOTCHECK', 'PAL', 'PREAMBLE', 'SAVEGAME', 'SCRATCHM', 'DIST', 'GRAVE',
	'BOOK', 'EGG', 'ITEMFIND', 'AREAFIND', 'SCRIT', 'CONTAIN', 'GLOB', 'ITEMDATA', 'MAPFILE', 'MONEGG',
	'PRELOAD', 'TYPE', 'GUARDIAN', 'CACHENOD', 'CACHE', 'FLEXHAND', 'HANDLER', 'SHAPHAND', 'BALLS', 'SCALE',
	'MUSICFLS', 'AMUSIC', 'PRISM', 'DCMP8', 'VIEWFILE', 'DEBUGUMP', 'CHARGEN', 'PRIORITY', 'SYSTIMER',
	'PROFILER', 'BIOSTIME', 'KERNEL', 'NOTLIST', 'PROCESS', 'MLIST', 'STDINT', 'INTLIST', 'INTERUPT',
	'CALLDTOR', 'GRANTPEA', 'PATHFIND', 'LOITER', 'COMBATBR', 'MONDATA'}
TLINK_FLAGS = '/c /C /v /s /P=2048'
# Files with no recorded time get this one: 1993-01-01 00:00, the pinned DOS clock.
FIXED_TIME = time.mktime((1993, 1, 1, 0, 0, 0, 0, 0, -1))
STALL_SECONDS = 90


# ------------------------------------------------------------------ sources

class Module:
	"""One object file: a source starter (or a lib/ .ASM) and how to build it."""
	def __init__(self, path, name, flags):
		self.path, self.name, self.flags = path, name, flags
		self.stem = name.split('\\')[-1].split('.')[0].upper()
		self.asm = path.suffix.upper() == '.ASM'


def read_modules():
	mods = []
	for p in sorted(SOURCE.rglob('*')) + sorted(LIB.glob('*.ASM')):
		if p.suffix.upper() not in ('.C', '.CPP', '.ASM') or not p.is_file():
			continue
		text = p.read_text(errors='replace')
		name = re.search(r'^\s*(?://|;)\s*name:\s*(\S+)', text, re.M)
		flags = re.search(r'^\s*(?://|;)\s*flags:(.*)$', text, re.M)
		if not name:
			raise SystemExit(f'{p}: no name line')
		asm = p.suffix.upper() == '.ASM'
		mods.append(Module(p, name[1], tuple(flags[1].split()) if flags else ASM_FLAGS if asm else ()))
	return mods


def link_objects():
	"""Object stems named in lib/U8.LNK, in link order."""
	text = (LIB / 'U8.LNK').read_text()
	return re.findall(r'^OBJ\\(\w+)\.OBJ', text, re.M)


def dos_path(rel):
	return Path(*[DOS_DIRS.get(x, x.upper()) for x in rel.parts])


def source_times():
	"""{path under source/: unix time} from the original build's recorded DOS timestamps."""
	out = {}
	with open(LIB / 'SRCTIME.TSV') as f:
		for r in csv.DictReader(f, delimiter='\t'):
			parts = ['U8']
			for x in r['path'].split('\\'):
				parts = parts[:-1] if x == '..' else parts + [x.upper()]
			out['/'.join(parts)] = time.mktime(time.strptime(r['dos_timestamp'], '%Y-%m-%d %H:%M:%S'))
	return out


def place(src, dst, when=FIXED_TIME):
	dst.parent.mkdir(parents=True, exist_ok=True)
	shutil.copyfile(src, dst)
	os.utime(dst, (when, when))


def write(dst, text, when=FIXED_TIME):
	dst.parent.mkdir(parents=True, exist_ok=True)
	dst.write_bytes(text.replace('\n', '\r\n').encode('latin1'))
	os.utime(dst, (when, when))


# ------------------------------------------------------------------ DOSBox-X

def find_dosbox(given):
	for c in ([given] if given else []) + ['dosbox-x', '/opt/homebrew/bin/dosbox-x', '/usr/local/bin/dosbox-x']:
		p = shutil.which(c) if os.sep not in c else (c if Path(c).exists() else None)
		if p:
			return p
	raise SystemExit('dosbox-x not found; pass --dosbox')


def run_dos(args, tools, work, batch, timeout):
	"""Run D:\\RUN.BAT in one DOSBox-X session; kill it if RUN.LOG stops growing."""
	write(work / 'RUN.BAT', '\n'.join(batch) + '\n')
	log = work / 'RUN.LOG'
	log.unlink(missing_ok=True)
	conf = '\n'.join([
		'[sdl]', 'autolock=false', 'waitonerror=false',
		'[dosbox]', 'machine=svga_s3', 'memsize=16', 'quit warning=false',
		'working directory option=noprompt', f'working directory default={work}',
		'[cpu]', 'core=normal', 'cputype=386', 'cycles=max', '[dos]', 'ems=false',
		'[autoexec]', f'mount c "{tools}"', f'mount d "{work}"', 'path=c:\\bc\\bin',
		'date 01/01/1993', 'time 00:00:00', 'd:', 'call run.bat', 'exit']) + '\n'
	(work / 'dosbox.conf').write_text(conf)
	env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy')
	proc = subprocess.Popen([find_dosbox(args.dosbox), '-conf', str(work / 'dosbox.conf'), '-exit', '-silent',
	                         '-nogui', '-nomenu', '-fastlaunch'],
	                        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, env=env)
	start = last = time.monotonic()
	size = -1
	while proc.poll() is None:
		time.sleep(0.25)
		now = time.monotonic()
		s = log.stat().st_size if log.exists() else -1
		if s != size:
			size, last = s, now
		if now - start > timeout or now - last > STALL_SECONDS:
			proc.kill()
			proc.wait()
			raise SystemExit(f'DOSBox-X session {"timed out" if now - start > timeout else "stalled"}; see {log}')
	return log.read_text(errors='replace') if log.exists() else ''


def make_tools(args, tools):
	"""C: drive: BC\\BIN, BC\\INCLUDE and PL\\INC from the installs (once)."""
	if all((tools / 'BC/BIN' / name).exists() for name in BORLAND_BIN):
		return
	for name in BORLAND_BIN:
		place(Path(args.borland) / 'BIN' / name, tools / 'BC/BIN' / name)
	shutil.copytree(Path(args.borland) / 'INCLUDE', tools / 'BC/INCLUDE', dirs_exist_ok=True)
	shutil.copytree(Path(args.pharlap) / 'disk1/INC', tools / 'PL/INC', dirs_exist_ok=True)


# ------------------------------------------------------------------ build steps

def prepare(args, mods, work):
	"""Lay out D:: source tree, include stubs, response files, libraries; return the batch lines."""
	if work.exists():
		shutil.rmtree(work)
	times = source_times()
	headers = []
	for p in sorted(SOURCE.rglob('*')):
		if p.is_file() and p.suffix.upper() in ('.C', '.CPP', '.ASM', '.H', '.INC'):
			rel = dos_path(p.relative_to(SOURCE))
			place(p, work / 'SRC' / rel, times.get(rel.as_posix(), FIXED_TIME))
			# Header folders relative to the build folder, SRC\U8: EVENT, ..\GLIB.
			parts = rel.parts[:-1]
			folder = '\\'.join(parts[1:]) if parts[:1] == ('U8',) else '..\\' + '\\'.join(parts)
			if p.suffix.upper() == '.H' and folder and folder not in headers:
				headers.append(folder)
	for p in LIB.iterdir():
		if p.suffix.upper() in ('.OBJ', '.LIB', '.LBC', '.DEF', '.LNK', '.ASM') and p.name != 'RUN286B.EXE':
			place(p, work / 'LIB' / p.name.upper())
	place(Path(args.pharlap) / 'disk1/BIN/GORUN286.EXE', work / 'LIB/GORUN286.EXE')
	(work / 'OBJ').mkdir()

	include = '-Ic:\\bc\\include;c:\\pl\\inc;' + ';'.join(headers)
	batch = ['echo off',
	         # Phar Lap's MKLIB: CL.LIB with TMPL.LIB's protected-mode modules swapped in.
	         'cd \\LIB', 'copy CL.LIB BCL286.LIB > nul', 'tlib TMPL.LIB @EXTMODS.LBC >> D:\\RUN.LOG',
	         'tlib BCL286.LIB /0 @INSMODS.LBC >> D:\\RUN.LOG',
	         # The Miles XMI driver DLL's entry points, in the order the game's import table lists them.
	         'implib AILXMI.LIB AILXMI.DEF >> D:\\RUN.LOG', 'cd \\']
	for m in mods:
		if m.asm:
			# TASM is run from the module's folder so its .INC files are found.
			folder = 'D:\\LIB' if m.path.parent == LIB else 'D:\\SRC\\' + str(dos_path(m.path.parent.relative_to(SOURCE))).replace('/', '\\')
			opts = ' '.join(m.flags)
			batch += [f'cd {folder}', f'tasm {opts} {m.path.name.upper()}, D:\\OBJ\\{m.stem}.OBJ >> D:\\RUN.LOG']
			continue
		# BCC lower-cases a file name given on its command line, and __FILE__ with it. A stub that
		# includes the source under its recorded spelling keeps "..\ITEM\ITEM.C"; the stub's own
		# name sets the code segment name (ITEM_TEXT).
		ext = Path(m.name).suffix.upper()
		write(work / 'STUB' / f'{m.stem}{ext}', f'#include "{m.name}"\n')
		rsp = ' '.join(CC_FLAGS + m.flags + ('-nD:\\OBJ', include))
		write(work / 'RSP' / f'{m.stem}.RSP', rsp + '\n')
		batch += compile_line(m)
	return batch + ['cd \\'] + library_pass(mods, work)


def library_pass(mods, work):
	"""Batch lines that pass objects through TLIB libraries and back out, as the original build did."""
	lines = ['cd \\OBJ']
	strip = sorted(m.stem for m in mods if m.stem in STRIPPED)
	plain = sorted(m.stem for m in mods if m.stem not in STRIPPED | LINE_MODULES)
	# TLIB runs out of memory on large libraries, so the objects go through in batches.
	batches = [(f'S{k}', ' /0', strip[k:k + 40]) for k in range(0, len(strip), 40)] + \
	          [(f'P{k}', '', plain[k:k + 40]) for k in range(0, len(plain), 40)]
	for lib, opt, stems in batches:
		write(work / 'OBJ' / f'{lib}.ADD', ' &\n'.join(f'+{s}.OBJ' for s in stems) + '\n')
		write(work / 'OBJ' / f'{lib}.GET', ' &\n'.join(f'*{s}.OBJ' for s in stems) + '\n')
		lines += [f'tlib {lib}.LIB{opt} @{lib}.ADD > nul'] + [f'del {s}.OBJ' for s in stems] + \
		         [f'tlib {lib}.LIB @{lib}.GET > nul']
	return lines + ['cd \\']


def compile_line(m, clock=None):
	"""Batch lines that compile C module m from the build folder (its stub and response file exist)."""
	clock = clock or BUILD_CLOCK.get(m.stem)
	clock = ['date ' + clock[0], 'time ' + clock[1]] if clock else []
	ext = Path(m.name).suffix.upper()
	return clock + ['cd \\SRC\\U8', f'echo == {m.stem} >> D:\\RUN.LOG',
	                f'bcc @D:\\RSP\\{m.stem}.RSP D:\\STUB\\{m.stem}{ext} >> D:\\RUN.LOG']


def seconds(hms):
	h, m, s = (int(x) for x in hms.split(':'))
	return h * 3600 + m * 60 + s


def bind(app, extender):
	"""Swap the TLINK stub for Origin's bound 286|DOS-Extender, moving the NE image behind it.

	The NE file offsets (segment sectors, nonresident names) move with it. The image keeps its
	512-byte sector alignment when the move is a whole number of sectors, so the extender is padded
	to make it one (the original needed no padding)."""
	ne = struct.unpack_from('<I', app, 0x3C)[0]
	ext = bytearray(extender)
	ext += bytes((ne - len(ext)) % 512)
	struct.pack_into('<I', ext, 0x3C, len(ext))
	shift = len(ext) - ne
	image = bytearray(app[ne:])
	shift_bits = struct.unpack_from('<H', image, 0x32)[0]
	table = struct.unpack_from('<H', image, 0x22)[0]
	for i in range(struct.unpack_from('<H', image, 0x1C)[0]):
		sector = struct.unpack_from('<H', image, table + i * 8)[0]
		if sector:
			struct.pack_into('<H', image, table + i * 8, sector + (shift >> shift_bits))
	nonres = struct.unpack_from('<I', image, 0x2C)[0]
	struct.pack_into('<I', image, 0x2C, nonres + shift)
	# Everything after the NE header is copied as is, so the shift must be a whole number of sectors.
	assert shift % (1 << shift_bits) == 0
	return bytes(ext + image)


# ------------------------------------------------------------------ compare

def ne_parts(d):
	"""[(part name, bytes)] in file order: extender, NE tables, segments, relocations, debug data."""
	ne = struct.unpack_from('<I', d, 0x3C)[0]
	count = struct.unpack_from('<H', d, ne + 0x1C)[0]
	table = ne + struct.unpack_from('<H', d, ne + 0x22)[0]
	shift = struct.unpack_from('<H', d, ne + 0x32)[0]
	segs = []
	for i in range(count):
		sector, length, flags, _ = struct.unpack_from('<HHHH', d, table + i * 8)
		segs.append((i + 1, sector << shift, (length or 0x10000) if sector else 0, flags))
	first = min(s[1] for s in segs if s[1])
	parts = [('extender', d[:ne]), ('ne_tables', d[ne:first])]
	end = first
	for i, start, length, flags in segs:
		kind = 'data' if flags & 1 else 'code'
		parts.append((f'{kind}/seg_{i:03}', d[start:start + length]))
		end = max(end, start + length)
		if start and flags & 0x100:
			n = struct.unpack_from('<H', d, start + length)[0]
			parts.append((f'relocations/seg_{i:03}', d[start + length:start + length + 2 + n * 8]))
			end = start + length + 2 + n * 8
	debug = d.rfind(b'\xfbR')
	parts.append(('debug', d[debug:] if debug >= end else b''))
	return parts


def compare(built, original, report):
	a, b = dict(ne_parts(built)), ne_parts(original)
	same, lines = [], []
	for name, want in b:
		got = a.get(name)
		if got == want:
			same.append(name)
			continue
		if got is None:
			lines.append(f'{name}: missing')
			continue
		diff = [i for i in range(min(len(got), len(want))) if got[i] != want[i]]
		where = f', first at {diff[0]:#x}' if diff else ''
		# Relocation records hold the same entries, written in another order.
		reordered = name.startswith('relocations/') and sorted(got[k:k + 8] for k in range(2, len(got), 8)) == \
			sorted(want[k:k + 8] for k in range(2, len(want), 8))
		lines.append(f'{name}: {len(got)} bytes vs {len(want)}, {len(diff)} differing{where}'
		             + (' (same records, other order)' if reordered else ''))
	head = [f'identical: {len(same)} of {len(b)} parts', f'whole file identical: {built == original}']
	report.write_text('\n'.join(head + lines + ['', 'identical parts:'] + same) + '\n')
	return head, lines


# ------------------------------------------------------------------ main

def main():
	ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
	ap.add_argument('--borland', default=str(BORLAND), help='Borland C++ 3.1 install (BIN, INCLUDE)')
	ap.add_argument('--pharlap', default=str(PHARLAP), help='Phar Lap 286 SDK 3.0 (disk1/BIN, disk1/INC)')
	ap.add_argument('--dosbox', help='dosbox-x executable')
	ap.add_argument('--out', default=str(HERE / 'out'))
	ap.add_argument('--original', default=str(ORIGINAL), help='U8.EXE to compare with')
	ap.add_argument('--no-compare', action='store_true')
	ap.add_argument('--link-only', action='store_true', help='reuse the objects in out/dos/OBJ')
	args = ap.parse_args()
	out = Path(args.out).resolve()
	tools, work = out / 'tools', out / 'dos'
	make_tools(args, tools)

	mods = read_modules()
	stems = {m.stem: m for m in mods}
	order = link_objects()
	if len(stems) != len(mods) or set(order) != set(stems):
		raise SystemExit(f'lib/U8.LNK and the sources disagree: {sorted(set(order) ^ set(stems))}')

	if not args.link_only:
		print(f'compiling {len(mods)} modules')
		(out / 'compile.log').write_text(run_dos(args, tools, work, prepare(args, mods, work), timeout=1800))
		for stem, (day, clock, stamp) in BUILD_CLOCK.items():
			for _ in range(8):
				obj = (work / 'OBJ' / f'{stem}.OBJ').read_bytes()
				if stamp in obj:
					break
				# Move the clock by however far the recorded time missed.
				at = obj.find(stamp[:12]) + 12
				missed = seconds(obj[at:at + 8].decode('latin1')) - seconds(stamp[12:].decode())
				clock = time.strftime('%H:%M:%S', time.gmtime(seconds(clock) - missed))
				print(f'recompiling {stem} for its build time')
				run_dos(args, tools, work, compile_line(stems[stem], (day, clock)), timeout=300)
			else:
				raise SystemExit(f'{stem}.OBJ: could not reproduce its __DATE__/__TIME__ stamp')
	log = (out / 'compile.log').read_text()
	missing = [m.stem for m in mods if not (work / 'OBJ' / f'{m.stem}.OBJ').exists()]
	errors = [x for x in log.splitlines() if re.match(r'(\*\*)?(Error|Fatal)\b', x) and 'messages' not in x]
	if missing or errors:
		print('\n'.join(errors[:20]))
		raise SystemExit(f'compile failed ({len(missing)} objects missing: {" ".join(missing[:10])}); see {out / "compile.log"}')

	print('linking')
	(work / 'U8.EXE').unlink(missing_ok=True)
	for p in LIB.glob('U8.*'):
		place(p, work / 'LIB' / p.name)
	place(Path(args.pharlap) / 'disk3/BIN/MARKPHAR.EXE', work / 'LIB/MARKPHAR.EXE')
	log = run_dos(args, tools, work, [f'tlink {TLINK_FLAGS} @LIB\\U8.LNK >> D:\\RUN.LOG',
	                                         # Phar Lap's marker: flags the NE header as a 286|DOS-Extender program.
	                                         'LIB\\MARKPHAR U8.EXE >> D:\\RUN.LOG'], timeout=600)
	(out / 'link.log').write_text(log)
	if not (work / 'U8.EXE').exists() or re.search(r'^(Error|Fatal)', log, re.M):
		print(log)
		raise SystemExit(f'link failed; see {out / "link.log"}')
	shutil.copyfile(work / 'U8.MAP', out / 'U8.MAP')

	exe = bind((work / 'U8.EXE').read_bytes(), (LIB / 'RUN286B.EXE').read_bytes())
	(out / 'U8.EXE').write_bytes(exe)
	print(f'wrote {out / "U8.EXE"} ({len(exe)} bytes)')
	if not args.no_compare:
		head, lines = compare(exe, Path(args.original).read_bytes(), out / 'compare.txt')
		print('\n'.join(head + lines[:40]))
		if len(lines) > 40:
			print(f'... {len(lines) - 40} more in {out / "compare.txt"}')
	return 0


if __name__ == '__main__':
	sys.exit(main())
