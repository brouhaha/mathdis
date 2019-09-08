# mathdis - disassembler for Atari Battlezone (etc.) math box microcode

Hosted at the
[mathdis Github repository](https://github.com/brouhaha/mathdis/).

## Introduction

mathdis is a program to disassemble the microcode ROMs of the "math box"
used by some Atari coin-operated video games, such as Red Baron,
Battlezone, and Tempest.

The output is not in a form intended to be reassembled. It is intended to
make sense to someone who understands the math box hardware.

The math box is a 16-bit processor using AMD Am2901 bit slice
components, and a very simple sequencer. The main processor of the
game, a 6502, can command the mathbox to execute one of 32 different
instructions, and can also provide an 8-bit operand. A dispatch PROM
maps the instruction code into an 8-bit starting address for the
microcode routine. The math box executes microinstructions until it
encounters a microinstruction with the halt bit set, at which time the
6502 can read a 16-bit result.
