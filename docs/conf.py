from datetime import UTC, datetime

project = "git2cpp"
author = "QuantStack"
copyright = f"2025-{datetime.now(UTC).year}"

extensions = [
    "myst_parser",
]

exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]

html_css_files = ["custom.css"]
html_static_path = ["_static"]
html_theme = "sphinx_book_theme"
html_theme_options = {
    "home_page_in_toc": True,
    "navbar_persistent": [],
    "repository_url": "https://github.com/QuantStack/git2cpp",
    "show_navbar_depth": 2,
    "use_repository_button": True,
}
html_title = "git2cpp documentation"
myst_enable_extensions = ["deflist"]
