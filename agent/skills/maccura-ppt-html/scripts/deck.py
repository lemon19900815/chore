import argparse
import base64
from html import escape
from html.parser import HTMLParser
import json
from pathlib import Path
import re
import shutil
import sys

BACKGROUND_NAMES = ('cover', 'catalogue', 'chapter', 'content', 'closing')
TEMPLATE = Path(__file__).resolve().parent.parent / 'assets' / 'template'


class Document(HTMLParser):
    def __init__(self, source):
        super().__init__(convert_charrefs=True)
        self.elements = []
        self.feed(source)

    def handle_starttag(self, tag, attrs):
        self.elements.append((tag, dict(attrs)))


def background_script(root):
    images = {name: 'data:image/png;base64,' + base64.b64encode(
        (root / 'assets' / (name + '.png')).read_bytes()).decode('ascii')
        for name in BACKGROUND_NAMES}
    return '''(() => {
  const backgrounds = %s;
  for (const image of document.querySelectorAll('img[data-background]')) {
    image.src = backgrounds[image.dataset.background];
  }
})();
''' % json.dumps(images, separators=(',', ':'))


def validate_html(source):
    elements = Document(source).elements
    ids = [attrs['id'] for _, attrs in elements if 'id' in attrs]
    if len(ids) != len(set(ids)):
        raise ValueError('Duplicate HTML IDs')
    slides = [a for _, a in elements if 'slide' in a.get('class', '').split()]
    tabs = [a for _, a in elements if a.get('role') == 'tab']
    slide_ids = [a.get('id') for a in slides]
    if not slides or len(slides) != len(tabs):
        raise ValueError('Each slide requires one matching tab, in the same order')
    for slide, tab in zip(slides, tabs):
        if not re.fullmatch(r'[a-zA-Z][a-zA-Z0-9_-]*', slide.get('id', '')):
            raise ValueError('Slide IDs must start with a letter and use letters, digits, hyphens or underscores')
        if (tab.get('data-slide') != slide['id'] or tab.get('aria-controls') != slide['id']
                or not tab.get('id') or slide.get('aria-labelledby') != tab['id']):
            raise ValueError('Slide/tab ID, aria and data-slide mappings disagree')
    required = {'export-image', 'export-status', 'layout-name', 'layout-note', 'page-counter', 'previous', 'next',
                'toggle-fullscreen', 'exit-fullscreen', 'fullscreen-previous', 'fullscreen-next',
                'fullscreen-export', 'fullscreen-counter', 'fullscreen-status', 'fullscreen-message'}
    if not required.issubset(ids):
        raise ValueError('Missing navigation, export or fullscreen controls: ' + ', '.join(sorted(required - set(ids))))
    for tag, attrs in elements:
        if 'data-slide' in attrs and attrs['data-slide'] not in slide_ids:
            raise ValueError('Navigation points to an unknown slide')
        if 'data-background' in attrs and attrs['data-background'] not in BACKGROUND_NAMES:
            raise ValueError('Unknown data-background name')
        if tag in ('iframe', 'object', 'embed', 'video', 'audio', 'source'):
            raise ValueError('Use embedded raster images instead of external media or nested documents')
        if 'srcset' in attrs:
            raise ValueError('Remove srcset; export requires one embedded image source')
        if tag == 'img' and 'data-background' not in attrs and not attrs.get('src', '').startswith('data:image/'):
            raise ValueError('Additional images must be embedded data:image URLs before building')
        for attr in ('src', 'href', 'xlink:href', 'poster'):
            value = attrs.get(attr)
            if value and tag != 'a' and not value.startswith(('data:', '#')):
                raise ValueError('Non-embedded resource remains: ' + value[:100])
    return len(slides)


def build(root, output, overwrite):
    root = root.resolve()
    output = output.resolve()
    if output.suffix.lower() != '.html':
        raise ValueError('Output must be an .html file')
    if output == root / 'index.html' or output.is_relative_to(root / 'assets'):
        raise ValueError('Output must not overwrite source files or assets')
    if output.exists() and not overwrite:
        raise ValueError('Output exists; choose a new file or explicitly use --overwrite')
    html = (root / 'index.html').read_text(encoding='utf-8')
    css = (root / 'style.css').read_text(encoding='utf-8')
    js = (root / 'preview.js').read_text(encoding='utf-8')
    if re.search(r'@import\b', css, re.I):
        raise ValueError('CSS imports are not allowed in the offline template')
    for url in re.findall(r'url\(\s*[\'"]?([^\)\'\"]+)', css + html, re.I):
        if not url.startswith(('data:', '#')):
            raise ValueError('CSS images/fonts must be embedded, not external resources')
    if re.search(r'</style', css, re.I):
        raise ValueError('CSS contains an embedded closing style tag')
    assets = background_script(root)
    replacements = {
        '<link rel="stylesheet" href="style.css">': '<style>\n' + css + '</style>',
        '<script src="assets/backgrounds.js" defer></script>': '',
        '<script src="preview.js" defer></script>': '',
        '</body>': '<script>\n' + assets + re.sub(r'</script', r'<\\/script', js, flags=re.I) + '</script>\n</body>'
    }
    for old, new in replacements.items():
        if html.count(old) != 1:
            raise ValueError('Keep exactly one template marker: ' + old)
        html = html.replace(old, new)
    count = validate_html(html)
    (root / 'assets' / 'backgrounds.js').write_text(assets, encoding='utf-8')
    with output.open('w' if overwrite else 'x', encoding='utf-8') as stream:
        stream.write(html)
    print(f'Built {count} slides: {output} ({output.stat().st_size} bytes)')
    print('Next: verify the standalone HTML in an offline browser.')


def main():
    parser = argparse.ArgumentParser(description='Create and package the maccura offline HTML slide template. Python 3.9+, standard library only.')
    commands = parser.add_subparsers(dest='command', required=True)
    init = commands.add_parser('init', help='Copy the template into a NEW directory; content is still illustrative')
    init.add_argument('directory', type=Path)
    init.add_argument('--title', default='迈克演示文稿')
    package = commands.add_parser('build', help='Embed CSS, JS and brand backgrounds into one HTML file')
    package.add_argument('directory', type=Path)
    package.add_argument('--output', type=Path)
    package.add_argument('--overwrite', action='store_true', help='Explicitly replace the generated HTML output')
    args = parser.parse_args()
    root = args.directory.resolve()
    if args.command == 'init':
        if root.exists():
            raise ValueError('Destination already exists; choose a new directory')
        if not root.parent.is_dir():
            raise ValueError('Destination parent directory must already exist')
        shutil.copytree(TEMPLATE, root)
        entry = root / 'index.html'
        html = entry.read_text(encoding='utf-8')
        title = escape(args.title)
        html = re.sub(r'<title>.*?</title>', lambda _: '<title>' + title + '</title>', html, count=1)
        html = html.replace('模板 A <span>/ HTML 视觉预览</span>', title + ' <span>/ 离线演示</span>')
        html = html.replace('让表达，更精准', title, 1)
        entry.write_text(html, encoding='utf-8')
        (root / 'assets' / 'backgrounds.js').write_text(background_script(root), encoding='utf-8')
        print(f'Initialized {root}; edit index.html and style.css, then run build.')
    else:
        build(root, args.output or root / '演示文稿-离线版.html', args.overwrite)


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError) as error:
        print('ERROR: ' + str(error), file=sys.stderr)
        sys.exit(1)
