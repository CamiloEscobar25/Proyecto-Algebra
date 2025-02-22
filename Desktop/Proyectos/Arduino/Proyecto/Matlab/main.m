% Red Coche para Evitar obstáculos
nn = NeuralNetwork([2, 3, 4], 'tanh');
X = [-1, 0;   % sin obstaculos
      -1, 1;   % sin obstaculos
      -1, -1;  % sin obstaculos
       0, -1;   % obstaculo detectado a derecha
       0, 1;     % obstaculo a izq
       0, 0;     % obstaculo centro
       1, 1;     % demasiado cerca a derecha
       1, -1;    % demasiado cerca a izq
       1, 0];     % demasiado cerca centro

% las salidas 'y' se corresponden con encender (o no) los motores
y = [1, 0, 0, 1; % avanzar
      1, 0, 0, 1; % avanzar
      1, 0, 0, 1; % avanzar
      0, 1, 0, 1; % giro derecha
      1, 0, 1, 0; % giro izquierda (cambie izq y derecha)
      1, 0, 0, 1; % avanzar
      0, 1, 1, 0; % retroceder
      0, 1, 1, 0; % retroceder
      0, 1, 1, 0]; % retroceder

nn = nn.fit(X, y, 0.03, 40001);

for index = 1:size(X, 1)
    e = X(index, :);
    prediccion = nn.predict(e);
    fprintf('X: [%d %d] esperado: [%d %d %d %d] obtenido: [%d %d %d %d]\n', ...
            e(1), e(2), y(index, 1), y(index, 2), y(index, 3), y(index, 4), ...
            valNN(prediccion(1)), valNN(prediccion(2)), valNN(prediccion(3)), valNN(prediccion(4)));
end

deltas = nn.get_deltas();
valores = [];
for i = 1:length(deltas)
    valores(end+1) = deltas{i}{2}(1) + deltas{i}{2}(2);
end

figure;
plot(1:length(valores), valores, 'b');
ylim([0 0.4]);
ylabel('Cost');
xlabel('Epochs');
title('Cost vs Epochs');
grid on;

% Obtenermos los pesos entrenados para poder usarlos en el codigo de arduino
pesos = nn.get_weights();

fprintf('// Reemplazar estas lineas en tu codigo arduino:\n');
fprintf('// float HiddenWeights ...\n');
fprintf('// float OutputWeights ...\n');
fprintf('// Con lo pesos entrenados.\n\n');
fprintf('%s\n', to_str('HiddenWeights', pesos{1}));
fprintf('%s\n', to_str('OutputWeights', pesos{2}));