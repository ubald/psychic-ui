# WIP

# Psychic UI

[![Build Status](https://travis-ci.org/ubald/psychic-ui.svg?branch=master)](https://travis-ci.org/ubald/psychic-ui)


## What's with the name?

This is intended to be part of a future larger project so it is named according to that projet's name. I intend to make
sources available as much as possible for all of the project's parts in the form of libraries like this.
# Roadmap

https://trello.com/b/XYz6BS9v

# Building Skia

    brew install libjpeg-turbo icu4c
    
    git clone 'https://chromium.googlesource.com/chromium/tools/depot_tools.git'
    export PATH="${PWD}/depot_tools:${PATH}"
    git clone https://skia.googlesource.com/skia.git
    cd skia
    python tools/git-sync-deps
    bin/gn gen out/Static --args='is_official_build=true extra_cflags=["-I/usr/local/opt/icu4c/include", "-I/usr/local/opt/jpeg-turbo/include"] extra_ldflags=["-L/usr/local/opt/icu4c/lib", "-L/usr/local/opt/jpeg-turbo"]'
    ninja -C out/Static
