"""Small shared helpers for distribution scripts (Python 3.9+)."""
from pathlib import Path
import hashlib
import re
import struct
import zipfile

ROOT = Path(__file__).resolve().parent.parent
PRIVATE_SUFFIXES = {'.p12', '.pfx', '.jks', '.keystore', '.key', '.pem'}
EXCLUDED_DIRS = {'.git', '.cache', '__pycache__', 'build', 'dist', 'artifacts', 'signing', 'tooling', 'doxygen'}


def version():
    value = (ROOT / 'VERSION').read_text().strip()
    if not re.fullmatch(r'\d+\.\d+\.\d+', value):
        raise ValueError('VERSION must contain major.minor.patch')
    return value


def source_archive(destination):
    """Archive buildable project sources, excluding credentials and generated files."""
    folders = ('src', 'include', 'resources', 'cmake', 'scripts', 'android', 'tests', 'docs', 'packaging')
    files = [ROOT / name for name in ('CMakeLists.txt', 'VERSION', 'LICENSE', 'README.md', 'AUTHORS', 'CHANGELOG.md', 'CONTRIBUTING.md', '.gitignore')]
    for folder in folders:
        files.extend((ROOT / folder).rglob('*'))
    destination = Path(destination)
    destination.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(destination, 'w', zipfile.ZIP_DEFLATED) as archive:
        for file in sorted(set(files)):
            relative = file.relative_to(ROOT)
            if not file.is_file() or file.is_symlink():
                continue
            if any(part in EXCLUDED_DIRS or part.startswith('build-') for part in relative.parts):
                continue
            if file.suffix.lower() in PRIVATE_SUFFIXES | {'.apk', '.aab', '.exe', '.zip', '.log', '.pyc', '.idsig'}:
                continue
            if 'password' in file.name.lower():
                continue
            archive.write(file, Path('CommLink-source') / relative)
    return destination


def sha256(file):
    digest = hashlib.sha256()
    with Path(file).open('rb') as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b''):
            digest.update(chunk)
    return digest.hexdigest()


def manifest(directory):
    directory = Path(directory)
    lines = [sha256(file) + '  ' + file.relative_to(directory).as_posix()
             for file in sorted(directory.rglob('*')) if file.is_file() and file.name != 'SHA256SUMS.txt']
    (directory / 'SHA256SUMS.txt').write_text('\n'.join(lines) + '\n', encoding='utf-8')


def pe_imports(file):
    """Return PE machine type and imported DLL names without external tools."""
    data = Path(file).read_bytes()
    if data[:2] != b'MZ' or len(data) < 64:
        raise ValueError(f'Not a Windows executable: {file}')
    pe = struct.unpack_from('<I', data, 60)[0]
    if data[pe:pe + 4] != b'PE\0\0':
        raise ValueError(f'Invalid PE header: {file}')
    machine, sections = struct.unpack_from('<HH', data, pe + 4)
    optional_size = struct.unpack_from('<H', data, pe + 20)[0]
    optional = pe + 24
    magic = struct.unpack_from('<H', data, optional)[0]
    directory = optional + {0x20b: 112, 0x10b: 96}[magic]
    import_rva = struct.unpack_from('<I', data, directory + 8)[0]
    if not import_rva:
        return machine, set()

    def offset(rva):
        if rva < struct.unpack_from('<I', data, optional + 60)[0]:
            return rva
        for number in range(sections):
            section = optional + optional_size + number * 40
            virtual_size, address, raw_size, raw_offset = struct.unpack_from('<IIII', data, section + 8)
            if address <= rva < address + max(virtual_size, raw_size):
                return raw_offset + rva - address
        raise ValueError(f'Invalid import address in {file}')

    position = offset(import_rva)
    imports = set()
    while any(data[position:position + 20]):
        name_rva = struct.unpack_from('<I', data, position + 12)[0]
        start = offset(name_rva)
        end = data.index(b'\0', start)
        imports.add(data[start:end].decode('ascii').lower())
        position += 20
    return machine, imports
