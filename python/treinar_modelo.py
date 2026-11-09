
import pandas as pd
import joblib

from pathlib import Path
from sklearn.tree import DecisionTreeClassifier
from sklearn.model_selection import train_test_split
from sklearn.metrics import classification_report, accuracy_score

# ==========================================
# SMART BUS GATE - TREINAMENTO DA IA
# ==========================================

BASE_DIR = Path(__file__).resolve().parent
CAMINHO_MODELO = BASE_DIR / "modelo_smart_bus_gate.pkl"

COLUNAS = ["distancia_cm", "movimento", "tempo_parado"]


# 1. Regras usadas para gerar dados simulados
def definir_situacao(distancia, movimento, tempo):
    # Leitura invalida ou fora da faixa simulada
    if distancia <= 0 or distancia >= 400:
        return "ANOMALIA"

    # Veiculo proximo por 30 segundos ou mais
    if distancia <= 20 and tempo >= 30:
        return "POSSIVEL_OBSTRUCAO"

    # Veiculo proximo por menos de 30 segundos
    if distancia <= 20:
        return "APROXIMACAO"

    # Nenhum veiculo na zona de aproximacao
    return "AREA_LIVRE"


# 2. Geracao dos dados de treinamento
dados = []

distancias = (
    [-5, -1, 0]
    + list(range(1, 61))
    + [61, 80, 100, 120, 150, 200, 300, 399, 400, 450]
)

for distancia in distancias:
    for movimento in [0, 1]:
        for tempo in range(0, 66):
            situacao = definir_situacao(
                distancia, movimento, tempo
            )

            dados.append([
                distancia,
                movimento,
                tempo,
                situacao
            ])

df = pd.DataFrame(
    dados,
    columns=COLUNAS + ["situacao"]
)

print("=== BASE DE TREINAMENTO ===")
print("Total de registros:", len(df))
print(df["situacao"].value_counts())


# 3. Separacao entre treino e teste
X = df[COLUNAS]
y = df["situacao"]

X_treino, X_teste, y_treino, y_teste = train_test_split(
    X,
    y,
    test_size=0.25,
    random_state=42,
    stratify=y
)


# 4. Treinamento do modelo
modelo = DecisionTreeClassifier(
    random_state=42,
    max_depth=8
)

modelo.fit(X_treino, y_treino)


# 5. Avaliacao do modelo
previsoes = modelo.predict(X_teste)

print("\n=== AVALIACAO DO MODELO ===")
print("Acuracia:", round(accuracy_score(y_teste, previsoes), 4))

print(classification_report(
    y_teste,
    previsoes,
    zero_division=0
))


# 6. Testes especificos dos cenarios do projeto
cenarios = [
    (120, 0, 0, "AREA_LIVRE"),
    (13, 0, 0, "APROXIMACAO"),
    (13, 0, 16, "APROXIMACAO"),
    (13, 0, 24, "APROXIMACAO"),
    (13, 0, 29, "APROXIMACAO"),
    (13, 0, 30, "POSSIVEL_OBSTRUCAO"),
    (13, 0, 45, "POSSIVEL_OBSTRUCAO"),
    (15, 1, 60, "POSSIVEL_OBSTRUCAO"),
    (20, 1, 30, "POSSIVEL_OBSTRUCAO"),
    (0, 0, 0, "ANOMALIA"),
    (-1, 0, 0, "ANOMALIA"),
    (450, 1, 5, "ANOMALIA"),
]

print("\n=== TESTES DE VALIDACAO ===")

todos_aprovados = True

for distancia, movimento, tempo, esperado in cenarios:
    entrada = pd.DataFrame(
        [[distancia, movimento, tempo]],
        columns=COLUNAS
    )

    resultado = modelo.predict(entrada)[0]
    aprovado = resultado == esperado

    if not aprovado:
        todos_aprovados = False

    status = "OK" if aprovado else "FALHOU"

    print(
        f"[{status}] "
        f"Distancia={distancia} cm | "
        f"PIR={movimento} | "
        f"Tempo={tempo}s | "
        f"Previsto={resultado} | "
        f"Esperado={esperado}"
    )


# 7. Salvar somente se passar nos testes
if todos_aprovados:
    joblib.dump(modelo, CAMINHO_MODELO)

    print("\nMODELO VALIDADO!")
    print("Modelo salvo com sucesso!")
    print("Arquivo:", CAMINHO_MODELO)
else:
    print("\nATENCAO: modelo nao passou em todos os testes.")
    print("O modelo anterior nao foi substituido.")
