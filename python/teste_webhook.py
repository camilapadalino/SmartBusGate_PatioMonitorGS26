
import requests

url = "https://globalsolutionrpa26.app.n8n.cloud/webhook/smart-bus-gate"

dados = {
    "dispositivo": "SMART_BUS_GATE_01",
    "evento": "POSSIVEL_OBSTRUCAO",
    "distancia_cm": 13.12,
    "tempo_parado": 30,
    "movimento": False,
    "cancela": "ABERTA"
}

try:
    resposta = requests.post(
        url,
        json=dados,
        timeout=15
    )

    print("HTTP Status:", resposta.status_code)
    print("Resposta:", resposta.text)

except requests.exceptions.RequestException as erro:
    print("Erro de conexao:", erro)
