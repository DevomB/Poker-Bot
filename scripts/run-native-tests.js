/**
 * Configure and run C++ GoogleTest suite (no Node addon). Uses a separate
 * build directory so it does not clash with cmake-js's `build/`.
 */
const { spawnSync } = require('child_process');
const fs = require('fs');
const path = require('path');

const root = path.join(__dirname, '..');
const buildDir = path.join(root, 'build_native_tests');

/** Latest rc.exe + mt.exe under Windows Kits\\10\\bin\\<ver>\\x64 (both Program Files roots). */
function findWindowsKitX64Tools() {
  const roots = [
    path.join(process.env['ProgramFiles(x86)'] || '', 'Windows Kits', '10', 'bin'),
    path.join(process.env.ProgramFiles || '', 'Windows Kits', '10', 'bin'),
  ];
  let bestVer = '';
  let best = null;
  for (const base of roots) {
    if (!fs.existsSync(base)) {
      continue;
    }
    try {
      for (const name of fs.readdirSync(base)) {
        if (!/^\d/.test(name)) {
          continue;
        }
        const rc = path.join(base, name, 'x64', 'rc.exe');
        const mt = path.join(base, name, 'x64', 'mt.exe');
        if (fs.existsSync(rc) && fs.existsSync(mt) && name > bestVer) {
          bestVer = name;
          best = { rc, mt, binDir: path.join(base, name, 'x64') };
        }
      }
    } catch {
      /* ignore */
    }
  }
  return best;
}

function sanitizedEnv() {
  const env = { ...process.env };
  if (process.platform === 'win32') {
    const kit = findWindowsKitX64Tools();
    if (kit) {
      env.PATH = kit.binDir + path.delimiter + (env.PATH || '');
    }
  }
  if (env.PATH) {
    env.PATH = env.PATH.split(path.delimiter)
      .filter((p) => {
        if (!p) {
          return false;
        }
        const q = p.replace(/\\/g, '/').toLowerCase();
        return !q.includes('node_modules') && !q.includes('node_m~');
      })
      .join(path.delimiter);
  }
  return env;
}

function run(cmd, args, extraEnv = {}) {
  const r = spawnSync(cmd, args, {
    cwd: root,
    stdio: 'inherit',
    shell: false,
    env: { ...sanitizedEnv(), ...extraEnv },
  });
  if (r.status !== 0) {
    process.exit(r.status === null ? 1 : r.status);
  }
}

fs.mkdirSync(buildDir, { recursive: true });

const isWin = process.platform === 'win32';
const baseConfigure = ['-S', root, '-B', buildDir, '-DPOKER_BUILD_TESTS=ON'];

if (isWin) {
  const kit = findWindowsKitX64Tools();
  const kitFlags = [];
  if (kit) {
    kitFlags.push(`-DCMAKE_RC_COMPILER=${kit.rc.replace(/\\/g, '/')}`);
    kitFlags.push(`-DCMAKE_MT=${kit.mt.replace(/\\/g, '/')}`);
  }
  run('cmake', [...baseConfigure, '-G', 'NMake Makefiles', '-DCMAKE_BUILD_TYPE=Release', ...kitFlags]);
  run('cmake', ['--build', buildDir]);
  run('ctest', ['--test-dir', buildDir, '--output-on-failure', '-C', 'Release']);
} else {
  run('cmake', [...baseConfigure, '-DCMAKE_BUILD_TYPE=Release']);
  run('cmake', ['--build', buildDir, '-j', String(require('os').cpus().length || 4)]);
  run('ctest', ['--test-dir', buildDir, '--output-on-failure']);
}
