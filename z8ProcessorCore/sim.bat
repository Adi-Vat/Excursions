@echo off
iverilog -g2012 -Wno-select-range -o sim instruction_set.sv arithmetic_logic_unit.sv control_unit.sv memory_manager.sv register_file.sv input_output_manager.sv z8ProcessorCore.sv tb.sv
if %errorlevel% equ 0 (
    vvp sim
)