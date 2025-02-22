function str = to_str(name, W)
    str = sprintf('float %s[%d][%d] = %s;', name, size(W, 1), size(W, 2), mat2str(W));
end