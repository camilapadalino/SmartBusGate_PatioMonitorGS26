
import os
import hmac
from pathlib import Path
from datetime import datetime, timezone
from collections import deque
from threading import Lock

import joblib
import pandas as pd

from fastapi import FastAPI, Header, HTTPException, Depends
from pydantic import BaseModel, Field
from dotenv import load_dotenv

# ==============================
# CONFIGURACOES
# ==============================

BASE_DIR = Path(__file__).resolve().parent

load_dotenv(BASE_DIR / ".env")

API_KEY = os.getenv("SMART_BUS_API_KEY")

if not API_KEY:
    raise RuntimeError(
        "SMART_BUS_API_KEY nao configurada no .env"
    )

app = FastAPI(
    title="Smart Bus Gate - API de IA",
    description="Monitoramento inteligente da cancela",
    version="1.2.0"
)

# ==============================
# MODELO DE MACHINE LEARNING
# ==============================

CAMINHO_MODELO = (
    BASE_DIR / "modelo_smart_bus_gate.pkl"
)

modelo = joblib.load(CAMINHO_MODELO)

# ==============================
# HISTORICO EM MEMORIA
# ==============================

historico = deque(maxlen=100)
historico_lock = Lock()

# ==============================
# AUTENTICACAO
# ==============================

def verificar_chave(
    x_api_key: str = Header(default="")
):
    if not hmac.compare_digest(x_api_key, API_KEY):
        raise HTTPException(
            status_code=401,
            detail="Chave de API invalida ou ausente"
        )

# ==============================
# DADOS DOS SENSORES
# ==============================

class DadosSensor(BaseModel):
    distancia_cm: float = Field(
        description="Distancia do ultrassonico"
    )

    movimento: bool = Field(
        description="Movimento detectado pelo PIR"
    )

    tempo_parado: int = Field(
        ge=0,
        description="Tempo de permanencia em segundos"
    )

# ==============================
# ENDPOINT INICIAL
# ==============================

@app.get("/")
def inicio():
    return {
        "sistema": "Smart Bus Gate",
        "status": "API funcionando",
        "modelo": "DecisionTreeClassifier",
        "versao": "1.2.0"
    }

# ==============================
# PREVISAO DA IA
# ==============================

@app.post("/prever")
def prever(
    dados: DadosSensor,
    _: None = Depends(verificar_chave)
):

    entrada = pd.DataFrame([{
        "distancia_cm": dados.distancia_cm,
        "movimento": int(dados.movimento),
        "tempo_parado": dados.tempo_parado
    }])

    classificacao = str(modelo.predict(entrada)[0])

    resultado = {
        "dispositivo": "SMART_BUS_GATE_01",
        "classificacao": classificacao,
        "distancia_cm": dados.distancia_cm,
        "movimento": dados.movimento,
        "tempo_parado": dados.tempo_parado
    }

    registro = {
        "data_hora": datetime.now(
            timezone.utc
        ).isoformat(),
        **resultado
    }

    with historico_lock:
        historico.append(registro)

    print(
        "[IA] Classificacao:",
        classificacao,
        "| Distancia:",
        dados.distancia_cm,
        "| Tempo:",
        dados.tempo_parado
    )

    return resultado

# ==============================
# CONSULTA AO HISTORICO
# ==============================

@app.get("/historico")
def consultar_historico(
    _: None = Depends(verificar_chave)
):

    with historico_lock:
        registros = list(reversed(historico))

    return {
        "total_registros": len(registros),
        "historico": registros
    }

# ==============================
# ULTIMA CLASSIFICACAO
# ==============================

@app.get("/ultima-classificacao")
def ultima_classificacao(
    _: None = Depends(verificar_chave)
):

    with historico_lock:
        ultimo = historico[-1] if historico else None

    if ultimo is None:
        return {
            "mensagem": "Nenhuma classificacao registrada"
        }

    return ultimo
