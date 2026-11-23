# DSU con rollback — Animación interactiva

**Proyecto Final 2 · CS2023 Algoritmos y Estructuras de Datos 2026-2**
Docente: Wilder Nina Choquehuayta

**Integrantes:** Keyra Huamanyauri · Antonio Guerrero · Elias Paz

**Estructura asignada:** DSU con rollback (Disjoint Set Union con retroceso).

---

## 1. ¿De qué trata el proyecto?

Un **DSU** (conjuntos disjuntos) mantiene grupos de elementos y responde rápidamente si dos elementos están en el mismo grupo. La variante **con rollback** permite además **deshacer la última unión**, dejando la estructura exactamente como estaba antes.

La implementación usa:
- **Unión por tamaño**: el árbol más pequeño se cuelga bajo la raíz del más grande.
- **Sin compresión de caminos**: la compresión cambia muchos padres sin registrarlos y el rollback no podría restaurarlos.
- **Pila `history`**: cada unión exitosa guarda el par `(x, y)` (raíz que queda, raíz que se cuelga).

| Operación | Complejidad |
|---|---|
| `find(u)` | O(log n) |
| `Same_Set(u, v)` | O(log n) |
| `unite(u, v)` | O(log n) |
| `rollback()` | O(1) |
| Memoria | O(n + uniones guardadas) |

## 2. Contenido del repositorio

| Archivo | Descripción |
|---|---|
| `DSU_rollback.cpp` | Implementación de la estructura DSU con rollback (con Kruskal en su `main` original). |
| `dsu_animacion.cpp` | Programa en C++ que ejecuta la estructura real y sirve la animación en el navegador. |
| `README.md` | Este archivo. |

## 3. ¿Cómo se genera la animación?

1. `dsu_animacion.cpp` incluye `DSU_rollback.cpp` y levanta un servidor web local.
2. Al pulsar un botón en la página (`unite`, `rollback`, `find`, `Same_Set`...), el navegador le pide el paso al programa.
3. El programa ejecuta la operación sobre un objeto `DSU` real y devuelve su estado: `parents`, `sizes`, `history` y número de componentes.
4. La página solo **dibuja** ese estado; no contiene lógica del DSU.

Para poder leer el estado, `DSU_rollback.cpp` incluye unos pocos métodos de solo lectura (`path`, `get_parents`, `get_sizes`, `get_history`, `top`) que no modifican el comportamiento de la estructura.

## 4. Software requerido

- **Compilador de C++17**: g++ 9 o superior (Linux, macOS, WSL, o MSYS2/MinGW en Windows).
- **Navegador web** moderno (Chrome, Edge, Firefox).
- Conexión a internet solo para cargar las fuentes de la página (opcional; sin ellas se usa una fuente por defecto).

No se necesita ninguna librería externa.

## 5. Compilar y ejecutar

### Linux, macOS o WSL
```bash
g++ -std=c++17 -O2 dsu_animacion.cpp -o dsu_animacion
./dsu_animacion
```

### Windows (PowerShell con MSYS2/MinGW)
```powershell
g++ -std=c++17 -O2 dsu_animacion.cpp -o dsu_animacion -lws2_32
.\dsu_animacion
```

### GitHub Codespaces (si tu computadora bloquea ejecutables)
1. En el repositorio: **Code → Codespaces → Create codespace on main**.
2. En la terminal, ejecuta los comandos de Linux de arriba.
3. Pulsa **Open in Browser** en el aviso del puerto 8080 (o abre la pestaña **PORTS** y haz clic en el globo).

### Abrir la animación
Con el programa corriendo, abre en el navegador:

**http://localhost:8080**

Para usar otro puerto: `./dsu_animacion 9000` y abre `http://localhost:9000`.
Cierra el programa con `Ctrl+C`.

### Ejecutar solo la estructura (Kruskal)
```bash
g++ -std=c++17 -O2 DSU_rollback.cpp -o kruskal
printf "4 5\n1 2 1\n2 3 2\n1 3 3\n3 4 4\n2 4 5\n" | ./kruskal    # imprime 7
```
En PowerShell:
```powershell
g++ -std=c++17 -O2 DSU_rollback.cpp -o kruskal
@"
4 5
1 2 1
2 3 2
1 3 3
3 4 4
2 4 5
"@ | .\kruskal
```
Entrada: `n m` y luego `m` líneas `u v w`. Imprime el peso del árbol de expansión mínima, o `IMPOSSIBLE` si el grafo no es conexo.

## 6. Uso de la página

- **n** y **Reiniciar**: cambian la cantidad de nodos (de 2 a 14).
- **u**, **v** con **unite(u, v)**: une los grupos de u y v.
- **rollback()**: deshace la última unión.
- **find(u)** y **Same_Set(u, v)**: consultas que solo leen (se ve el `find` recursivo y su pila de llamadas).
- **▶ Reproducir demo**: secuencia automática de unos 65 segundos (usa los nodos 1 a 4).
- **Casos borde**: `rollback` con `history` vacía y `unite` de nodos que ya están conectados.
- Paneles de **estado** (`parents`, `sizes`, componentes, `time()`), **history** y **complejidad**.

## 7. Herramientas usadas

- C++17 (estructura, servidor y lógica de la animación).
- HTML, CSS y JavaScript (solo para dibujar el estado que devuelve el programa).