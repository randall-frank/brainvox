import os

# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'Brainvox'
html_short_title = html_title = 'Brainvox'
copyright = '2026, Randall Frank'
author = 'Randall Frank'

root = os.path.join(os.path.dirname(__file__), "..", "..")
with open(os.path.join(root, "version.txt"), "r") as f:
    version = f.read().strip()
release = version

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    "sphinx.ext.extlinks",
    "sphinx.ext.githubpages",
    "sphinxcontrib.video",
]

templates_path = ['_templates']
exclude_patterns = []


# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = "pydata_sphinx_theme"
html_static_path = ['_static']
html_show_sourcelink = False
html_favicon = "_static/favicon.ico"