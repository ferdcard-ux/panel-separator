# xfce4-custom-separator

Plugin separador configurable para el panel XFCE.

Permite insertar separadores con ancho, color y transparencia personalizables mediante un diálogo de configuración accesible desde el clic derecho del panel.

## Características

- Ancho configurable de 1 a 5000 píxeles
- Selector de color (cualquier color RGB)
- Modo transparente / visible
- Configuración persistente en `~/.config/xfce4-panel-custom-separator.conf`
- Compatible con XFCE 4.18+ (API 2.0, wrapper-2.0 externo)
- Paquete `.deb` incluido para instalación rápida

## Requisitos

- XFCE Panel 4.18+
- `libxfce4panel-2.0`
- `libgtk-3-0`
- `libxfconf-0`
- `libxfce4util`

En Ubuntu/Zorin:
```
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

El paquete se genera en `../xfce4-custom-separator_*.deb`.

## Uso

1. Tras instalar, el separador aparece al final del panel.
2. Clic derecho sobre el fondo del panel → **Separator Settings**.
3. Ajustar ancho, color y transparencia.
4. La configuración se guarda en `~/.config/xfce4-panel-custom-separator.conf`.

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
