# xfce4-custom-separator

Plugin separador configurable para el panel XFCE.

Permite insertar separadores con ancho, color y transparencia personalizables mediante un diálogo de configuración accesible desde el clic derecho del panel. Soporta múltiples instancias independientes.

## Características

- Ancho configurable de 1 a 5000 píxeles
- Selector de color (cualquier color RGB)
- Modo transparente / visible
- Auto-expand: el separador llena todo el espacio disponible
- Configuración por instancia (cada separador guarda sus propios ajustes)
- Configuración persistente en `~/.config/xfce4-panel-custom-separator.conf`
- Compatible con XFCE 4.18+ (API 2.0, wrapper-2.0 externo)
- Paquete `.deb` incluido para instalación rápida

## Múltiples instancias

Cada separador se identifica por su plugin ID (asignado dinámicamente por el panel). La configuración se almacena por secciones en el archivo de config:

```ini
[2]
width=20
transparent=true
auto_expand=false
color=#800080

[4]
width=6
transparent=false
auto_expand=true
color=#ff0000
```

## Requisitos

- XFCE Panel 4.18+
- `libxfce4panel-2.0`
- `libgtk-3-0`
- `libxfconf-0`
- `libxfce4util`

En Ubuntu/Zorin:
```bash
sudo apt install libxfce4panel-2.0-dev libxfce4ui-2-dev libxfce4util-dev libxfconf-0-dev libgtk-3-dev
```

## Instalación desde .deb

```bash
sudo dpkg -i output/xfce4-custom-separator_1.0.0_amd64.deb
```

El postinst añade automáticamente el plugin al panel. Cerrar sesión y volver a entrar para verlo.

## Compilación manual

```bash
make
sudo make install
```

El plugin se instala en:
- `.so` → `/usr/lib/x86_64-linux-gnu/xfce4/panel/plugins/`
- `.desktop` → `/usr/share/xfce4/panel/plugins/`

## Generar el .deb

```bash
make deb
```

El paquete se genera en `output/`.

## Uso

1. Tras instalar, el separador aparece al final del panel.
2. Clic derecho sobre el fondo del panel → **Separator Settings**.
3. Ajustar ancho, color, transparencia y auto-expand.
4. La configuración se guarda por instancia en `~/.config/xfce4-panel-custom-separator.conf`.

## Desinstalar

```bash
sudo dpkg -r xfce4-custom-separator
```

El postrm elimina la entrada del plugin del XML del panel.

## Estructura del proyecto

```
panel-separator/
├── separator-plugin.c          # Código fuente del plugin
├── Makefile                    # Compilación e instalación
├── debian/                     # Empaquetado Debian
│   ├── control
│   ├── rules
│   ├── changelog
│   ├── custom-separator.desktop
│   ├── install
│   ├── postinst
│   └── postrm
├── output/                     # Paquetes generados
│   └── xfce4-custom-separator_1.0.0_amd64.deb
├── .gitignore
└── README.md
```

## Licencia

GPL-2.0
