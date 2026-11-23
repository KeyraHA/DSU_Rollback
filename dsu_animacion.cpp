#define DSU_ANIMACION
#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #define NOMINMAX
  #include <winsock2.h>
  #include <ws2tcpip.h>
  typedef SOCKET sock_t;
  #define CLOSESOCK closesocket
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <unistd.h>
  typedef int sock_t;
  #define CLOSESOCK close
#endif
#include "DSU_rollback.cpp"
#include <sstream>
#include <map>
#include <memory>
#include <tuple>
#include <array>

static string S(int x) { return to_string(x); }
static string esc(const string& s) {
    string r;
    for (char c : s) { if (c == '"' || c == '\\') r += '\\'; r += c; }
    return r;
}
static string lst(const vector<int>& v) {
    string s = "[";
    for (size_t i = 0; i < v.size(); i++) { if (i) s += ","; s += S(v[i]); }
    return s + "]";
}
static string csj(const vector<string>& calls, int line) {   // panel del find recursivo
    string s = "{\"c\":[";
    for (size_t i = 0; i < calls.size(); i++) { if (i) s += ","; s += "\"" + calls[i] + "\""; }
    return s + "],\"l\":" + S(line) + "}";
}

struct Animator {
    int n;
    DSU dsu;                    //la estructura real
    vector<string> frames;
    bool demo = false;          // true: pausas largas para narrar; false: manual

    Animator(int n_) : n(n_), dsu(n_) {}
    void intro(bool d) {
        if (d) emit("Empezamos con " + S(n) + " nodos. Cada uno es su propia raíz: hay " + S(n) +
                    " componentes y <b>history</b> está vacía.", {}, {}, {}, 0, 9000);
        else emit("Elige dos nodos y pulsa <b>unite</b>.", {}, {}, {}, 0, 0);
    }
    void msg(const string& m) { emit(m, {}, {}, {}, 0, 0); }
    string take() {
        string s = "[";
        for (size_t i = 0; i < frames.size(); i++) s += (i ? ",\n" : "") + frames[i];
        frames.clear();
        return s + "]";
    }
    // d = milisegundos que se mantiene el cuadro en pantalla
    void emit(const string& m, const vector<int>& pa, const vector<int>& h, const vector<int>& u,
              int eh, int d, const string& cs = "null") {
        string hs = "[";
        auto H = dsu.get_history();
        for (size_t i = 0; i < H.size(); i++) {
            if (i) hs += ",";
            hs += "[" + S(H[i].first) + "," + S(H[i].second) + "]";
        }
        hs += "]";
        frames.push_back("{\"m\":\"" + esc(m) + "\",\"p\":" + lst(dsu.get_parents()) +
            ",\"s\":" + lst(dsu.get_sizes()) + ",\"hist\":" + hs + ",\"c\":" + S(dsu.count_components()) +
            ",\"pa\":" + lst(pa) + ",\"h\":" + lst(h) + ",\"u\":" + lst(u) + ",\"eh\":" + S(eh) +
            ",\"d\":" + S(d) + ",\"cs\":" + cs + "}");
    }
    bool ok(int a) const { return a >= 1 && a <= n; }

