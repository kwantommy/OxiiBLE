clear; clc; close all; 
V_supply = 2.9;
Pmin = 0;
Pmax = 10;

P_applied = -6.890:0.05:6.89; %kPa

V_out = (0.8*V_supply)/(Pmax-Pmin) * (P_applied - Pmin) + 0.1*V_supply;

plot(P_applied,1.35+V_out, 'LineWidth', 5)
xlabel ('Pressure_a_p_p_l_i_e_d', 'FontSize', 20)
ylabel ('Output Voltage', 'FontSize', 20)
title ('Applied Voltage vs Pressure Applied','FontSize', 20)
axis tight;

