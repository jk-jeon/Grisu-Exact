addpath('shaded_plots');
plot_uniform_benchmark('uniform_benchmark_binary32.csv', 32);
plot_uniform_benchmark('uniform_benchmark_binary64.csv', 64);
avg32 = plot_digit_benchmark('digits_benchmark_binary32.csv');
avg64 = plot_digit_benchmark('digits_benchmark_binary64.csv');