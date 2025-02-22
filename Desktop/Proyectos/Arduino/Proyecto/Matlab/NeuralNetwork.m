classdef NeuralNetwork
    properties
        activation
        activation_prime
        weights
        deltas
    end
    
    methods
        function obj = NeuralNetwork(layers, activation)
            if nargin < 2
                activation = 'tanh';
            end
            
            if strcmp(activation, 'sigmoid')
                obj.activation = @sigmoid;
                obj.activation_prime = @sigmoid_derivada;
            elseif strcmp(activation, 'tanh')
                obj.activation = @my_tanh;  % Usar my_tanh en lugar de tanh
                obj.activation_prime = @my_tanh_derivada;  % Usar my_tanh_derivada
            end
            
            obj.weights = {};
            obj.deltas = {};
            
            for i = 1:length(layers) - 1
                r = 2 * rand(layers(i) + 1, layers(i+1) + 1) - 1;
                obj.weights{end+1} = r;
            end
            
            r = 2 * rand(layers(end) + 1, layers(end)) - 1;
            obj.weights{end+1} = r;
        end
        
        function obj = fit(obj, X, y, learning_rate, epochs)
            ones_col = ones(size(X, 1), 1);
            X = [ones_col, X];
            
            for k = 1:epochs
                i = randi(size(X, 1));
                a = {X(i, :)};
                
                for l = 1:length(obj.weights)
                    dot_value = a{l} * obj.weights{l};
                    activation = obj.activation(dot_value);
                    a{end+1} = activation;
                end
                
                error = y(i, :) - a{end};
                deltas = {error .* obj.activation_prime(a{end})};
                
                for l = length(a)-2:-1:1
                    deltas{end+1} = deltas{end} * obj.weights{l+1}' .* obj.activation_prime(a{l+1});
                end
                
                obj.deltas{end+1} = deltas;
                deltas = flip(deltas);
                
                for l = 1:length(obj.weights)
                    layer = a{l};
                    delta = deltas{l};
                    obj.weights{l} = obj.weights{l} + learning_rate * layer' * delta;
                end
                
                if mod(k, 10000) == 0
                    fprintf('epochs: %d\n', k);
                end
            end
        end
        
        function prediction = predict(obj, x)
            ones_col = ones(1, 1);
            a = [ones_col, x];
            
            for l = 1:length(obj.weights)
                a = obj.activation(a * obj.weights{l});
            end
            
            prediction = a;
        end
        
        function print_weights(obj)
            disp('LISTADO PESOS DE CONEXIONES');
            for i = 1:length(obj.weights)
                disp(obj.weights{i});
            end
        end
        
        function weights = get_weights(obj)
            weights = obj.weights;
        end
        
        function deltas = get_deltas(obj)
            deltas = obj.deltas;
        end
    end
end

% Funciones de activación
function y = sigmoid(x)
    y = 1.0 ./ (1.0 + exp(-x));
end

function y = sigmoid_derivada(x)
    y = sigmoid(x) .* (1.0 - sigmoid(x));
end

function y = my_tanh(x)  % Cambiado de tanh a my_tanh
    y = tanh(x);  % Usar la función tanh incorporada de MATLAB
end

function y = my_tanh_derivada(x)  % Cambiado de tanh_derivada a my_tanh_derivada
    y = 1.0 - x.^2;
end