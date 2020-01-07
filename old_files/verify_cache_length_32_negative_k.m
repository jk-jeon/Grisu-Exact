e = 3:96;
k = ceil(-(e+1) * log10(2));
u = zeros(1, length(e));
bits_required = zeros(1, length(e));
intmax = uint64(sym(2)^25 - 1);

for i=1:length(e)
    [m, M] = minmax_euclid_v2(sym(2)^(7 + e(i) + k(i)), sym(5)^-k(i), intmax);
    u(i) = e(i) + k(i) + 7 + eval(floor(log2(intmax * sym(5)^-k(i) / (sym(5)^-k(i) - M)))) + 1;
    
    edge_case_a = sym(2)^(6 + e(i) + k(i)) * intmax;
    edge_case_b = sym(5)^-k(i);
    edge_case_remainder = edge_case_a - floor(edge_case_a / edge_case_b) * edge_case_b;
    edge_case = e(i) + k(i) + 6 + eval(floor(log2(intmax * sym(5)^-k(i) / (sym(5)^-k(i) - edge_case_remainder)))) + 1;
    
    if edge_case > u(i)
        u(i) = edge_case;
    end
    
    bits_required(i) = u(i) + floor(k(i) * log2(5)) + 1;
end
plot(e, u, e, bits_required)

j = 0;
problematic = [];
for i=1:length(e)
    if bits_required(i) > 64
        j = j + 1;
        problematic(j) = i;
    end
end
e(problematic)