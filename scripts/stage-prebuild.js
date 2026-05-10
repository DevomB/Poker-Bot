/**
 * After `cmake-js compile`, copy the built `poker_calculations.node` into
 * `prebuilds/<tuple>/node.napi.node` for node-gyp-build.
 *
 * Usage: node scripts/stage-prebuild.js win32-x64
 */
const fs = require('fs');
const path = require('path');

const tuple = process.argv[2];
if (!tuple) {
  console.error('Usage: node scripts/stage-prebuild.js <platform-arch>');
  process.exit(1);
}

const root = path.join(__dirname, '..');

function walkForPreferred(dir, depth = 0) {
  if (depth > 12 || !fs.existsSync(dir)) {
    return null;
  }
  const preferred = path.join(dir, 'poker_calculations.node');
  if (fs.existsSync(preferred)) {
    return preferred;
  }
  const entries = fs.readdirSync(dir, { withFileTypes: true });
  for (const e of entries) {
    if (!e.isDirectory()) {
      continue;
    }
    const r = walkForPreferred(path.join(dir, e.name), depth + 1);
    if (r) {
      return r;
    }
  }
  return null;
}

const buildDir = path.join(root, 'build');
const built = walkForPreferred(buildDir);

if (!built) {
  console.error(
    `Could not find poker_calculations.node under ${buildDir}. Run cmake-js compile first.`,
  );
  process.exit(1);
}

const outDir = path.join(root, 'prebuilds', tuple);
fs.mkdirSync(outDir, { recursive: true });
const dest = path.join(outDir, 'node.napi.node');
fs.copyFileSync(built, dest);
console.log(`Staged ${built} -> ${dest}`);
