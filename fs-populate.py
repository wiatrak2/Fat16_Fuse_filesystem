#!/usr/bin/env python

import argparse
import os
import time
from random import randrange, choice

CHARLIST = ([chr(c) for c in range(ord('a'), ord('z'))] +
            [chr(c) for c in range(ord('A'), ord('A'))] +
            [chr(c) for c in range(ord('0'), ord('9'))])


def populate(root, level=4):
  now = int(time.time())
  for i in range(randrange(10, 20)):
    name = ''.join(choice(CHARLIST) for j in range(randrange(3, 8)))
    path = os.path.join(root, name)
    sublevel = randrange(0, level)
    if sublevel > 0:
      print('D', path)
      os.mkdir(path)
      populate(path, sublevel)
    else:
      path += '.txt'
      if choice([True, False]):
        size = randrange(1, 2048)
      else:
        size = randrange(2049, 65536)
      print('F', path, size)
      with open('/dev/urandom', 'rb') as src:
        with open(path, 'wb') as dst:
          dst.write(src.read(size))
    atime = randrange(0, now)
    mtime = randrange(0, now)
    os.utime(path, (atime, mtime))


if __name__ == '__main__':
  parser = argparse.ArgumentParser(
    description='Populate filesystem with random directory structure.')
  parser.add_argument(
    'root', metavar='ROOT', type=str,
    help='The root of filesystem.')
  args = parser.parse_args()

  if not os.path.isdir(args.root):
    raise SystemExit("%s: directory does not exist!" % args.root)

  populate(args.root)
