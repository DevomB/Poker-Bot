'use strict';

const fs = require('fs');
const path = require('path');
const math = require('./poker-math');

function findNativeModule() {
  const root = __dirname;
  const candidates = [
    path.join(root, 'build', 'Release', 'poker_calculations.node'),
    path.join(root, 'build', 'Debug', 'poker_calculations.node'),
    path.join(root, 'build', 'poker_calculations.node'),
    path.join(root, 'build', 'Release', 'poker_overlay.node'),
    path.join(root, 'build', 'Debug', 'poker_overlay.node'),
    path.join(root, 'build', 'poker_overlay.node'),
  ];
  for (const p of candidates) {
    if (fs.existsSync(p)) {
      return require(p);
    }
  }
  try {
    return require('bindings')('poker_calculations');
  } catch (e) {
    return require('bindings')('poker_overlay');
  }
}

const native = findNativeModule();
module.exports = Object.assign({}, native, math);
