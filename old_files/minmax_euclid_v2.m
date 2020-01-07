function [ ret_min, ret_max ] = minmax_euclid_v2( a, b, M )

% a * si - b * ti = a
% b * vi - a * ui = b

ai = a;
bi = [b b];
si = sym(1);
ti = sym(0);
ui = [sym(0) sym(0)];
vi = sym(1);
i = 1;

while true
    if bi(i) >= ai(i)
        q = floor(bi(i) / ai(i));
        if ui(i) + q * si(i) >= M
            ret_min = ai(i);
            ret_max = b - bi(i);
            
            j = i;
            remaining = M - ui(i);
            while remaining > 0
                q = floor(remaining / si(j));
                remaining = remaining - q * si(j);
                if ret_max + q * ai(j) < b
                    ret_max = ret_max + q * ai(j);
                else
                    q = floor((b - ret_max) / ai(j));
                    ret_max = ret_max + q * ai(j);
                    return;
                end
                j = j - 1;
            end
            return;
        end
        bi(i + 1) = bi(i) - q * ai(i);
        ui(i + 1) = ui(i) + q * si(i);
        vi = vi + q * ti;
    end
    if bi(i + 1) == 0
        ret_min = 1;
        ret_max = b - 1;
        return;
    end
    
    if ai(i) >= bi(i + 1)
        q = floor(ai(i) / bi(i + 1));
        if si(i) + q * ui(i + 1) >= M
            ret_min = ai(i);
            ret_max = b - bi(i + 1);
            
            j = i;
            remaining = M - si(i);
            while remaining > 0
                q = floor(remaining / ui(j + 1));
                remaining = remaining - q * ui(j + 1);
                if ret_min > q * bi(j + 1)
                    ret_min = ret_min - q * bi(j + 1);
                else
                    q = floor(ret_min / bi(j + 1));
                    ret_min = ret_min - q * bi(j + 1);
                    return;
                end
                j = j - 1;
            end
            return;
        end
        ai(i + 1) = ai(i) - q * bi(i + 1);
        si(i + 1) = si(i) + q * ui(i + 1);
        ti = ti + q * vi;
    end
    if ai(i + 1) == 0
        ret_min = 1;
        ret_max = b - 1;
        return;
    end
    
    i = i + 1;
end

end

