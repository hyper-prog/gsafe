---
name: gsafe-po-template-language
description: "Use when editing gSAFE po template documents (doc.txt, .pot and generated instruction strings) interpreted by HTextProcessor and HPageTileRenderer in po.h/po.cpp."
---

# gSAFE po Template Language

Use this skill whenever you edit template instruction documents rendered by gSAFE po components.

## 1) Scope And Source Of Truth
- Runtime behavior is defined by parser code in gSAFE po sources:
  - po.h (language docs and API)
  - po.cpp (actual command dispatch and edge-case behavior)
- If this skill and po.cpp conflict, po.cpp is authoritative.
- Commands are case-sensitive.

## 2) Processing Model
The language has two layers.

1. HTextProcessor preprocessing
- Handles comments, annotations, tokens, conditions, functions, and ALLVARIABLES.
- Produces final instruction text.

2. HPageTileRenderer rendering
- Parses one command per line using # separators.
- Executes drawing/layout operations.

## 3) Line Rules
- One line = one command.
- Empty lines are ignored.
- Only lines starting with // are comments in HTextProcessor.
- Multiline argument form is allowed only with a line ending in #{.
- Multiline block ends on a line that only contains } (after trim).

Valid multiline sample:
```text
html#100%#{
This is multiline text.
Second line.
}
```

## 4) HTextProcessor Syntax

### 4.1 Comments and annotations
- Comments: lines whose first token starts with //.
- Annotation form in comment: // @Name:Value
- Stored and queryable via annotations().

### 4.2 Token expansion
- Token form: {{TOKEN}}
- Map/list lookup tokens:
  - {{.container.key}}
  - List index lookup is supported with numeric key.
- Fallback chain:
  - {{.a.b|.x.y|literal fallback}}
- Special escaped pseudo-tokens:
  - {{..point}} -> .
  - {{..colon}} -> :
  - {{..semicolon}} -> ;
  - {{..question}} -> ?
  - {{..openbrackets}} -> {
  - {{..closebrackets}} -> }

### 4.3 Conditional expression token
- Form inside token: left=right?then:else
- Use [[...]] in then/else branch for deferred token expansion.
- [[...]] is used to avoid too-early substitution before branch selection.

### 4.4 Block conditions
- Start: COND#<value1>#<operator>#<value2>
- End: ENDC
- Operators: =  !=  <  >  <=  >=
- Numeric comparisons require both sides parse as numbers.

### 4.5 Function blocks
- Define: FUNC#<name>
- End define: ENDF
- Call: CALL#<name>
- Function body is recursively processed via processDoc.

### 4.6 ALLVARIABLES
- Expands all registered maps/lists as renderer instructions.
- Useful for debug/template introspection.

## 5) Renderer Command Set

### 5.1 Cursor and flow
- mova#x,y
- movr#dx,dy
- newl
- newp
- npif#requiredHeight
- npuc#pageCount

### 5.2 Text and images
- text#width#plain text
- html#width#html text
- mark#width#markdown text
- imgr#width#imagePathOrResource
- imgb#width#base64Image

Absolute variants (do not move cursor):
- text#x,y,width#...
- html#x,y,width#...
- mark#x,y,width#...
- imgr#x,y,width#...
- imgb#x,y,width#...

### 5.3 Shapes and spacing
- spac#width,height
- rect#width,height
- rect#x,y,width,height
- grid#x,y,width,height

### 5.4 Line height controls
- fixh
- smhz
- smhv#height
- smht#width#text
- smhh#width#html
- smhm#width#markdown
- smhr#width#imagePathOrResource
- smhi#width#base64Image

### 5.5 Areas
- area#width,height
- area#x,y,width,height
- reta

### 5.6 Style and color
- alig#left|center|right|just
- setf#FontName,PointSize
- setd#FontName,PointSize
- deff
- colf#rrggbb
- coll#rrggbb
- colb#rrggbb
- fram#none|all|top|right|bottom|left|fill (comma separated)
- marg#top,right,bottom,left
- sizc#factor

### 5.7 Position capture
- getp#name
- Stored positions are used by callers (for example signature placement).

### 5.8 Per-page replay sections
- EVERYPAGE_START
- EVERYPAGE_END
- Commands between them are replayed when a new page starts.

## 6) Size String Semantics
Supported size forms:
- 120 -> pixels
- 120px -> pixels
- 20% -> percent of usable width/height
- 2em -> font-relative size
- 5mm, 1cm -> physical units (affected by sizc)
- -10%, -80, -2em -> reverse from right/bottom edge
- >50%, >-2em -> relative to current cursor position

## 7) Runtime Token Substitution In Renderer
Renderer text/image commands also support:
- @@{pagenum}@@
- @@{renderdate}@@
- @@{rendertime}@@

## 8) Unknown Command Policy
- Do not invent new commands.
- If requirement cannot be expressed with existing commands, ask the programmer/author what to do.
- Treat parser extension as an explicit engineering task, not a template edit.

## 9) Authoring Guardrails For Agents
- Preserve existing command casing exactly.
- Avoid reformatting command lines unless needed for correctness.
- Keep multiline blocks strict: start with #{, close with } on its own line.
- For ternary token branches, use [[...]] for deferred substitution.
- Prefer minimal diffs and keep behavior stable unless requested.

## 10) Quick Validation Checklist
After editing a template:
1. Confirm every opened COND/FUNC/multiline block is closed.
2. Confirm one command per line outside multiline payloads.
3. Confirm command names are valid and case-correct.
4. Confirm resource paths used by imgr exist in resources.
5. Enable renderer unknown-command warnings during test rendering when available.

## 11) CivisDesk Notes
- Typical template sources include doc.txt and .pot resources.
- Generated instruction strings in C++ are interpreted by the same language.
- Keep Hungarian user-facing text intact unless the task requests text changes.

## 12) Compatibility Policy
- This skill targets current po.cpp behavior.
- When po.* changes, update this skill from source (not from memory).
- If moved into gSAFE repository later, keep this file next to po.* docs and maintain it there.