    // find recursivo: baja por las llamadas y luego retorna
    int climb(int i) {
        auto p = dsu.path(i);
        int r = p.back();
        vector<string> calls;
        for (size_t k = 0; k < p.size(); k++) {
            vector<int> pa(p.begin(), p.begin() + k);
            calls.push_back("find(" + S(p[k]) + ")");
            bool root = (k == p.size() - 1);
            int pp = dsu.get_parents()[p[k]];
            string m = root ? "<b>find(" + S(p[k]) + ")</b>: parents[" + S(p[k]) + "] = " + S(p[k]) + " → caso base, retorna " + S(p[k])
                            : "<b>find(" + S(p[k]) + ")</b>: parents[" + S(p[k]) + "] = " + S(pp) + " ≠ " + S(p[k]) + " → llama a <b>find(" + S(pp) + ")</b>";
            emit(m, pa, {p[k]}, {}, 0, 750, csj(calls, root ? 1 : 2));
        }
        vector<int> pa(p.begin(), p.end() - 1);
        for (int k = (int)p.size() - 1; k >= 0; k--) {
            emit("<b>find(" + S(p[k]) + ")</b> retorna <b>" + S(r) + "</b>", pa, {r}, {}, 0, 450,
                 csj(calls, k == (int)p.size() - 1 ? 1 : 2));
            calls.pop_back();
        }
        return r;
    }
    void opFind(int a, int after) {
        int r = climb(a);
        auto p = dsu.path(a);
        vector<int> pa(p.begin(), p.end() - 1);
        string camino;
        for (size_t k = 0; k < p.size(); k++) camino += S(p[k]) + (k + 1 < p.size() ? " → " : "");
        string m = "<b>find(" + S(a) + ") = " + S(r) + "</b>: subió " + camino + ". find solo lee, no cambia nada.";
        emit(m, pa, {r}, {}, 0, after, csj({}, -1));
        if (!demo) emit(m, {}, {}, {}, 0, 0, csj({}, -1));
    }
    void opSame(int a, int b, int after) {
        int x = climb(a), y = climb(b);
        bool r = dsu.Same_Set(a, b);
        string m = "<b>Same_Set(" + S(a) + ", " + S(b) + ")</b>: find(" + S(a) + ") = " + S(x) + ", find(" + S(b) +
                   ") = " + S(y) + " → <b>" + (r ? "true" : "false") + "</b>";
        emit(m, {}, r ? vector<int>{x, y} : vector<int>{x}, r ? vector<int>{} : vector<int>{y}, 0, after, csj({}, -1));
        if (!demo) emit(m, {}, {}, {}, 0, 0, csj({}, -1));
    }
    void opUnite(int a, int b, int after) {
        auto pa = dsu.path(a), pb = dsu.path(b);
        int x = pa.back(), y = pb.back();
        vector<int> all = pa; all.insert(all.end(), pb.begin(), pb.end());
        emit("<b>find(" + S(a) + ") = " + S(x) + "</b>, <b>find(" + S(b) + ") = " + S(y) + "</b>", all, {x, y}, {}, 0, 900);
        if (x == y) {
            string m = "Misma raíz (" + S(x) + "): <b>unite</b> devuelve false y no guarda nada en history.";
            emit(m, all, {x, y}, {}, 0, 600);
            emit(m, {}, {}, {}, 0, demo ? after + 500 : after);
            return;
        }
        auto antes = dsu.get_sizes();
        dsu.unite(a, b);                    
        auto [u, v] = dsu.top();
        string m = "tam[" + S(u) + "]=" + S(antes[u]) + " ≥ tam[" + S(v) + "]=" + S(antes[v]) + " → <b>parents[" + S(v) +
                   "] = " + S(u) + "</b>, <b>sizes[" + S(u) + "] += " + S(antes[v]) + "</b>, <b>history.push(" + S(u) + ", " + S(v) + ")</b>";
        emit(m, {}, {v}, {}, 0, 820);
        emit(m, {}, {}, {}, 0, after);
    }
    void opRollback(int after) {
        if (dsu.time() == 0) {
            emit("history está vacía: no hay nada que deshacer.", {}, {}, {}, 0, after);
            return;
        }
        auto [u, v] = dsu.top();
        int sv = dsu.get_sizes()[v];
        emit("Tope de history: (" + S(u) + ", " + S(v) + ") → <b>parents[" + S(v) + "] = " + S(v) + "</b>, <b>sizes[" + S(u) +
             "] -= " + S(sv) + "</b>, <b>components++</b>", {}, {u}, {v}, v, 900);
        dsu.rollback();                        // <- rollback real
        string m = "Tope de history: (" + S(u) + ", " + S(v) + ") → <b>parents[" + S(v) + "] = " + S(v) + "</b>, <b>sizes[" + S(u) +
                   "] -= " + S(sv) + "</b>, <b>components++</b>";
        emit(m, {}, {u}, {v}, 0, 520);
        emit(m, {}, {}, {}, 0, after);
    }
    void run(istream& in) {
        demo = true;
        vector<tuple<string, int, int>> ops;
        string cmd;
        while (in >> cmd) {
            int a = 0, b = 0;
            if (cmd == "unite" || cmd == "same") in >> a >> b;
            else if (cmd == "find") in >> a;
            else if (cmd != "rollback") continue;
            if ((cmd != "rollback") && !(ok(a) && (cmd == "find" || ok(b)))) continue;
            ops.emplace_back(cmd, a, b);
        }
        for (size_t i = 0; i < ops.size(); i++) {
            auto [c, a, b] = ops[i];
            int extra = (i + 1 == ops.size()) ? 500 : 0;
            if (c == "unite") opUnite(a, b, 3000 + extra);
            else if (c == "find") opFind(a, 4500 + extra);
            else if (c == "same") opSame(a, b, 4000 + extra);
            else opRollback(3500 + extra);
        }
        demo = false;
    }
};

