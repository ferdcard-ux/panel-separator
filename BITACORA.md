# BITÁCORA DE DESARROLLO — xfce4-custom-separator

Historial completo del desarrollo del plugin separador configurable para XFCE Panel.

**Repositorio:** https://github.com/ferdcard/panel-separator
**Autor:** ferdcard
**Inicio:** 14 de julio de 2026
**Licencia:** GPL-2.0

---

## Resumen del proyecto

Plugin externo para XFCE Panel 4.18+ que permite insertar separadores visuales con ancho, color, transparencia y modo auto-expand configurables. Soporta múltiples instancias independientes con configuración persistente.

**Stack:** C, GTK 3, libxfce4panel-2.0, Makefile, packaging Debian.

**Estado actual:** v1.0.0 — funcional y empaquetado como `.deb`.

---

## Registro de cambios

### 2026-07-14 — v1.0.0 (commit `bd5e608`)

**Lanzamiento inicial del plugin.**

Se creó el proyecto completo en una sola sesión:

**Funcionalidades implementadas:**
- Plugin externo registrado con `XFCE_PANEL_PLUGIN_REGISTER(construct)`
- `SeparatorData` struct para estado de cada instancia (ancho, color, transparencia)
- Ancho configurable de 1 a 5000 píxeles con `GtkSpinButton`
- Selector de color RGB con `GtkColorButton`
- Modo transparente / visible (toggle checkbox)
- Dibujado del separador con Cairo (`cairo_paint` sobre `GtkDrawingArea`)
- Diálogo de configuración accesible desde clic derecho del panel (`configure-plugin` signal)
- Configuración persistente en formato INI: `~/.config/xfce4-panel-custom-separator.conf`
- Parser de config escrito a mano (sin dependencias externas)
- Handler de `size-changed` para re-aplicar ancho al cambiar tamaño del panel

**Empaquetado Debian:**
- `debian/control` con dependencias de build
- `debian/rules` con overrides para GCC directo
- `debian/custom-separator.desktop` descriptor del plugin (API 2.0, wrapper-2.0 externo)
- `debian/postinst` que detecta usuario real, lee `xfce4-panel.xml`, y añade el plugin automáticamente
- `debian/postrm` que elimina la entrada del plugin al desinstalar
- `debian/install` mapeo de archivos
- `make deb` para generar paquete en `output/`

**Archivos creados (11):**
`separator-plugin.c` (210 líneas), `Makefile`, `README.md`, `.gitignore`, y 7 archivos en `debian/`.

---

### 2026-07-15 — v1.0.0+ (commit `8da0ba6`)

**Multi-instance, auto-expand y limpieza.**

Se refactorizó el plugin para soportar múltiples instancias independientes:

**Cambios en `separator-plugin.c` (210 → 354 líneas, +174 netas):**
- Nueva función `read_plugin_id_from_cmdline()`: lee `/proc/self/cmdline` para obtener el plugin ID asignado por el panel
- `load_config()` reescrito: parsea secciones `[plugin-ID]` del archivo INI en vez de leer todo globalmente
- `save_config()` reescrito: reemplaza o agrega la sección correspondiente manteniendo las demás intactas
- Nuevo campo `auto_expand` en `SeparatorData`
- Nueva función `apply_expand_mode()`: alterna entre `hexpand=TRUE` (llena espacio) y ancho fijo
- Nuevo callback `on_expand_toggled()`: desactiva el spin de ancho cuando auto-expand está activo
- Callback `on_color_toggled()` para desactivar botón de color en modo transparente
- Refactor del diálogo `on_configure()`: layout con `GtkGrid`, checkboxes reordenados
- Configuración por instancia: cada separador guarda `[2]`, `[4]`, etc. independientemente

**Cambios en `README.md`:**
- Documentación actualizada con nuevas funciones (auto-expand, multi-instance)
- Ejemplo de config INI con múltiples secciones

**Limpieza:**
- Eliminados artefactos de build que se habían commiteado por error

---

### 2026-07-18 — Bugfix: crash intermitente al iniciar sesión

**Problema:** Algunas veces al hacer login, el separador no cargaba y XFCE mostraba "el complemento se ha detenido forzosamente".

**Causa raíz:** Buffer overread en `read_plugin_id_from_cmdline()`. Al parsear `/proc/self/cmdline`, cuando se procesaba el último argumento, `p += strlen(p) + 1` apuntaba más allá del buffer分配ido por `g_file_get_contents`. La llamada a `strlen(p)` sobre ese puntero era undefined behavior — dependiendo del estado de la memoria, podía funcionar o crashear con segfault.

