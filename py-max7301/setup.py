import sys
import subprocess

try:
    from setuptools import setup
except ImportError:
    from distutils.core import setup

setup(
    name='pymax7301',
    version='1.0.0',
    description='MAX7301 driver',
    author='Tobias Schneider',
    author_email='schneider@xtort.eu',
    url='https://github.com/muccc/flipdots',
    packages=['max7301'],
    long_description=open('README.md').read(),
    classifiers=[
        "License :: OSI Approved :: GNU General Public License v3 or ",
        "later (GPLv3+)",
        "Programming Language :: Python :: 2",
        "Intended Audience :: Developers",
        "Topic :: Software Development :: Libraries :: Python Modules",
    ],
    keywords='max7301',
    license='GPLv3+',
)
