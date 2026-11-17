// Rules live outside rendering and physics so skins cannot change scoring.
export class BowlingGame {
  constructor() { this.frames = Array.from({ length: 10 }, () => []); this.frame = 0; this.complete = false; }
  get current() { return this.frames[this.frame]; }
  get freshRack() {
    const r = this.current;
    return r.length === 0 || (this.frame === 9 && (r.length === 1 && r[0] === 10 || r.length === 2 && (r[0] + r[1] === 10 || r[0] === 10 && r[1] === 10)));
  }
  get standing() {
    if (this.complete) return 0;
    const r = this.current;
    if (this.freshRack) return 10;
    return 10 - (this.frame === 9 && r[0] === 10 ? r[1] : r.reduce((a, b) => a + b, 0));
  }
  roll(pins) {
    if (this.complete) throw new Error('Game is complete');
    if (!Number.isInteger(pins) || pins < 0 || pins > this.standing) throw new Error('Invalid pin count');
    const r = this.current;
    r.push(pins);
    if (this.frame < 9) {
      if (pins === 10 || r.length === 2) this.frame++;
    } else if (r.length === 3 || r.length === 2 && r[0] + r[1] < 10) this.complete = true;
  }
  scores() {
    let total = 0;
    return this.frames.map((r, i) => {
      if (i === 9) {
        if (!this.complete) return null;
        total += r.reduce((a, b) => a + b, 0);
        return total;
      }
      const next = this.frames.slice(i + 1).flat();
      if (r[0] === 10) {
        if (next.length < 2) return null;
        total += 10 + next[0] + next[1];
      } else if (r.length === 2) {
        if (r[0] + r[1] === 10) {
          if (!next.length) return null;
          total += 10 + next[0];
        } else total += r[0] + r[1];
      } else return null;
      return total;
    });
  }
}
