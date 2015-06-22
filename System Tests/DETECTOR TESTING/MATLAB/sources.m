%% SOURCE ANALYSIS OF CESIUM IODIDE SCINTILLATORS
clear; close all; clc;
load sources.mat; len = length(background_spectrum);
%%
nfInd1 = 1; % noise floor index 1
figure(1); % cobalt-60 (peaks @ 1.17 MeV and 1.33 MeV)
filtered_spectrum1 = ...
    cobalt60_spectrum(nfInd1:len)-background_spectrum(nfInd1:len);
filtered_spectrum1(filtered_spectrum1 < 0) =0;
plot(filtered_spectrum1); xlim([0 255]); ylim([0 700]);
xlabel('Channel No.'); ylabel('Counts');
title('Proteus Inc. - CsI:Tl detector - ^{60}Cobalt (peaks @ 1.17 MeV and 1.33 MeV) ');


nfInd2 = 1; % noise floor index 2
figure(2); % cesium-137 (peak @ 0.662 MeV)
filtered_spectrum2 = ...
    cesium137_spectrum(nfInd2:len)-background_spectrum(nfInd2:len);
filtered_spectrum2(filtered_spectrum2 < 0) =0;
plot(filtered_spectrum2); xlim([0 255]); ylim([0 20000]);
xlabel('Channel No.'); ylabel('Counts');
title('Proteus Inc. - CsI:Tl detector - ^{137}Cesium (peak @ 0.662 MeV)');

%%
chNo = [42 75 86]'; Eval = [0.662 1.17 1.33]';
[coeffs,goodness] = polyfit(chNo,Eval,1);

figure(3);
plot(chNo,Eval,'ko',linspace(40,90)',coeffs(1)*linspace(40,90)'+coeffs(2));
ylabel('Energy [MeV]'); xlabel('Channel No.');
legend('measured peaks','linear fit','Location','NorthWest');
E = @(CHNO) coeffs(1)*CHNO + coeffs(2);
title_string = ['Energy = ' num2str(coeffs(1)) '*Channel No. + ' num2str(coeffs(2))];
title(title_string);

%% SOURCE ANALYSIS OF BC-412 PLASTIC SCINTILLATORS
clear; close all; clc;
load psources.mat; len = length(plastic_background_spectrum);
%%
nfInd1 = 1; % noise floor index 1
figure(1); % cobalt-60 (peaks @ 1.17 MeV and 1.33 MeV)
filtered_spectrum1 = ...
    plastic_cobalt60_spectrum(nfInd1:len)-plastic_background_spectrum(nfInd1:len);
filtered_spectrum1(filtered_spectrum1 < 0) =0;
plot(filtered_spectrum1); xlim([0 255]); ylim([0 700]);
xlabel('Channel No.'); ylabel('Counts');
title('Saint Gobain - BC-412 plastic detector - ^{60}Cobalt (peaks @ 1.17 MeV and 1.33 MeV) ');


nfInd2 = 1; % noise floor index 2
figure(2); % cesium-137 (peak @ 0.662 MeV)
filtered_spectrum2 = ...
    plastic_cesium137_spectrum(nfInd2:len)-plastic_background_spectrum(nfInd2:len);
filtered_spectrum2(filtered_spectrum2 < 0) =0;
plot(filtered_spectrum2); xlim([0 255]); ylim([0 20000]);
xlabel('Channel No.'); ylabel('Counts');
title('Saint Gobain - BC-412 plastic detector - ^{137}Cesium (peak @ 0.662 MeV)');

%%
chNo = [42 75 86]'; Eval = [0.662 1.17 1.33]';
[coeffs,goodness] = polyfit(chNo,Eval,1);

figure(3);
plot(chNo,Eval,'ko',linspace(40,90)',coeffs(1)*linspace(40,90)'+coeffs(2));
ylabel('Energy [MeV]'); xlabel('Channel No.');
legend('measured peaks','linear fit','Location','NorthWest');
E = @(CHNO) coeffs(1)*CHNO + coeffs(2);
title_string = ['Energy = ' num2str(coeffs(1)) '*Channel No. + ' num2str(coeffs(2))];
title(title_string);