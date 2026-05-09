'use strict';

/**
 * Pure JavaScript helpers for common NLHE chip and odds calculations.
 * For Monte Carlo / hand strength use the native addon methods.
 */

function assertNonNegFinite(name, x) {
  if (typeof x !== 'number' || !Number.isFinite(x) || x < 0) {
    throw new TypeError(`${name} must be a finite non-negative number`);
  }
}

function assertPositiveFinite(name, x) {
  if (typeof x !== 'number' || !Number.isFinite(x) || x <= 0) {
    throw new TypeError(`${name} must be a finite positive number`);
  }
}

/** Stack-to-pot ratio: effective stack divided by pot (common SPR metric). */
function spr(potChips, effectiveStackChips) {
  assertNonNegFinite('potChips', potChips);
  assertNonNegFinite('effectiveStackChips', effectiveStackChips);
  if (potChips === 0) {
    return effectiveStackChips === 0 ? 0 : Infinity;
  }
  return effectiveStackChips / potChips;
}

/** Minimum of stacks (typical effective stack when two players cover each other). */
function effectiveStack(...stacks) {
  const nums = stacks.filter((x) => typeof x === 'number');
  if (nums.length === 0) {
    return 0;
  }
  for (const x of nums) {
    assertNonNegFinite('stack', x);
  }
  return Math.min(...nums);
}

/**
 * Equity needed to break even on a pure chip call (pot before your call, amount to call).
 * Same as price you're getting: toCall / (pot + toCall).
 */
function breakevenCallEquity(potBeforeCall, toCall) {
  assertNonNegFinite('potBeforeCall', potBeforeCall);
  assertNonNegFinite('toCall', toCall);
  const denom = potBeforeCall + toCall;
  if (denom <= 0) {
    return 0;
  }
  if (toCall === 0) {
    return 0;
  }
  return toCall / denom;
}

/**
 * Minimum defense frequency when facing a bet (pot before opponent bets, size of their bet).
 * Fraction of your range you must continue with so their automatic bluffs cannot profit.
 */
function minimumDefenseFrequency(potBeforeOpponentBet, opponentBetSize) {
  assertNonNegFinite('potBeforeOpponentBet', potBeforeOpponentBet);
  assertNonNegFinite('opponentBetSize', opponentBetSize);
  const denom = potBeforeOpponentBet + opponentBetSize;
  if (denom <= 0) {
    return 0;
  }
  return potBeforeOpponentBet / denom;
}

/** Chip stack expressed in big blinds. */
function stackInBigBlinds(stackChips, bigBlind) {
  assertNonNegFinite('stackChips', stackChips);
  assertPositiveFinite('bigBlind', bigBlind);
  return stackChips / bigBlind;
}

/** Pot odds expressed as pot : toCall (e.g. 3.5 means 3.5:1). */
function potOddsRatioDisplay(potBeforeCall, toCall) {
  assertNonNegFinite('potBeforeCall', potBeforeCall);
  assertNonNegFinite('toCall', toCall);
  if (toCall === 0) {
    return Infinity;
  }
  return potBeforeCall / toCall;
}

/** Human-readable "x.xx:1" pot odds string. */
function formatPotOdds(potBeforeCall, toCall, decimals = 2) {
  assertNonNegFinite('potBeforeCall', potBeforeCall);
  assertNonNegFinite('toCall', toCall);
  if (toCall === 0) {
    return '∞:1';
  }
  const r = potBeforeCall / toCall;
  const f = Math.round(r * 10 ** decimals) / 10 ** decimals;
  return `${f}:1`;
}

/**
 * Rule of 4 — approximate chance to hit (~two cards to come), as a fraction 0..1.
 * Pass number of outs (non-double-counted).
 */
function ruleOfFourEquity(outs) {
  assertNonNegFinite('outs', outs);
  const o = Math.min(outs, 48);
  return Math.min(1, (o * 4) / 100);
}

/**
 * Rule of 2 — approximate chance to hit (~one card to come), as a fraction 0..1.
 */
function ruleOfTwoEquity(outs) {
  assertNonNegFinite('outs', outs);
  const o = Math.min(outs, 48);
  return Math.min(1, (o * 2) / 100);
}

/**
 * Average total extra chips you expect to win on later streets (beyond the current
 * pot + call) so that calling breaks even, given your showdown equity after paying `toCall`.
 * Same pot semantics as `expectedValueCall` (pot chips in the middle before you call).
 * Returns Infinity if equity is 0.
 */
function impliedBreakevenFutureWin(potBeforeCall, toCall, equity) {
  assertNonNegFinite('potBeforeCall', potBeforeCall);
  assertNonNegFinite('toCall', toCall);
  if (typeof equity !== 'number' || !Number.isFinite(equity)) {
    throw new TypeError('equity must be a finite number');
  }
  const e = Math.max(0, Math.min(1, equity));
  if (e <= 0) {
    return Infinity;
  }
  const immediateWinTotal = potBeforeCall + toCall;
  return Math.max(0, ((1 - e) * toCall) / e - immediateWinTotal);
}

/**
 * Polarized river betting: approximate bluff-to-value ratio Bet / (Pot + 2×Bet)
 * (Pot before villain's bet).
 */
function bluffToValueRatio(potBeforeBet, betSize) {
  assertNonNegFinite('potBeforeBet', potBeforeBet);
  assertNonNegFinite('betSize', betSize);
  const denom = potBeforeBet + 2 * betSize;
  if (denom <= 0) {
    return 0;
  }
  return betSize / denom;
}

module.exports = {
  spr,
  effectiveStack,
  breakevenCallEquity,
  minimumDefenseFrequency,
  stackInBigBlinds,
  potOddsRatioDisplay,
  formatPotOdds,
  ruleOfFourEquity,
  ruleOfTwoEquity,
  impliedBreakevenFutureWin,
  bluffToValueRatio,
};
