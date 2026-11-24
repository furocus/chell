/* Documentation only. Run with Node.js; no npm install and no application changes. */
'use strict';
const fs = require('node:fs');
const path = require('node:path');
const os = require('node:os');
const crypto = require('node:crypto');
const {execFileSync} = require('node:child_process');
const {pathToFileURL} = require('node:url');
const root = path.resolve(__dirname, '..');
const versions = {mermaid: '11.12.1', marked: '15.0.12'};
const files = ['README.md', 'BEHAVIOR.md', 'PUBLIC_API.md', 'DEVELOPMENT_GUIDE.md', 'VALIDATION.md'];
const ids = ['overview', 'behavior', 'api', 'guide', 'validation'];
const titles = ['全体像', '動作仕様', '公開API', '担当別ガイド', '検証記録'];
const hash = value => crypto.createHash('sha256').update(value).digest('hex');
const sources = files.map((name, i) => ({name, id: ids[i], title: titles[i], text: fs.readFileSync(path.join(root, name), 'utf8').replace(/\r\n/g, '\n')}));
const sourceHashes = Object.fromEntries(sources.map(doc => [doc.name, hash(doc.text)]));
sourceHashes['tools/build-preview.cjs'] = hash(fs.readFileSync(__filename, 'utf8').replace(/\r\n/g, '\n'));
const manifestPath = path.join(root, 'diagrams', 'manifest.json');

function checkLinksAndTables() {
  let links = 0;
  for (const doc of sources) {
    let columns = null;
    for (const line of doc.text.split('\n')) {
      if (line.startsWith('|')) {
        const width = line.split(/(?<!\\)\|/).length;
        if (columns !== null && columns !== width) throw Error(doc.name + ': table column mismatch: ' + line);
        columns = width;
      } else columns = null;
    }
    for (const match of doc.text.matchAll(/\]\(([^)]+)\)/g)) {
      const href = match[1];
      if (/^(https?:|[A-Z]:|#)/i.test(href)) continue;
      const target = path.resolve(root, href.split('#')[0]);
      if (!fs.existsSync(target) && !href.endsWith('index.html')) throw Error(doc.name + ': broken local link: ' + href);
      links++;
    }
  }
  return links;
}

if (process.argv.includes('--check')) {
  const links = checkLinksAndTables();
  const manifest = JSON.parse(fs.readFileSync(manifestPath, 'utf8'));
  if (JSON.stringify(manifest.sources) !== JSON.stringify(sourceHashes)) throw Error('Source changed; rebuild the preview.');
  if (JSON.stringify(manifest.versions) !== JSON.stringify(versions)) throw Error('Renderer version changed.');
  for (const [name, digest] of Object.entries(manifest.outputs)) {
    if (hash(fs.readFileSync(path.join(root, name))) !== digest) throw Error('Generated output changed: ' + name);
  }
  const expected = sources.reduce((n, doc) => n + [...doc.text.matchAll(/^~~~mermaid$/gm)].length, 0);
  if (manifest.diagrams !== expected) throw Error('Diagram count mismatch.');
  console.log(JSON.stringify({status: 'OK', documents: sources.length, diagrams: expected, localLinks: links, checks: ['source/output SHA-256', 'local links', 'table columns']}, null, 2));
  process.exit(0);
}

