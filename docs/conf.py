from datetime import date

project = "git2cpp"
author = "QuantStack"
copyright = f"2025-{date.today().year}"

extensions = [
    "myst_parser",
]

exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]

html_static_path = []
html_theme = "sphinx_book_theme"
html_theme_options = {
    "github_url": "https://github.com/QuantStack/git2cpp",
    "home_page_in_toc": True,
    "show_navbar_depth": 2,
}
html_title = "git2cpp documentation"
