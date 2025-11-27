CommLink logo assets

Files created
- `assets/logo/CommLink_logo.svg` — primary logo (icon + wordmark + tagline). Good for high-res export and branding.
- `assets/logo/CommLink_icon.svg` — simplified icon-only SVG (rounded square with link motif). Good for app icons, favicons, notifications.

How to export PNG / other sizes

If you have `rsvg-convert` (from librsvg):

```bash
# 512x512 PNG
rsvg-convert -w 512 -h 512 assets/logo/CommLink_logo.svg -o assets/logo/CommLink_logo_512.png

# Icon-only PNG (256x256)
rsvg-convert -w 256 -h 256 assets/logo/CommLink_icon.svg -o assets/logo/CommLink_icon_256.png
```

Using Inkscape CLI:

```bash
# 512x512
inkscape assets/logo/CommLink_logo.svg --export-type=png --export-filename=assets/logo/CommLink_logo_512.png --export-width=512 --export-height=512
```

Using ImageMagick (rasterizes via librsvg/png):

```bash
convert -background none assets/logo/CommLink_logo.svg -resize 512x512 assets/logo/CommLink_logo_512.png
```

Notes and next steps
- Want other color variants (dark/light), different taglines, or alternative fonts? I can produce them.
- I can also export standard icon sizes (16,32,48,64,128,256,512) and produce an .ico for Windows or ICNS for macOS if you want.
- If you'd like the wordmark text removed or a horizontal vs stacked layout, tell me which layout and I will add it.

License
- These logo files are created for your project. If you want an open-source license header, say which license to add.
