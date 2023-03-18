// A tri-state buffer is available as a primitive in verilog.
bufif1 bufInstance(bufOut, bufIn, enable);

// Wires
// -----
// * Make connections
// * Implement nets and nodes
// * Driven by signals
// * May not always have a value
//
// Registers
// ---------
// * Make connections
// * Can be assigned values
// * Hold values
// * Can drive wires
//
// Note registers != flip-flops

// Verilog has compiler directives includeing:
`define SIZE 4
`ifdef `SIZE
    // Include code
`endif
`include "ROM.v"
`timescale 1ns/10ps

// Display tasks can be used in simulation. $display runs once when called, monitor displayas variables when their
// value changes.
$display("fmt_str", arg1, arg2, arg3, ...);
$monitor("fmt_str", arg1, arg2, arg3, ...);