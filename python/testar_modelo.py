import joblib
import pandas as pd

# Carregar o modelo treinado
modelo = joblib.load("modelo_smart_bus_gate.pkl")

# Novas situações simuladas
testes = pd.DataFrame([
    [180, 0, 0],
    [14, 1, 3],
    [12, 1, 50],
    [-1, 0, 0]
], columns=[
    "distancia_cm",
    "movimento",
    "tempo_parado"
])

# Realizar previsões
previsoes = modelo.predict(testes)

print("=== TESTE DA IA SMART BUS GATE ===")

for i, previsao in enumerate(previsoes):
    print(
        f"Teste {i + 1}: "
        f"Distância={testes.iloc[i]['distancia_cm']} cm | "
        f"Movimento={testes.iloc[i]['movimento']} | "
        f"Tempo={testes.iloc[i]['tempo_parado']} s "
        f"=> {previsao}"
    )
