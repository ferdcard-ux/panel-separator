# xfce4-custom-separator v1.0.1

## Bugfix

- **Crash intermitente al iniciar sesión.** Corregido buffer overread en `read_plugin_id_from_cmdline()` que causaba segfault dependiendo del layout de memoria del sistema. El plugin ahora parsea `/proc/self/cmdline` con bounds checking correcto y tiene un fallback para obtener el plugin ID si la lectura del cmdline falla.

## Instalación

```bash
sudo dpkg -i xfce4-custom-separator_1.0.1_amd64.deb
```

## Cambios desde v1.0.0

- `separator-plugin.c:34-60` — read_plugin_id_from_cmdline() reescrita con `p < end` en vez de `strlen(p) > 0`
- `separator-plugin.c:336-340` — fallback para plugin ID via `xfce-plugin-id` property
- `debian/changelog` — versión 1.0.1
