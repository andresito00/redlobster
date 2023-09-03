#!/bin/sh
./build/simple_cross < actions.txt
./build/simple_cross < test/garbage_actions.txt
./build/test_kill_to_empty
./build/test_partial_fill
./build/test_full_fills_asc_desc
./build/test_full_fills_asc_asc
./build/test_find_oid_after_many_pushes
