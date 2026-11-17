export const LANE = { width: 10, length: 28, foulLine: 2, pinLine: 23 };
const clamp = (v, lo, hi) => Math.max(lo, Math.min(hi, v));
export class LaneSimulation {
  constructor() { this.resetRack(); this.ball = null; this.active = false; }
  resetRack() {
    this.pins = [];
    for (let row = 0; row < 4; row++) for (let col = 0; col <= row; col++) {
      this.pins.push({ id: this.pins.length + 1, x: 5 + (col - row / 2) * 1.05, y: 23 + row * .92, vx: 0, vy: 0, r: .36, down: false });
    }
  }
  clearFallen() { this.pins = this.pins.filter(p => !p.down); }
  launch({ position, aim, power, curve }) {
    if (this.active) return false;
    const speed = 11 + clamp(power, 0, 1) * 13;
    const angle = clamp(aim, -1, 1) * .16;
    this.ball = { x: clamp(position, .55, 9.45), y: 2, vx: Math.sin(angle) * speed, vy: Math.cos(angle) * speed, r: .5, gutter: false };
    this.curve = clamp(curve, -1, 1); this.elapsed = 0; this.active = true;
    return true;
  }
  step(dt) {
    if (!this.active) return false;
    this.elapsed += dt;
    const b = this.ball;
    if (b.y < 30) {
      if (!b.gutter) b.vx += this.curve * .85 * dt;
      b.x += b.vx * dt; b.y += b.vy * dt;
      if (b.x < b.r || b.x > 10 - b.r) { b.gutter = true; b.x = clamp(b.x, .1, 9.9); b.vx = 0; }
    }
    for (const p of this.pins) {
      p.x += p.vx * dt; p.y += p.vy * dt;
      const drag = Math.exp(-2.4 * dt); p.vx *= drag; p.vy *= drag;
      if (p.x < .36 || p.x > 9.64) { p.x = clamp(p.x, .36, 9.64); p.vx *= -.3; }
      if (!b.gutter && b.y < 30) this.collide(b, p, true);
    }
    for (let i = 0; i < this.pins.length; i++) for (let j = i + 1; j < this.pins.length; j++) this.collide(this.pins[i], this.pins[j], false);
    const moving = this.pins.some(p => Math.hypot(p.vx, p.vy) > .08);
    if (this.elapsed > 7 || b.y > 29 && !moving) { this.active = false; return true; }
    return false;
  }
  collide(a, b, ball) {
    const dx = b.x - a.x, dy = b.y - a.y, distance = Math.hypot(dx, dy);
    const radius = a.r + b.r;
    if (distance >= radius || distance < .00001) return;
    const nx = dx / distance, ny = dy / distance;
    const relative = (a.vx - b.vx) * nx + (a.vy - b.vy) * ny;
    if (relative > .05) {
      const impulse = relative * (ball ? 1.3 : .8);
      b.vx += nx * impulse; b.vy += ny * impulse;
      if (!ball) { a.vx -= nx * impulse; a.vy -= ny * impulse; }
      else { a.vx -= nx * impulse * .12; a.vy -= ny * impulse * .12; }
      if (impulse > .65) b.down = true;
      if (!ball && impulse > .65) a.down = true;
    }
    b.x += nx * (radius - distance + .001); b.y += ny * (radius - distance + .001);
  }
}
