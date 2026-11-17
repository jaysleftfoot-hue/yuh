const { app, BrowserWindow, Menu } = require('electron');
const path = require('node:path');
let window;
function createWindow() {
  window = new BrowserWindow({title:'Big League Bowling',width:1440,height:1000,minWidth:800,minHeight:650,backgroundColor:'#101e29',show:false,webPreferences:{nodeIntegration:false,contextIsolation:true,sandbox:true}});
  Menu.setApplicationMenu(null);
  window.webContents.setWindowOpenHandler(()=>({action:'deny'}));
  window.webContents.on('will-navigate',e=>e.preventDefault());
  window.webContents.session.setPermissionRequestHandler((_w,_p,callback)=>callback(false));
  window.webContents.on('before-input-event',(event,input)=>{
    if(input.type!=='keyDown')return;
    if(input.key==='F11'){event.preventDefault();window.setFullScreen(!window.isFullScreen());}
    if(input.key==='Escape'&&window.isFullScreen())window.setFullScreen(false);
  });
  window.loadFile(path.join(__dirname,'game.html'));
  window.once('ready-to-show',()=>window.show());
}
app.whenReady().then(createWindow);
app.on('window-all-closed',()=>app.quit());
