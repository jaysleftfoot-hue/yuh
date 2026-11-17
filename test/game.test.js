import test from 'node:test';
import assert from 'node:assert/strict';
import { BowlingGame } from '../src/scoring.js';
import { LaneSimulation } from '../src/physics.js';
import { teams } from '../src/teams.js';

function play(rolls) { const g = new BowlingGame(); rolls.forEach(n => g.roll(n)); return g; }
test('perfect game scores 300', () => {
  const g = play(Array(12).fill(10)); assert.equal(g.scores()[9], 300); assert.ok(g.complete);
  assert.throws(() => g.roll(0));
});
test('all spares score 150', () => assert.equal(play(Array(21).fill(5)).scores()[9], 150));
test('gutter game and open frames', () => {
  assert.equal(play(Array(20).fill(0)).scores()[9], 0);
  assert.equal(play(Array(20).fill(4)).scores()[9], 80);
});
test('bonuses wait until the required rolls exist', () => {
  const g = play([10, 10]); assert.equal(g.scores()[0], null);
  g.roll(4); assert.equal(g.scores()[0], 24); assert.equal(g.scores()[1], null);
  g.roll(3); assert.equal(g.scores()[1], 41); assert.equal(g.scores()[2], 48);
});
test('tenth frame racks and non-strike bonus restrictions', () => {
  const g = play([...Array(18).fill(0), 10, 7]);
  assert.equal(g.standing, 3); assert.equal(g.freshRack, false); assert.throws(() => g.roll(4));
  g.roll(3); assert.equal(g.scores()[9], 20); assert.ok(g.complete);
  const spare = play([...Array(18).fill(0), 7, 3]);
  assert.equal(spare.standing, 10); assert.ok(spare.freshRack);
  spare.roll(10); assert.equal(spare.scores()[9], 20);
  const strikes = play([...Array(18).fill(0), 10, 10]);
  assert.equal(strikes.standing, 10); strikes.roll(10); assert.equal(strikes.scores()[9], 30);
});
test('invalid rolls leave game state intact', () => {
  const g = play([7]);
  for (const n of [-1, 4, 2.5, NaN]) assert.throws(() => g.roll(n));
  assert.deepEqual(g.current, [7]); g.roll(3); assert.equal(g.frame, 1);
});
function simulate(shot) {
  const sim = new LaneSimulation(); sim.launch(shot);
  let steps = 0; while (sim.active && steps++ < 1000) sim.step(1 / 120);
  assert.ok(!sim.active, 'shot must settle');
  assert.ok(sim.pins.every(p => Number.isFinite(p.x) && Number.isFinite(p.y)));
  return sim;
}
test('center shot hits pins, gutter shot misses, results repeat', () => {
  const shot = { position: 5, aim: 0, power: .7, curve: 0 };
  const a = simulate(shot), b = simulate(shot);
  assert.ok(a.pins.some(p => p.down)); assert.deepEqual(a.pins, b.pins);
  assert.equal(simulate({ ...shot, position: 1, aim: -1 }).pins.filter(p => p.down).length, 0);
});
test('full simulated game preserves scoring and rack counts', () => {
  const g = new BowlingGame(), s = new LaneSimulation();
  let rolls = 0;
  while (!g.complete && rolls++ < 22) {
    assert.equal(s.pins.length, g.standing);
    s.launch({ position: 5, aim: .08, power: .75, curve: -.1 });
    for (let i = 0; s.active && i < 1000; i++) s.step(1 / 120);
    g.roll(s.pins.filter(p => p.down).length);
    if (g.freshRack) s.resetRack(); else s.clearFallen();
  }
  assert.ok(g.complete); assert.ok(g.scores()[9] >= 0 && g.scores()[9] <= 300);
});
test('all 30 clubs have unique asset slots', () => {
  assert.equal(teams.length, 30); assert.equal(new Set(teams.map(t => t.assetKey)).size, 30);
});
