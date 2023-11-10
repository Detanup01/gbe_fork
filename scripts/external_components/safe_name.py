
import re


ALLOWED_CHARS = set([
    '`', '~', '!', '@',
    '#', '$', '%', '&',
    '(', ')', '-', '_',
    '=', '+', '[', '{',
    ']', '}', ';', '\'',
    ',', '.', ' ', '\t',
    '®', '™',
])

def create_safe_name(app_name : str):
    safe_name = ''.join(c for c in f'{app_name}' if c.isalnum() or c in ALLOWED_CHARS)\
        .rstrip()\
        .rstrip('.')\
        .replace('\t', ' ')
    safe_name = re.sub('\s\s+', ' ', safe_name)
    return safe_name