const css = `
*{box-sizing:border-box}html{scroll-behavior:smooth}body{margin:0;background:var(--background,#f7f8fa);color:var(--foreground,#202a37);font:16px/1.85 var(--font-sans,'Segoe UI','Yu Gothic',sans-serif)}
header{padding:24px max(20px,calc((100% - 1120px)/2));border-bottom:1px solid var(--border,#dce2ea);background:var(--card,#fff)}header strong{font-size:24px}header p{margin:5px 0;color:var(--muted-foreground,#536174)}
nav{position:sticky;top:0;z-index:5;display:flex;gap:8px;flex-wrap:wrap;padding:12px max(16px,calc((100% - 1120px)/2));background:var(--card,#fff);border-bottom:1px solid var(--border,#dce2ea)}
a{color:var(--primary,#2457a6);text-underline-offset:3px}nav a{padding:6px 13px;text-decoration:none;border-radius:7px;border:1px solid var(--border,#dce2ea)}nav a[aria-current=page]{background:var(--primary,#2457a6);color:var(--primary-foreground,#fff)}
main{max-width:1160px;padding:28px 20px 70px;margin:auto}article{min-width:0;scroll-margin-top:110px}h1{font-size:29px;line-height:1.5;margin:0 0 24px}h2{font-size:23px;border-top:1px solid var(--border,#dce2ea);padding-top:24px;margin-top:38px}h3{font-size:19px}h1,h2,h3{scroll-margin-top:110px}p{margin:16px 0}li{margin:5px 0}
.table-scroll{overflow:auto;margin:20px 0}table{border-collapse:collapse;width:100%;min-width:560px;background:var(--card,#fff);font-size:15px;line-height:1.65}th,td{padding:11px 14px;border:1px solid var(--border,#dce2ea);text-align:left;vertical-align:top}th{background:var(--muted,#edf1f6);font-weight:600}code{font:0.9em/1.6 var(--font-mono,Consolas,monospace);overflow-wrap:anywhere}p code,td code{background:var(--muted,#edf1f6);padding:2px 4px;border-radius:4px}pre{overflow:auto;padding:20px;background:var(--muted,#edf1f6);border-radius:10px;line-height:1.6}pre code{white-space:pre;overflow-wrap:normal}
figure{margin:24px 0;border:1px solid var(--border,#dce2ea);border-radius:12px;overflow:hidden;background:#fff;color:#202a37}figcaption{padding:10px 16px;background:#edf1f6;color:#334155;font-size:14px;display:flex;gap:10px;align-items:center;flex-wrap:wrap}.diagram-scroll{overflow:auto;padding:20px}figure svg{display:block;max-width:none;height:auto;margin:auto}button{font:inherit;cursor:pointer;border:1px solid #b8c7d8;border-radius:5px;background:#fff;color:#2457a6;padding:2px 10px}button:hover{background:#e0eaff}footer{margin-top:30px;color:var(--muted-foreground,#536174);font-size:14px}@media(max-width:600px){header strong{font-size:21px}main{padding:20px 14px 50px}nav{gap:6px;padding:8px}nav a{font-size:14px;padding:4px 8px}h1{font-size:24px}h2{font-size:21px}.diagram-scroll{padding:12px}table{font-size:14px}}
@media print{nav,figcaption button{display:none}article[hidden]{display:block}h2{break-after:avoid}figure{break-inside:avoid}.diagram-scroll{overflow:visible}figure svg{width:100%!important}header{padding:10px}main{padding:0}}
`;

// This script is retained in the offline HTML; rendering libraries are removed.
const viewer = `
function selectDocument(){
 const target=document.getElementById(decodeURIComponent(location.hash.slice(1)));
 const article=target?.closest('article') || document.querySelector('article');
 document.querySelectorAll('article').forEach(el=>el.hidden=el!==article);
 document.querySelectorAll('nav a').forEach(el=>{if(el.hash==='#'+article.id)el.setAttribute('aria-current','page');else el.removeAttribute('aria-current')});
 if(target)target.scrollIntoView();
}
addEventListener('hashchange',selectDocument);selectDocument();
document.querySelectorAll('[data-zoom]').forEach(button=>button.addEventListener('click',()=>{
 const figure=button.closest('figure'),svg=figure.querySelector('svg');
 let zoom=Number(figure.dataset.zoom||1);
 zoom=button.dataset.zoom==='reset'?1:Math.max(0.5,Math.min(2,zoom+Number(button.dataset.zoom)));
 figure.dataset.zoom=zoom;svg.style.width=(Number(svg.dataset.width)*zoom)+'px';
}));
`;
const payload = JSON.stringify({sources, versions, viewer}).replace(/</g, '\\u003c');
const staging = `<!doctype html><html lang="ja"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Chell 開発仕様・公開API</title><style>${css}</style></head><body><header><strong>Chell 開発仕様・公開API</strong><p>レビュー用草案 · 2026-11-17 · 未合意 D01〜D06 / 本体機能は未検証</p></header><nav aria-label="資料">${sources.map(doc=>'<a href="#'+doc.id+'">'+doc.title+'</a>').join('')}</nav><main></main><script type="module">
const {sources,versions,viewer}=${payload};
try {
 const [{default:mermaid},{marked}]=await Promise.all([
  import('https://cdn.jsdelivr.net/npm/mermaid@'+versions.mermaid+'/dist/mermaid.esm.min.mjs'),
  import('https://cdn.jsdelivr.net/npm/marked@'+versions.marked+'/lib/marked.esm.js')]);
 mermaid.initialize({startOnLoad:false,securityLevel:'strict',theme:'neutral',fontFamily:'Segoe UI, Yu Gothic, sans-serif',flowchart:{htmlLabels:false,curve:'linear'},deterministicIds:true});
 const diagrams=[];const main=document.querySelector('main');
 for (const doc of sources) {
  const article=document.createElement('article');article.id=doc.id;article.innerHTML=marked.parse(doc.text);main.append(article);
  let heading=0;article.querySelectorAll('h1,h2,h3').forEach(el=>el.id=doc.id+'-heading-'+(++heading));
  article.querySelectorAll('a').forEach(a=>{const other=sources.find(d=>d.name===a.getAttribute('href'));if(other)a.href='#'+other.id});
  article.querySelectorAll('table').forEach(table=>{const wrap=document.createElement('div');wrap.className='table-scroll';table.replaceWith(wrap);wrap.append(table)});
  await document.fonts.ready;let n=0;
  for(const code of article.querySelectorAll('pre > code.language-mermaid')) {
   const name=doc.id+'-'+(++n),{svg}=await mermaid.render('diagram-'+name,code.textContent);
   const figure=document.createElement('figure');figure.innerHTML='<figcaption>'+doc.title+' / 図 '+n+'<button data-zoom="-0.25" aria-label="図を縮小">−</button><button data-zoom="reset">等倍</button><button data-zoom="0.25" aria-label="図を拡大">＋</button><span>左右にスクロールできます</span></figcaption><div class="diagram-scroll">'+svg+'</div>';
   code.parentElement.replaceWith(figure);const image=figure.querySelector('svg');const box=image.viewBox.baseVal;
   image.dataset.width=box.width;image.style.maxWidth='none';image.style.width=box.width+'px';image.setAttribute('role','img');image.setAttribute('aria-label',doc.title+'の図'+n);
   diagrams.push({name,svg:image.outerHTML,width:box.width,height:box.height});
  }
 }
 document.querySelectorAll('script').forEach(el=>el.remove());
 const script=document.createElement('script');script.textContent=viewer;document.body.append(script);
 window.previewResult={html:'<!doctype html>\\n'+document.documentElement.outerHTML,diagrams};
} catch(error) { window.previewError=String(error.stack||error); }
const done=document.createElement('i');done.id='render-complete';document.body.append(done);
</script></body></html>`;