**Fix (2 cambios en `separator-plugin.c`):**
1. `read_plugin_id_from_cmdline()`: se usa `gsize length` y un puntero `end = cmdline + length` para limitar el ciclo. La condición ahora es `while (p < end)` en vez de `while (strlen(p) > 0)`, evitando el acceso fuera de rango.
2. `construct()`: se agregó un fallback — si `read_plugin_id_from_cmdline()` retorna -1, intenta obtener el ID via `g_object_get_data(G_OBJECT(plugin), "xfce-plugin-id")` antes de continuar. Si ambos fallan, el plugin carga con defaults en vez de crashear.

---

## Arquitectura y diseño

### Flujo de vida del plugin

```
construct() [entry point]
  ├── g_new0(SeparatorData)
  ├── read_plugin_id_from_cmdline() → obtiene ID de /proc/self/cmdline
  ├── load_config() → lee sección [ID] del archivo INI
  ├── gtk_drawing_area_new() → crea widget de dibujo
  ├── apply_expand_mode() → configura hexpand o ancho fijo
  ├── gtk_container_add() → inserta en el panel
  └── Conecta señales: size-changed, configure-plugin, draw
```

### Sistema de configuración

- **Formato:** INI manual, sin librería externa
- **Ruta:** `~/.config/xfce4-panel-custom-separator.conf`
- **Clave:** Sección `[plugin-ID]` por instancia
- **Lectura:** Busca la sección, parsea línea por línea con prefijos (`width=`, `transparent=`, etc.)
- **Escritura:** Reconstruye el archivo completo reemplazando la sección correspondiente

### Identificación de instancias

El plugin lee `/proc/self/cmdline` al iniciar. XFCE panel invoca cada instancia como:
```
wrapper-2.0 --plugin <socket-path> <plugin-id> ...
```
El tercer argumento (`argv[2]`) es el plugin ID numérico, que se usa como clave de sección en la config.

### Empaquetado Debian

- `postinst`: ejecuta `xfconf-query` o parsea `xfce4-panel.xml` directamente para insertar la entrada del plugin
- `postrm`: busca y elimina la línea correspondiente al `internal-name="custom-separator"` en el XML del panel
- Ambos scripts detectan el usuario real (`SUDO_USER` o `logname`) para acceder a su config de XFCE

---

## Dependencias

### Runtime
- XFCE Panel 4.18+
- libxfce4panel-2.0
- libxfce4ui-2
- libxfce4util
- libxfconf-0
- GTK 3

### Build
- gcc
- make
- pkg-config
- debhelper-compat (= 13)

---

## Decisions de diseño

| Decisión | Alternativa descartada | Razón |
|---|---|---|
| Parser INI a mano | libconfuse, GKeyFile | Evitar dependencias, el formato es trivial |
| ID de plugin vía `/proc/self/cmdline` | xfconf API, archivos temporales | Método más directo, no requiere APIs adicionales |
| Plugin externo (wrapper-2.0) | Plugin interno | Mejor aislamiento, compatible con API 2.0 |
| Config en `~/.config/` | xfconf (xfce settings daemon) | Más simple, sin daemon, archivo directo |
| GtkGrid layout en diálogo | GtkTable (deprecated), manual packing | Layout moderno y limpio para GTK 3 |

---

## Pendiente / Ideas futuras

- [ ] Soporte para separador vertical (orientación del panel)
- [ ] Borde/outline configurable
- [ ] Icono personalizado en el panel
- [ ]国际化 (i18n) — infrastructure de gettext ya definida pero no activa
- [ ] Soporte para wayland (verificar compatibilidad de `/proc/self/cmdline`)
- [ ] Publicación en repositorio PPA
- [ ] Tests unitarios para el parser de config

---

## Comandos útiles

```bash
# Compilar
make

# Instalar localmente
sudo make install

# Generar .deb
make deb

# Instalar .deb
sudo dpkg -i output/xfce4-custom-separator_1.0.0_amd64.deb

# Desinstalar
sudo dpkg -r xfce4-custom-separator

# Ver config
cat ~/.config/xfce4-panel-custom-separator.conf
```

---

## Git

```
* 8da0ba6  2026-07-15  feat: per-instance config, auto-expand, cleanup
* bd5e608  2026-07-14  feat: XFCE panel separator plugin v1.0.0
```

Branch: `main`
