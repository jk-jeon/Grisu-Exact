e = 2:-1:-157;
k = ceil(-(e+1) * log10(2));
l = zeros(1, length(e));
bits_required = zeros(1, length(e));
intmax = uint64(sym(2)^25 - 1);

for i=1:13
    l(i) = 0;
    bits_required(i) = floor(k(i) * log2(5)) + 1;
end

for i=14:length(e)
    m = minmax_euclid_v2(sym(5)^k(i), sym(2)^(-e(i) - k(i) - 7), intmax);
    l(i) = eval(floor(log2(m / sym(intmax))));
    
    edge_case_a = sym(5)^k(i) * intmax;
    edge_case_b = sym(2)^(-e(i) - k(i) - 6);
    edge_case_remainder = edge_case_a - floor(edge_case_a / edge_case_b) * edge_case_b;
    edge_case = eval(floor(log2(edge_case_remainder / intmax)));
    
    if edge_case < l(i)
        l(i) = edge_case;
    end
    
    if l(i) < 0
        l(i) = 0;
    end
    
    bits_required(i) = floor(k(i) * log2(5)) + 1 - l(i);
end
plot(e, l, e, bits_required);

j = 0;
problematic = [];
for i=1:length(e)
    if bits_required(i) > 64
        j = j + 1;
        problematic(j) = i;
    end
end
e(problematic)