const orca = process.env.ORCA_CLI_COMMAND || (process.env.ORCA_DEV_REPO_ROOT ? 'orca-dev' : process.platform === 'linux' ? 'orca-ide' : 'orca');
function call(args) {
  const raw = execFileSync(orca, [...args, '--json'], {encoding:'utf8', maxBuffer:16*1024*1024, timeout:60000});
  const response = JSON.parse(raw);
  if (!response.ok) throw Error(JSON.stringify(response.error));
  return response.result;
}
const temp = fs.mkdtempSync(path.join(os.tmpdir(),'chell-spec-render-'));
const stagingPath = path.join(temp,'render.html');
fs.writeFileSync(stagingPath,staging,'utf8');
const tab = call(['tab','create','--url',pathToFileURL(stagingPath).href]);
console.log('Rendering page: '+tab.browserPageId);
// Some Orca hosts cannot return a pending selector wait. Poll read-only state instead.
let ready = false;
for (let attempt=0;attempt<40;attempt++) {
  const state = call(['eval','--page',tab.browserPageId,'--expression',"document.getElementById('render-complete') ? 'ready' : 'pending'"]).result;
  if (state === 'ready') { ready=true; break; }
  Atomics.wait(new Int32Array(new SharedArrayBuffer(4)),0,0,250);
}
if (!ready) throw Error('Rendering timed out; inspect the page '+tab.browserPageId);
const rawRendered = call(['eval','--page',tab.browserPageId,'--expression','({error:window.previewError,result:window.previewResult})']).result;
const rendered = typeof rawRendered === 'string' ? JSON.parse(rawRendered) : rawRendered;
if (rendered.error || !rendered.result) throw Error(rendered.error || 'No rendered result');
const result = rendered.result;
fs.mkdirSync(path.join(root,'diagrams'),{recursive:true});
const outputs = {'index.html':result.html};
for (const diagram of result.diagrams) outputs['diagrams/'+diagram.name+'.svg']=diagram.svg+'\n';
for (const [name, content] of Object.entries(outputs)) fs.writeFileSync(path.join(root,name),content,'utf8');
checkLinksAndTables();
fs.writeFileSync(manifestPath,JSON.stringify({versions,sources:sourceHashes,diagrams:result.diagrams.length,outputs:Object.fromEntries(Object.keys(outputs).map(name=>[name,hash(fs.readFileSync(path.join(root,name)))]))},null,2)+'\n','utf8');
console.log(JSON.stringify({status:'rendered',diagrams:result.diagrams.map(({name,width,height})=>({name,width,height})),output:'docs/spec/index.html'},null,2));
console.log('Preview tab retained for inspection: '+tab.browserPageId);
