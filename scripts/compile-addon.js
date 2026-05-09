/**
 * Run cmake-js with PATH cleaned so a conflicting `rc` on PATH (e.g. from
 * another npm package) does not replace the Windows SDK resource compiler.
 *
 * On Windows, plain PowerShell does not have `cl.exe` on PATH. We locate MSVC
 * via vswhere and prepend its Hostx64\\x64 bin directory so CMake can compile.
 */
const { execFileSync, spawnSync } = require('child_process');
const fs = require('fs');
const path = require('path');

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

/** Directory containing cl.exe / nmake.exe for x64 (not on PATH in normal PowerShell). */
function findMsvcHostBinDir() {
  const vswhere = path.join(
    process.env['ProgramFiles(x86)'] || 'C:\\Program Files (x86)',
    'Microsoft Visual Studio',
    'Installer',
    'vswhere.exe',
  );
  if (!fs.existsSync(vswhere)) {
    return null;
  }
  let installPath;
  try {
    installPath = execFileSync(
      vswhere,
      ['-latest', '-products', '*', '-requires', 'Microsoft.VisualStudio.Component.VC.Tools.x86.x64', '-property', 'installationPath'],
      { encoding: 'utf8' },
    ).trim();
  } catch {
    try {
      installPath = execFileSync(vswhere, ['-latest', '-products', '*', '-property', 'installationPath'], {
        encoding: 'utf8',
      }).trim();
    } catch {
      return null;
    }
  }
  if (!installPath) {
    return null;
  }
  const msvcRoot = path.join(installPath, 'VC', 'Tools', 'MSVC');
  if (!fs.existsSync(msvcRoot)) {
    return null;
  }
  let best = '';
  let binDir = null;
  try {
    for (const name of fs.readdirSync(msvcRoot)) {
      if (!/^\d/.test(name)) {
        continue;
      }
      const candidate = path.join(msvcRoot, name, 'bin', 'Hostx64', 'x64');
      if (fs.existsSync(path.join(candidate, 'cl.exe')) && name > best) {
        best = name;
        binDir = candidate;
      }
    }
  } catch {
    return null;
  }
  return binDir;
}

let userArgs = process.argv.slice(2);
if (userArgs.length === 0) {
  userArgs = ['compile'];
}

const env = { ...process.env };
if (env.PATH) {
  const parts = env.PATH.split(path.delimiter).filter((p) => {
    if (!p) {
      return false;
    }
    const q = p.replace(/\\/g, '/').toLowerCase();
    return !q.includes('node_modules') && !q.includes('node_m~');
  });
  env.PATH = parts.join(path.delimiter);
}

if (process.platform === 'win32') {
  const msvcBin = findMsvcHostBinDir();
  if (msvcBin) {
    env.PATH = msvcBin + path.delimiter + env.PATH;
  } else {
    console.warn(
      '[poker-calculations] Could not find MSVC via vswhere. Install "Desktop development with C++" ' +
        '(Build Tools), or open **x64 Native Tools Command Prompt** and run pnpm install again.',
    );
  }
}

let cmakeJsBin;
try {
  cmakeJsBin = require.resolve('cmake-js/bin/cmake-js');
} catch {
  cmakeJsBin = null;
}
if (!cmakeJsBin) {
  console.error('cmake-js is not installed; run npm install or pnpm install from the project root.');
  process.exit(1);
}

const cmd = userArgs[0];
const tail = userArgs.slice(1);
const rc = process.platform === 'win32' ? findWindowsRcCompiler() : null;
const cmakeCd =
  rc && cmd !== 'help' && cmd !== '--help' && cmd !== '-h'
    ? ['--CD', `CMAKE_RC_COMPILER=${rc.replace(/\\/g, '/')}`]
    : [];

/** NMake avoids CMake+VS2026 multi-config issues and matches single-config addon layout. */
const winNMake =
  process.platform === 'win32' && cmd !== 'help' && cmd !== '--help' && cmd !== '-h' ? ['-G', 'NMake Makefiles'] : [];

const r = spawnSync(process.execPath, [cmakeJsBin, cmd, ...cmakeCd, ...winNMake, ...tail], {
  stdio: 'inherit',
  env,
  shell: false,
});
process.exit(r.status === null ? 1 : r.status);
