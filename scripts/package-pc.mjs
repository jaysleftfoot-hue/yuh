import {build} from 'esbuild';
import {readFile,writeFile,mkdir,copyFile} from 'node:fs/promises';
const out='dist/Big-League-Bowling-PC';
await mkdir(out,{recursive:true});
const result=await build({entryPoints:['src/main.js'],bundle:true,format:'iife',minify:true,write:false,legalComments:'inline'});
let html=await readFile('index.html','utf8');
html=html.replace('<link rel="stylesheet" href="src/style.css">',`<style>${await readFile('src/style.css','utf8')}</style>`)
.replace(/<script type="importmap">[\s\S]*?<\/script>/,'')
.replace('<script type="module" src="src/main.js"></script>',()=>`<script>${result.outputFiles[0].text.replace(/<\/script/gi,'<\\/script')}</script>`);
await writeFile(`${out}/Big League Bowling.html`,html);
await writeFile(`${out}/Play.cmd`,'@echo off\r\nstart "" "%~dp0Big League Bowling.html"\r\n');
await copyFile('node_modules/three/LICENSE',`${out}/THREE-LICENSE.txt`);
await writeFile(`${out}/READ ME.txt`, `BIG LEAGUE BOWLING - PC edition\r\n\r\n1. Extract the ZIP (right-click > Extract All).\r\n2. Double-click Big League Bowling.html, or Play.cmd.\r\n3. Play in a recent Microsoft Edge, Chrome, or Firefox browser.\r\n\r\nEverything is included. No internet, installation, Node, or Python required.\r\nWebGL-capable graphics are required. This is a portable browser game, not a native Windows EXE.\r\n\r\nChoose a team, adjust the sliders, and click LET IT RIP.\r\nNew game resets the score and enables team selection.\r\nClose the browser tab to quit.\r\n\r\nThis build has one stylized stadium shared by the teams, simplified arcade physics, and no audio.\r\nThree.js license is included in THREE-LICENSE.txt.\r\n`);
console.log(out);
