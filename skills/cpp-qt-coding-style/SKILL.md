---
name: cpp-qt-coding-style
description: 'Use when editing C++/Qt code in any project (Qt Widgets, qmake/CMake). Enforces Allman braces, readable one-line parameter lists, CSS/QSS adjacent-string indentation patterns, and reusable implementation patterns over ad-hoc boilerplate.'
---

# C++ Qt Coding Style

Use this skill whenever you edit C++/Qt source files.

## 1) File Types
- Apply this style to .cpp and .h files in Qt-based projects.
- Use unix line endings (LF) and UTF-8 encoding.
- Follow existing project conventions for file organization and naming.
- Put .cpp and .h files in the same directory with .pro or CMakeLists.txt, unless the project has a different established structure.

## 2) Braces and Layout
- Use Allman brace style.
- Opening brace goes on the next line for functions, classes, conditionals, loops, and switches.
- Closing brace is on its own line, aligned with the opening scope.

## 3) Function Signatures
- Keep parameter lists on one line when readable.
- Do not force one-parameter-per-line formatting.
- Wrap parameter lists only when line length/readability requires it.

## 4) CSS/QSS In C++ String Literals
- For adjacent CSS/QSS literals, keep selector/opening and closing-brace literal lines at a base indent.
- Indent property literal lines one level deeper than the selector/opening line.
- Keep indentation consistent across all blocks in the same file.

Example pattern:
```cpp
const char *const kExampleStyle =
        "QLabel {"
            "padding: 8px;"
            "border-radius: 6px;"
        "}"
        "QLabel:hover {"
            "background-color: #f0f0f0;"
        "}";
```

## 5) Change Discipline
- Prefer minimal, focused diffs.
- Preserve existing APIs and behavior unless the task explicitly requires changes.
- Avoid reformatting unrelated code.

## 6) Reusability First
- Prefer reusable structure over one-off inline logic.
- When behavior may be reused (for example widget styling/setup), prefer class methods or helper functions over anonymous lambda blocks.
- Avoid copy-paste boilerplate across files or functions; extract repeated patterns into a shared method/utility.
- Keep UI setup DRY: centralize repeated style strings, button setup, and common layout setup in named helpers.
- If logic appears in 2+ places, refactor toward a single source of truth unless there is a clear reason not to.

Preferred approach example:
```cpp
// Prefer a reusable method
void MainWindow::applyPrimaryButtonStyle(QToolButton *button)
{
    button->setStyleSheet(sPrimaryButtonStyle);
    button->setCursor(Qt::PointingHandCursor);
}

// Instead of repeating ad-hoc inline setup in many places
```

## 7) Namespace Usage
- Do not introduce namespaces unless they provide clear value or are explicitly requested.
- Avoid unnecessary anonymous namespaces for small single-file helpers.
- For file-local helper functions, prefer `static` functions when that keeps the code simpler and clear.
- Use named namespaces for real domain/module grouping, not as default boilerplate.
