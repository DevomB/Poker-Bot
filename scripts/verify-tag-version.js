/**
 * Release gate: Git tag vX.Y.Z must match package.json "version" X.Y.Z.
 * In GitHub Actions, GITHUB_REF_NAME is set to the tag name (e.g. v1.2.3).
 */
'use strict';

const fs = require('fs');
const path = require('path');

const tag = process.env.GITHUB_REF_NAME || '';
if (!tag.startsWith('v')) {
  console.log('verify-tag-version: skipped (set GITHUB_REF_NAME=vX.Y.Z to enforce)');
  process.exit(0);
}

const pkg = JSON.parse(
  fs.readFileSync(path.join(__dirname, '..', 'package.json'), 'utf8'),
);
const expected = tag.slice(1);
if (expected !== pkg.version) {
  console.error(
    `Tag "${tag}" requires package.json "version" "${expected}" but found "${pkg.version}".`,
  );
  console.error('Fix: npm version ' + expected + ' --git-tag-version false && git add package.json');
  process.exit(1);
}
console.log(`verify-tag-version: OK (${pkg.version})`);
