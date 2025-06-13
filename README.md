[VOICEVOX/pyopenjtalk](https://github.com/VOICEVOX/pyopenjtalk)を参考にした、通常のopenJTalkのHTS部分を削除し、日本語文を変換する処理のみに変更したものです。
[nikkorinyuki/wasm_open_jtalk](https://github.com/nikkorinyuki/wasm_open_jtalk)に使用するためのリポジトリです。

# open_jtalk

[![C/C++ CI](https://github.com/r9y9/open_jtalk/actions/workflows/ccpp.yaml/badge.svg)](https://github.com/r9y9/open_jtalk/actions/workflows/ccpp.yaml)

A fork of open_jtalk based on v1.10.

## Why

Wanted to fork it with *git*.

**NOTE**: To preserve history of cvs version of open_jtalk, this fork was originially created by:

```
git cvsimport -v \
  -d :pserver:anonymous@open-jtalk.cvs.sourceforge.net:/cvsroot/open-jtalk \
  -C open_jtalk open_jtalk
```