// Página (HTML/JS): solo dibuja lo que devuelve el programa C++.

static const char* HEAD = R"HTML(<!doctype html>
<html lang="es"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover">
<title>DSU con rollback</title>
<link href="https://fonts.googleapis.com/css2?family=Bricolage+Grotesque:wght@500;700&family=JetBrains+Mono:wght@400;600&display=swap" rel="stylesheet">
<style>
:root{--bg:#eef1ec;--panel:#fbfcfa;--ink:#1d2a2a;--mut:#5d6b69;--line:#c9d1cc;--root:#0f766e;--hot:#d9480f;--undo:#6d28d9;--ok:#15803d;--bad:#b42318;
box-sizing:border-box;padding-top:env(safe-area-inset-top,0px);padding-bottom:env(safe-area-inset-bottom,0px)}
@media (prefers-color-scheme:dark){:root:not([data-theme="light"]){--bg:#101716;--panel:#18211f;--ink:#e6eeeb;--mut:#93a39f;--line:#2e3b38;--root:#2dd4bf;--hot:#fb923c;--undo:#a78bfa;--ok:#4ade80;--bad:#f87171}}
:root[data-theme="dark"]{--bg:#101716;--panel:#18211f;--ink:#e6eeeb;--mut:#93a39f;--line:#2e3b38;--root:#2dd4bf;--hot:#fb923c;--undo:#a78bfa;--ok:#4ade80;--bad:#f87171}
html{scroll-padding-top:env(safe-area-inset-top,0px)}
*{box-sizing:border-box}[hidden]{display:none!important}
body{margin:0;background:var(--bg);color:var(--ink);font:15px/1.5 "Bricolage Grotesque",system-ui,sans-serif}
main{max-width:1040px;margin:0 auto;padding:20px 16px 40px}
h1{font-size:26px;margin:0 0 4px;letter-spacing:-.01em}
p.sub{margin:0 0 16px;color:var(--mut)}
.mono,input,.cell{font-family:"JetBrains Mono",ui-monospace,Menlo,monospace}
.panel{background:var(--panel);border:1px solid var(--line);border-radius:10px;padding:12px 14px;margin-bottom:12px}
.row{display:flex;flex-wrap:wrap;gap:8px;align-items:center}
label{color:var(--mut);font-size:13px}
input{background:var(--bg);color:var(--ink);border:1px solid var(--line);border-radius:6px;padding:6px 8px;font-size:14px}
input[type=number]{width:64px}
button{font:inherit;font-weight:700;border:1px solid var(--ink);background:var(--ink);color:var(--bg);border-radius:6px;padding:6px 12px;cursor:pointer}
button.alt{background:transparent;color:var(--ink)}
button.undo{background:var(--undo);border-color:var(--undo);color:#fff}
button:disabled{opacity:.4;cursor:wait}
button:focus-visible,input:focus-visible{outline:3px solid var(--hot);outline-offset:2px}
.stage{padding:0;overflow-x:auto}
svg{display:block;width:100%;min-width:520px;height:auto;max-height:420px}
.edge{stroke:var(--mut);stroke-width:2}.edge.undo{stroke:var(--undo);stroke-width:4;stroke-dasharray:6 4}
.node circle{fill:var(--panel);stroke:var(--mut);stroke-width:2}
.node.isroot circle{stroke:var(--root);stroke-width:3.5}
.node.hot circle{fill:var(--hot);stroke:var(--hot)}.node.path circle{stroke:var(--hot);stroke-width:3.5}
.node.undo circle{fill:var(--undo);stroke:var(--undo)}
.node text{text-anchor:middle;dominant-baseline:central;font:600 14px "JetBrains Mono",monospace;fill:var(--ink)}
.node.hot text,.node.undo text{fill:#fff}
.tam{font:12px "JetBrains Mono",monospace;fill:var(--root);text-anchor:middle}
.grid{display:grid;grid-template-columns:1fr 1fr;gap:12px}
@media(max-width:700px){.grid{grid-template-columns:1fr}}
.arr{display:flex;flex-wrap:wrap;gap:4px;margin-bottom:6px}
.cell{border:1px solid var(--line);border-radius:5px;padding:2px 6px;text-align:center;font-size:13px;min-width:38px}
.cell small{display:block;color:var(--mut);font-size:10px}
.cell.ch{border-color:var(--hot);color:var(--hot)}
.stack{display:flex;flex-direction:column;gap:3px;min-height:28px}
.stack .cell{text-align:left}.stack .cell:first-child{border-color:var(--undo);color:var(--undo);font-weight:600}
#log{min-height:48px;font-size:14px}
#log b{font-family:"JetBrains Mono",monospace;font-weight:600}
h2{font-size:15px;margin:0 0 8px}.ln{white-space:pre;padding:0 6px}.ln.on{background:var(--hot);color:#fff;border-radius:3px}
.big{font-size:22px;font-weight:700}
</style></head>
<body><main>
<h1>DSU con rollback</h1>
<p class="sub">Un DSU junta elementos en grupos y responde si dos están en el mismo grupo. Con <b>rollback</b> además puede deshacer la última unión.</p>

<div class="panel">
 <div class="row">
  <label>n <input id="n" type="number" min="2" max="14" value="8"></label>
  <button class="alt" id="reset">Reiniciar</button>
  <span style="flex:1"></span>
  <label>u <input id="u" type="number" min="1" value="1"></label>
  <label>v <input id="v" type="number" min="1" value="2"></label>
  <button id="un">unite(u, v)</button>
  <button class="undo" id="rb">rollback()</button>
 </div>
 <div class="row" style="margin-top:10px">
  <span style="color:var(--mut);font-size:13px">Consultas (solo leen, no cambian nada):</span>
  <button class="alt" id="fd">find(u)</button>
  <button class="alt" id="ss">Same_Set(u, v)</button>
 </div>
 <div class="row" style="margin-top:10px">
  <button id="auto">▶ Reproducir demo</button>
  <button class="alt" id="e1">Caso borde: rollback vacío</button>
  <button class="alt" id="e2">Caso borde: mismo conjunto</button>
 </div>
</div>

<div class="panel stage"><svg id="svg" viewBox="0 0 800 300" role="img" aria-label="Bosque del DSU"></svg></div>
<div class="panel" id="log" aria-live="polite"></div>
<div class="panel" id="cs" hidden><h2>find recursivo: código y pila de llamadas</h2><div class="row" style="align-items:flex-start;gap:24px"><pre class="mono" id="code" style="margin:0;font-size:13px"></pre><div class="stack" id="calls" style="min-width:140px"></div></div></div>

<div class="grid">
 <div class="panel">
  <h2>Estado</h2>
  <div class="arr" id="parr"></div><div class="arr" id="sarr"></div>
  <div>componentes: <span class="big mono" id="comp"></span> &nbsp; time(): <span class="big mono" id="time"></span></div>
 </div>
 <div class="panel">
  <h2>history (tope arriba)</h2>
  <div class="stack" id="stack"></div>
 </div>
</div>

<div class="panel"><h2>Complejidad (unión por tamaño, sin compresión)</h2>
 <div class="mono" style="font-size:13px;line-height:1.8">find(u) &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;O(log n) &nbsp;— la altura del árbol es ≤ log₂ n<br>Same_Set &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;O(log n) &nbsp;— dos find<br>unite &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;O(log n) &nbsp;— dos find + O(1) de enlazar y apilar<br>rollback &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;O(1) &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;— restaura un solo puntero y un tamaño<br>memoria &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;O(n + uniones guardadas)</div></div>
</main>
<script>
)HTML";

static const char* TAIL = R"HTML(
const $=id=>document.getElementById(id),sleep=ms=>new Promise(r=>setTimeout(r,ms));
const U=64;let pos={},W=800,H=300,busy=false;
function layout(p){const n=p.length-1,ch={},t={};for(let v=1;v<=n;v++)ch[v]=[];for(let v=1;v<=n;v++)if(p[v]!==v)ch[p[v]].push(v);
 const w=v=>ch[v].length?ch[v].reduce((a,c)=>a+w(c),0):1;let cx=0;
 const put=(v,d,x0)=>{const ww=w(v);t[v]={x:(x0+ww/2)*U,y:d*78+40};let o=x0;for(const c of ch[v]){put(c,d+1,o);o+=w(c)}};
 for(let v=1;v<=n;v++)if(p[v]===v){put(v,0,cx);cx+=w(v)+.4}
 W=Math.max(520,cx*U);H=Math.max(...Object.values(t).map(q=>q.y))+50;return t}
function draw(f){const p=f.p,n=p.length-1,hl={};f.pa.forEach(v=>hl[v]="path");f.h.forEach(v=>hl[v]="hot");f.u.forEach(v=>hl[v]="undo");let s="";
 for(let v=1;v<=n;v++)if(p[v]!==v)s+=`<line class="edge${f.eh===v?" undo":""}" x1="${pos[v].x}" y1="${pos[v].y}" x2="${pos[p[v]].x}" y2="${pos[p[v]].y}"/>`;
 for(let v=1;v<=n;v++){const r=p[v]===v;
  s+=`<g class="node${r?" isroot":""} ${hl[v]||""}"><circle cx="${pos[v].x}" cy="${pos[v].y}" r="20"/><text x="${pos[v].x}" y="${pos[v].y}">${v}</text></g>`;
  if(r)s+=`<text class="tam" x="${pos[v].x}" y="${pos[v].y-28}">tam ${f.s[v]}</text>`}
 $("svg").setAttribute("viewBox",`0 0 ${W+U/2} ${H}`);$("svg").innerHTML=s;
 $("parr").innerHTML=Array.from({length:n},(_,i)=>`<div class="cell ${hl[i+1]?"ch":""}"><small>par[${i+1}]</small>${p[i+1]}</div>`).join("");
 $("sarr").innerHTML=Array.from({length:n},(_,i)=>`<div class="cell"><small>sz[${i+1}]</small>${f.s[i+1]}</div>`).join("");
 $("comp").textContent=f.c;$("time").textContent=f.hist.length;
 $("stack").innerHTML=f.hist.length?[...f.hist].reverse().map(([a,b])=>`<div class="cell">(${a}, ${b})</div>`).join(""):'<span style="color:var(--mut)">vacía</span>';
 $("log").innerHTML=f.m;
 if(f.cs){$("cs").hidden=false;
  $("code").innerHTML=["int find(int i) {","    if (parents[i] == i) return i;","    return find(parents[i]);","}"].map((t,k)=>`<div class="ln${k===f.cs.l?" on":""}">${t}</div>`).join("");
  $("calls").innerHTML=[...f.cs.c].reverse().map(c=>`<div class="cell">${c}</div>`).join("")||'<span style="color:var(--mut)">vacía</span>'}}
function show(f){const t=layout(f.p),a=JSON.parse(JSON.stringify(Object.keys(pos).length?pos:t));
 const moved=Object.keys(t).some(v=>!a[v]||Math.abs(a[v].x-t[v].x)>.5||Math.abs(a[v].y-t[v].y)>.5),t0=performance.now(),ms=moved?520:0;
 (function g(now){let q=ms?Math.min(1,(now-t0)/ms):1;q=q*q*(3-2*q);
  for(const v in t){const b=a[v]||t[v];pos[v]={x:b.x+(t[v].x-b.x)*q,y:b.y+(t[v].y-b.y)*q}}
  draw(f);if(q<1)requestAnimationFrame(g)})(t0)}
function lock(b){busy=b;document.querySelectorAll("button").forEach(x=>x.disabled=b)}
// Pide el paso al programa C++ y reproduce los cuadros que devuelve.
async function go(url,after){if(busy)return;lock(true);
 try{const fr=await (await fetch(url)).json();for(const f of fr){show(f);await sleep(f.d)}if(after)after()}
 catch(e){$("log").textContent="No se pudo hablar con el programa en C++. ¿Sigue ejecutándose ./dsu_animacion?"}
 lock(false)}
const N=()=>Math.max(2,Math.min(14,+$("n").value||8));
const q=()=>`a=${+$("u").value}&b=${+$("v").value}`;
$("reset").onclick=()=>go(`/api/reset?n=${N()}`);
$("n").onchange=()=>$("reset").click();
$("un").onclick=()=>go(`/api/op?cmd=unite&${q()}`);
$("rb").onclick=()=>go(`/api/op?cmd=rollback`);
$("fd").onclick=()=>go(`/api/op?cmd=find&${q()}`);
$("ss").onclick=()=>go(`/api/op?cmd=same&${q()}`);
$("e1").onclick=()=>go(`/api/edge1?n=${N()}`);
$("e2").onclick=()=>go(`/api/edge2?n=${N()}`);
$("auto").onclick=()=>go(`/api/demo?n=${N()}`,()=>window.scrollTo({top:document.body.scrollHeight,behavior:"smooth"}));
go(`/api/reset?n=${N()}`);
</script></body></html>
)HTML";

// Servidor HTTP mínimo (un solo hilo, una petición a la vez).

static unique_ptr<Animator> A;

static string handle(const string& path, map<string, string>& q, string& type) {
    if (path == "/") { type = "text/html; charset=utf-8"; return string(HEAD) + TAIL; }
    type = "application/json";
    auto num = [&](const char* k, int d) {
        auto it = q.find(k);
        if (it == q.end()) return d;
        try { return stoi(it->second); } catch (...) { return d; }
    };
    auto nuevo = [&]() { A = make_unique<Animator>(max(2, min(14, num("n", 8)))); };
    if (path == "/api/reset") { nuevo(); A->intro(false); }
    else if (path == "/api/demo") {
        nuevo(); A->intro(true);
        istringstream in("unite 1 2 unite 3 4 unite 1 3 find 4 same 2 4 unite 2 4 rollback rollback rollback rollback");
        A->run(in);
    }
    else if (path == "/api/edge1") { nuevo(); A->intro(false); A->opRollback(0); }
    else if (path == "/api/edge2") { nuevo(); A->intro(false); A->opUnite(1, 2, 500); A->opUnite(2, 1, 0); }
    else if (path == "/api/op") {
        string c = q["cmd"];
        int a = num("a", 0), b = num("b", 0);
        if (c == "rollback") A->opRollback(0);
        else if (!A->ok(a) || (c != "find" && !A->ok(b)))
            A->msg("u y v deben estar entre 1 y " + S(A->n) + ".");
        else if (c == "unite") A->opUnite(a, b, 0);
        else if (c == "find") A->opFind(a, 900);
        else if (c == "same") A->opSame(a, b, 1200);
    }
    else return "[]";
    return A->take();
}

static void enviar(sock_t c, const string& status, const string& type, const string& body) {
    string h = "HTTP/1.1 " + status + "\r\nContent-Type: " + type + "\r\nContent-Length: " + to_string(body.size()) +
               "\r\nCache-Control: no-store\r\nConnection: close\r\n\r\n";
    string all = h + body;
    size_t off = 0;
    while (off < all.size()) {
        int k = send(c, all.data() + off, (int)(all.size() - off), 0);
        if (k <= 0) break;
        off += k;
    }
}

int main(int argc, char** argv) {
    int port = argc > 1 ? atoi(argv[1]) : 8080;
#ifdef _WIN32
    WSADATA w; WSAStartup(MAKEWORD(2, 2), &w);
#endif
    sock_t srv = socket(AF_INET, SOCK_STREAM, 0);
    int yes = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof yes);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((unsigned short)port);
    if (bind(srv, (sockaddr*)&addr, sizeof addr) < 0 || listen(srv, 8) < 0) {
        cerr << "No se pudo usar el puerto " << port << ". Prueba otro: ./dsu_animacion 9000\n";
        return 1;
    }
    A = make_unique<Animator>(8);
    cout << "Animación lista. Abre en el navegador:  http://localhost:" << port << "\n(Ctrl+C para cerrar)\n";
    while (true) {
        sock_t c = accept(srv, nullptr, nullptr);
        if (c < 0) continue;
        char buf[4096];
        int r = recv(c, buf, sizeof buf - 1, 0);
        if (r > 0) {
            buf[r] = 0;
            istringstream ls(string(buf).substr(0, string(buf).find("\r\n")));
            string met, url;
            ls >> met >> url;
            string path = url, query;
            size_t qm = url.find('?');
            if (qm != string::npos) { path = url.substr(0, qm); query = url.substr(qm + 1); }
            map<string, string> q;
            istringstream qs(query);
            string kv;
            while (getline(qs, kv, '&')) {
                size_t e = kv.find('=');
                if (e != string::npos) q[kv.substr(0, e)] = kv.substr(e + 1);
            }
            string type;
            string body = (met == "GET") ? handle(path, q, type) : "";
            if (path == "/favicon.ico") enviar(c, "204 No Content", "text/plain", "");
            else enviar(c, "200 OK", type, body);
        }
        CLOSESOCK(c);
    }
}