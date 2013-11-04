clear all; close all;

prefix = 'delay';
methods = {'timer', 'realtime', 'cputime'};
experiments = [5000, 1000, 200, 100, 50, 10, 5, 3];

colors = 'brgkcmy';

figure;
hold on;

for i = 1:numel(methods)
    for j = 1:length(experiments)
        method = methods{i};
        experiment = experiments(j);
        filename = sprintf('data/%s_%s_%dms.txt',prefix,method,experiment);
        disp(filename);
        x = load(filename);
        % Do whatever you want with the data
        plot(x, colors(j));

    end    
end