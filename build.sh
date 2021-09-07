#!/bin/bash

mkdir -p msa &&
eosio-cpp -abigen src/q.msa.cpp -o msa/msa.wasm &&
echo "contract compiled to directory <msa>"