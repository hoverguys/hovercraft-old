#!/bin/sh
cppcheck --enable=all -inconclusive --std=posix -I include src/*
