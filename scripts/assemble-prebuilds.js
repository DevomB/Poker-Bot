/**
 * CI / maintainer: place flat `${tuple}.napi.node` files into prebuilds/<tuple>/node.napi.node.
 *
 * Usage: node scripts/assemble-prebuilds.js <directory>
 *
 * Expects filenames like win32-x64.napi.node (one per supported platform).
 */
'use strict';

const fs = require('fs');
const path = require('path');

const srcDir = process.argv[2];
if (!srcDir) {
  console.error('Usage: node scripts/assemble-prebuilds.js <directory-with-*.napi.node>');
  process.exit(1);
}

const abs = path.resolve(srcDir);
if (!fs.existsSync(abs)) {
  console.error(`Directory not found: ${abs}`);
  process.exit(1);
}

const root = path.join(__dirname, '..');
const suffix = '.napi.node';
let count = 0;

for (const name of fs.readdirSync(abs)) {
  if (!name.endsWith(suffix)) {
    continue;
  }
  const tuple = name.slice(0, -suffix.length);
  if (!tuple || !/^[a-z0-9]+(?:-[a-z0-9]+)*$/i.test(tuple)) {
    continue;
  }
  const from = path.join(abs, name);
  const outDir = path.join(root, 'prebuilds', tuple);
  fs.mkdirSync(outDir, { recursive: true });
  const dest = path.join(outDir, 'node.napi.node');
  fs.copyFileSync(from, dest);
  console.log(`${from} -> ${dest}`);
  count++;
}

if (count === 0) {
  console.error(`No *${suffix} files in ${abs}`);
  process.exit(1);
}
