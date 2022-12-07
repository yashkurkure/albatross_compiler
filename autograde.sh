#!/bin/bash

timeout -k 150 150 ./runtests.sh | tee out.log || true
