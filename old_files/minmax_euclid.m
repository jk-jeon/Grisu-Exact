function [ ret_min, ret_max ] = minmax_euclid( a, b, M )

ai = a;
bi = b;
si = sym(1);
ti = sym(0);
ui = sym(0);
vi = sym(1);

while true
%     while bi >= ai
%         bi = bi - ai;
%         ui = ui - si;
%         vi = vi - ti;
%         if -ui >= M
%             ret_min = ai;
%             ret_max = b - bi;
%             if ret_max >= b
%                 ret_max = ret_max - b;
%             end
%             return;
%         end
%     end
    if bi >= ai
        q = floor(bi / ai);
        if q * si >= M + ui
            q = ceil((M + ui) / si);
            ret_min = ai;
            ret_max = b - (bi - q * ai);
            if ret_max == b
                ret_max = b - 1;
            end
            return;
        end
        bi = bi - q * ai;
        ui = ui - q * si;
        vi = vi - q * ti;
    end
    if bi == 0
        ret_min = 1;
        ret_max = b - 1;
        return;
    end
    
%     while ai >= bi
%         ai = ai - bi;
%         si = si - ui;
%         ti = ti - vi;
%         if si >= M
%             ret_min = ai;
%             ret_max = b - bi;
%             if ret_max >= b
%                 ret_max = ret_max - b;
%             end
%             return;
%         end
%     end
    if ai >= bi
        q = floor(ai / bi);
        if M <= si - q * ui
            q = ceil((si - M) / ui);
            ret_min = ai - q * bi;
            ret_max = b - bi;
            if ret_min == 0
                ret_min = 1;
            end
            return;
        end
        ai = ai - q * bi;
        si = si - q * ui;
        ti = ti - q * vi;
    end
    if ai == 0
        ret_min = 1;
        ret_max = b - 1;
        return;
    end
end


end

