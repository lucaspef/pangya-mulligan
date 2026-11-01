#!/usr/bin/env python3
"""Recompila cada .gbin 0x71 (ou .wep) de uma pasta de dados e compara com o original pelo parser real.

uso: run_batch.py <dados> <saida> [--wep] [--limit N] [--tol X]
Os dados são os extraídos da sua cópia do jogo; nada disso entra no git.
"""
import collections
import glob
import os
import struct
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
BIN = os.path.join(HERE, '..', 'bin')


def main():
    data, out = sys.argv[1], sys.argv[2]
    use_wep = '--wep' in sys.argv
    tol = sys.argv[sys.argv.index('--tol') + 1] if '--tol' in sys.argv else '0'
    limit = int(sys.argv[sys.argv.index('--limit') + 1]) if '--limit' in sys.argv else 0
    os.makedirs(out, exist_ok=True)
    gbins = {}
    for p in glob.glob(os.path.join(data, '**', '*.gbin'), recursive=True):
        gbins.setdefault(os.path.basename(p).lower(), p)
    sources = [(k, p) for k, p in sorted(gbins.items())]
    if use_wep:
        weps = {os.path.basename(p).lower()[:-4]: p for p in glob.glob(os.path.join(data, '**', '*.wep'), recursive=True)}
        sources = [(k, gbins[k]) for k in sorted(gbins) if k[:-5] in weps]
    tally = collections.Counter()
    total = collections.Counter()
    firsts = {}
    done = skipped = failed = 0
    for name, orig in sources:
        with open(orig, 'rb') as f:
            head = f.read(8)
        if struct.unpack('<I', head[4:])[0] != 0x71:
            skipped += 1
            continue
        if limit and done >= limit:
            break
        src = weps[name[:-5]] if use_wep else orig
        dst = os.path.join(out, name)
        r = subprocess.run([os.path.join(BIN, 'fwcompile.exe'), src, dst, '--root', data], capture_output=True)
        if r.returncode:
            failed += 1
            print('FALHOU', name, r.stdout[-300:])
            continue
        r = subprocess.run([os.path.join(BIN, 'gbincheck.exe'), orig, dst, tol], capture_output=True)
        text = r.stdout.decode('cp949', 'replace').splitlines()
        for i, line in enumerate(text):
            if line.startswith('SUM '):
                _, cat, same, count = line.split()
                tally[cat] += int(same)
                total[cat] += int(count)
            elif line.startswith('DIFF '):
                cat = line.split()[1]
                firsts.setdefault(cat, (name, line, text[i + 1], text[i + 2]))
        done += 1
    print('buracos', done, 'pulados (0x72)', skipped, 'falhas', failed)
    for cat in sorted(total):
        print('%-18s %7d / %7d  (%.1f%%)' % (cat, tally[cat], total[cat], 100.0 * tally[cat] / total[cat]))
    sys.stdout.reconfigure(encoding='utf-8')
    for cat, v in sorted(firsts.items()):
        print('primeira em', cat, ':', v[0], v[1], '\n ', v[2][:170], '\n ', v[3][:170])


if __name__ == '__main__':
    main()
