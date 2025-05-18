"use strict";

const blackhole = [];

function scene1() {
    const objects = [];
    for (let i = 0; i < 1000; i++) {
        objects.push({
            x: 0.0,
            y: 0.0,
            vx: i / 100.0,
            vy: i / 1000.0,
        });
    }
    const t1 = Date.now();
    for (let n = 0; n < 60 * 60; n++) {
        for (let i = 0; i < 1000; i++) {
            objects[i].x += objects[i].vx;
            objects[i].y += objects[i].vy;
        }
    }
    const t2 = Date.now();
    console.log(`scene1: ${(t2 - t1) / 1000.0}`)
    const result = [];
    for (let i = 0; i < 1000; i++) {
        result.push(`[${i + 1}]{x=${objects[i].x},y=${objects[i].y}}`);
    }
    blackhole.push(result.join(""));
}

for (let i = 0; i < 10; i++) {
    scene1();
}
console.log(blackhole.join("").substring(0, 100));
