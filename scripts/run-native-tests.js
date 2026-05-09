/**
 * Configure and run C++ GoogleTest suite (no Node addon). Uses a separate
 * build directory so it does not clash with cmake-js's `build/`.
 */
const { spawnSync } = require('child_process');
const fs = require('fs');
const path = require('path');

const root = path.join(__dirname, '..');
const buildDir = path.join(root, 'build_native_tests');

/** Prefer Windows SDK rc.exe so CMake never picks an unrelated `rc` from PATH. */
function findWindowsRcCompiler() {
  const base = path.join(process.env['ProgramFiles(x86)'] || '', 'Windows Kits', '10', 'bin');
  if (!fs.existsSync(base)) {
    return null;
  }
  let bestVer = '';
  let bestPath = null;
  try {
    for (const name of fs.readdirSync(base)) {
      if (!/^\d/.test(name)) {
        continue;
      }
      const candidate = path.join(base, name, 'x64', 'rc.exe');
      if (fs.existsSync(candidate) && name > bestVer) {
        bestVer = name;
        bestPath = candidate;
      }
    }
  } catch {
    return null;
  }
  return bestPath;
}

function sanitizedEnv() {
  const env = { ...process.env };
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
  const rc = findWindowsRcCompiler();
  const rcFlag = rc ? [`-DCMAKE_RC_COMPILER=${rc.replace(/\\/g, '/')}`] : [];
  run('cmake', [...baseConfigure, '-G', 'NMake Makefiles', '-DCMAKE_BUILD_TYPE=Release', ...rcFlag]);
  run('cmake', ['--build', buildDir]);
  run('ctest', ['--test-dir', buildDir, '--output-on-failure', '-C', 'Release']);
} else {
  run('cmake', [...baseConfigure, '-DCMAKE_BUILD_TYPE=Release']);
  run('cmake', ['--build', buildDir, '-j', String(require('os').cpus().length || 4)]);
  run('ctest', ['--test-dir', buildDir, '--output-on-failure']);
}
