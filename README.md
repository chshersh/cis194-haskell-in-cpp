# CIS 194 (2013): Haskell in C++

This repository contains the implementation of the
[CIS 194 (2013)](https://www.cis.upenn.edu/~cis1940/spring13/lectures.html)
course but in C++.

CIS 194 teaches Functional Programming in Haskell.

This repo demonstrates the same Functional Programming paradigms in C++.

It is (or was, depending on when you read this) streamed on Twitch:

- [Twitch @chshersh](https://www.twitch.tv/chshersh)

## For Devs

```shell
# Configure into ./build
$ cmake --preset=default

# Build everything
$ cmake --build --preset=default -- -s

# Build just Homework2
$ cmake --build --preset=default --target Homework2 -- -s
```
