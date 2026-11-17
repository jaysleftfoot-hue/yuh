import { BowlingGame } from './scoring.js';
import { LaneSimulation } from './physics.js';
import { PlaceholderRenderer } from './renderer.js';
import { teams } from './teams.js';

const $ = id => document.getElementById(id);
const renderer = new PlaceholderRenderer($('lane'));
let game, sim, state;
const sliders = ['position', 'aim', 'power', 'curve'];
const shot = () => Object.fromEntries(sliders.map(id => [id, Number($(id).value)]));
function controls() {
  for (const id of sliders) {
    const value = Number($(id).value);
    document.querySelector(`output[for="${id}"]`).value = id === 'power' ? `${Math.round(value * 100)}%` : value.toFixed(2);
    $(id).disabled = state !== 'ready';
  }
  $('bowl').disabled = state !== 'ready';
  $('team').disabled = state !== 'ready' || game.frames.some(f => f.length);
}
function scorecard() {
  $('score-head').replaceChildren(); $('rolls').replaceChildren(); $('totals').replaceChildren();
  const scores = game.scores();
  game.frames.forEach((rolls, i) => {
    const head = document.createElement('th'); head.scope = 'col'; head.textContent = i + 1;
    if (i === game.frame) head.setAttribute('aria-current', 'true');
    $('score-head').append(head);
    const marks = rolls.map((v, j) => {
      if (v === 10 && (j === 0 || i === 9 && rolls[j - 1] === 10 || i === 9 && j === 2 && rolls[0] !== 10)) return 'X';
      if (j > 0 && rolls[j - 1] + v === 10 && !(i === 9 && j === 2 && rolls[0] !== 10)) return '/';
      return v === 0 ? '–' : String(v);
    });
    for (const [row, text] of [['rolls', marks.join(' ') || '·'], ['totals', scores[i] ?? '·']]) {
      const cell = document.createElement('td'); cell.textContent = text; $(row).append(cell);
    }
  });
}
function readyText() { return `Frame ${game.frame + 1} · roll ${game.current.length + 1} · ${game.standing} pins standing`; }
function reset() {
  game = new BowlingGame(); sim = new LaneSimulation(); state = 'ready';
  $('result').textContent = ''; $('status').textContent = readyText();
  controls(); scorecard();
}
function bowl() {
  if (state !== 'ready') return;
  sim.launch(shot()); state = 'rolling'; controls(); $('status').textContent = 'Ball in play…';
}
function settle() {
  const knocked = sim.pins.filter(p => p.down).length;
  game.roll(knocked); scorecard();
  $('result').textContent = knocked === 10 ? 'Strike!' : `${knocked} pin${knocked === 1 ? '' : 's'} knocked down.`;
  if (game.complete) {
    state = 'complete'; $('status').textContent = `Game over · ${game.scores()[9]} points`; controls(); return;
  }
  if (game.freshRack) sim.resetRack(); else sim.clearFallen();
  sim.ball = null; state = 'ready'; controls(); $('status').textContent = readyText();
}
for (const team of teams) { const option = document.createElement('option'); option.value = team.id; option.textContent = team.name; $('team').append(option); }
function selectTeam() { const t = teams.find(t => t.id === $('team').value); renderer.setTeam(t.id); $('stadium').textContent = `${t.stadium} · ${t.id}`; }
$('team').addEventListener('change', selectTeam); selectTeam();
$('controls').addEventListener('submit', e => { e.preventDefault(); bowl(); });
$('restart').addEventListener('click', reset);
sliders.forEach(id => $(id).addEventListener('input', controls));
$('lane').tabIndex = 0;
$('lane').addEventListener('keydown', e => {
  if (e.code === 'Space') { e.preventDefault(); bowl(); }
  if (state === 'ready' && ['ArrowLeft', 'ArrowRight'].includes(e.key)) {
    e.preventDefault(); $('aim').value = Number($('aim').value) + (e.key === 'ArrowLeft' ? -.05 : .05); controls();
  }
});
reset();
// Fixed timestep keeps collisions stable across display refresh rates.
let last = performance.now(), accumulator = 0;
function animate(now) {
  accumulator += Math.min((now - last) / 1000, .1); last = now;
  while (accumulator >= 1 / 120) {
    if (state === 'rolling' && sim.step(1 / 120)) { state = 'reaction'; setTimeout(() => { if (state === 'reaction') settle(); }, 1400); }
    accumulator -= 1 / 120;
  }
  renderer.draw(sim, shot(), state === 'ready'); requestAnimationFrame(animate);
}
requestAnimationFrame(animate);
