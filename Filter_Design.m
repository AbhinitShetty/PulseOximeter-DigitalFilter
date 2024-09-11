% PuOxi Raw Data Simulator

f_resp = 0.11;   %Respiratory Frequency
RESP = 0.05;     
DC = 1.0;        %DC Offset (Mimics offset caused by bones and other muscle tissues)
f_ac = 80 / 60;  %Heartbeat Frequency
AC1 = 0.04;      %AC Component
AC2 = -0.02;
AC3 = 0.005;
NOISE = 0.01;    %Noise Constant
fs = 43.0;       %SAMPLING FREQUENCY
T = 1.0 / fs;    %SAMPLING TIME 

N = 10000;       %No. of Samples
k = (0:N-1)';

% Generating RAW Signal by merging signals with multiple frequencies and noise  
u_po = DC + RESP * sin(2.0*pi*f_resp*T*k);
u_po = u_po + AC1 * sin(2.0*pi*f_ac*T*k);
u_po = u_po + AC2 * sin(2.0*pi*2.0*f_ac*T*k);
u_po = u_po + AC3 * sin(2.0*pi*3.0*f_ac*T*k);
u_po = u_po + NOISE * randn(N, 1);
u_po = round(u_po*1200.0);
Pulse = [k*T, u_po];

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% LOWPASS & HIGHPASS IIR BUTTERWORTH Filter
Sample_Freq = 200;          %Sampling Frequency = 200Hz

%LOWPASS FILTER 
Fp_lpf = 8;                 %Passband Freq = 9Hz
Fs_lpf = 20;                %Stopband Freq = 20Hz
As_lpf = 20;                %Stopband Attenuation = 30dB
Rp_lpf = 0.3;               %Passband Ripple 

Wp_lpf = Fp_lpf/(Sample_Freq/3);   %Digital Passband Freq (w=f/fs) Range: 0-1
Ws_lpf = Fs_lpf/(Sample_Freq/3);   %Digital Stopband Freq (w=f/fs) Range: 0-1

[order_lpf, Wc_lpf] = buttord(Wp_lpf, Ws_lpf, Rp_lpf, As_lpf);	%Calculating Order of filter 
[b_lpf,a_lpf] = butter(order_lpf, Wc_lpf, 'low');				%Filter Coefficients - Lowpass Filter

% HIGHPASS FILTER
Fp_hpf = 0.5;              %Passband Freq = 0.5Hz
Fs_hpf = 0.15;             %Stopband Freq = 0.1Hz
As_hpf = 30;               %Stopband Attenuation = 40dB
Rp_hpf = 0.3;              %Passband Ripple 
Wp_hpf = Fp_hpf/(Sample_Freq/4);    %Digital Passband Freq : Range: 0-1
Ws_hpf = Fs_hpf/(Sample_Freq/4);    %Digital Stopband Freq : Range: 0-1

[order_hpf, Wc_hpf] = buttord(Wp_hpf, Ws_hpf, Rp_hpf, As_hpf);	%Calculating Order of filter 
[b_hpf, a_hpf] = butter(order_hpf, Wc_hpf, 'high');             %Filter Coefficients - Highpass Filter


%FREQUENCY RESPONSE VISUALIZATION
fvtool(b_lpf,a_lpf,b_hpf,a_hpf);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% GRAPH PLOTS  

% RAW Signal Plot
plot(k*T, u_po, "linewidth", 1);     %X-axis range: 0-233 | Y-axis range: 0-1500
hold on;

% GRAPH Details
title('Pulse Oximeter raw data simulator (80 beats/min)');
axis([0 10 0 1500]);
grid;
hold on;
disp('That`s all, folks.');

% Implementing Lowpass filter on Raw signal
output_lpf = filter(b_lpf, a_lpf, u_po);
plot(k*T, output_lpf, "linewidth", 1);
hold on;

% Implementing Highpass filter on Lowpass filter Output
output_hpf = filter(b_hpf, a_hpf, output_lpf);
plot(k*T, output_hpf, "linewidth", 1);
xlabel('Time');
ylabel('Amplitude');
legend('Raw Signal','LPF Output', 'HPF Output');
