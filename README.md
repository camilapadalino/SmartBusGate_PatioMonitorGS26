
# 🚌 Smart Bus Gate | Pátio Monitor

### 👩‍💻 Integrantes

**Global Solution 2026 — 2º semestre | Engenharia de Software — FIAP**

- **Camila do Prado Padalino** — RM 98316
- **Felipe Cavalcante Bressane** — RM 97688
- **Gabriel Teixeira Machado** — RM 551570

---

## 🌟 Sobre o projeto

Imagine um terminal de ônibus onde os operadores precisam acompanhar constantemente a entrada, a saída e a permanência dos veículos.

Quando essas informações não estão centralizadas, o controle operacional se torna mais difícil, podendo gerar atrasos, falhas de comunicação e dificuldades para identificar situações inesperadas.

Foi pensando nesse problema que surgiu o **Pátio Monitor**! 🚌📍

Nossa proposta é desenvolver uma plataforma de monitoramento inteligente para pátios, garagens e terminais, reunindo informações operacionais em um único ambiente.

Como parte dessa solução, desenvolvemos o **Smart Bus Gate**, um protótipo de cancela inteligente, simulado no Wokwi, que combina sensores, ESP32, automação, Machine Learning e registro de eventos.

> 💡 **Nossa missão:** transformar dados capturados por sensores em informações úteis para uma operação mais inteligente, segura e organizada.

---

## 🎯 Objetivos

O Smart Bus Gate foi desenvolvido para:

- 🚍 Detectar a aproximação de veículos.
- 📏 Monitorar distâncias utilizando sensores.
- 🚦 Controlar automaticamente uma cancela.
- 👀 Identificar presença ou movimento.
- 🧠 Integrar sensores a um modelo de Inteligência Artificial.
- 🌐 Enviar informações por uma API REST.
- ⚙️ Automatizar registros e alertas utilizando n8n.
- 📊 Contribuir para o monitoramento operacional do Pátio Monitor.
- 🛡️ Manter as decisões locais de controle mesmo quando serviços externos estiverem indisponíveis.

---

## 🧩 Tecnologias utilizadas

| Tecnologia | Aplicação |
|---|---|
| 🟦 ESP32 | Controle dos sensores e atuadores |
| 🔌 Wokwi | Simulação do circuito |
| 📏 HC-SR04 | Medição de distância |
| 👀 PIR | Detecção de movimento |
| ⚙️ Servomotor | Movimentação da cancela |
| 🚦 LEDs | Sinalização operacional |
| 🐍 Python | Desenvolvimento da IA e da API |
| 🧠 Scikit-learn | Modelo de Machine Learning |
| 📊 Pandas | Preparação dos dados |
| 📦 Joblib | Salvamento e carregamento do modelo |
| 🚀 FastAPI | Comunicação entre ESP32 e IA |
| ☁️ Cloudflare Tunnel | Acesso HTTPS temporário à API |
| 🔄 n8n | Automação, auditoria e alertas |
| 📂 GitHub | Versionamento e documentação |

---

## 🔧 Componentes eletrônicos

Para a montagem do protótipo, utilizamos:

- 1 × ESP32 DevKit
- 1 × Sensor ultrassônico HC-SR04
- 1 × Sensor de movimento PIR
- 1 × Servomotor
- 1 × LED verde
- 1 × LED vermelho
- Resistores e conexões para o circuito

### 📌 Pinagem utilizada

| Componente | Pino ESP32 |
|---|---|
| HC-SR04 — TRIG | GPIO 5 |
| HC-SR04 — ECHO | GPIO 18 |
| PIR — OUT | GPIO 25 |
| Servomotor — sinal | GPIO 19 |
| LED verde | GPIO 26 |
| LED vermelho | GPIO 27 |

> 🛠️ O circuito foi desenvolvido e simulado no Wokwi. A montagem pode ser reproduzida utilizando os arquivos `sketch.ino` e `diagram.json`.

---

## 🚦 Como funciona nossa cancela inteligente?

O ESP32 monitora continuamente os sensores e executa regras locais de controle.

### 🟢 1. Aproximação do veículo

Quando o sensor ultrassônico identifica uma distância menor ou igual a **20 cm**:

- O sistema reconhece a proximidade.
- O servomotor abre a cancela.
- O estado operacional é sinalizado.
- Um evento `CANCELA_ABERTA` pode ser enviado ao n8n.

### 🔴 2. Saída do veículo

Quando a distância ultrapassa **30 cm** e o PIR não detecta movimento durante **3 segundos**:

- O sistema interpreta que a área pode estar livre.
- A cancela é fechada.
- O evento `CANCELA_FECHADA` é gerado.

### ⚠️ 3. Possível obstrução

Se a condição de proximidade permanecer por pelo menos **30 segundos**:

- O sistema identifica uma possível obstrução.
- Um alerta `POSSIVEL_OBSTRUCAO` pode ser enviado ao n8n.
- O evento pode ser disponibilizado para auditoria.

### 🛡️ 4. Funcionamento local

O controle básico da cancela utiliza regras executadas diretamente no ESP32.

Isso permite manter a lógica local de abertura e fechamento mesmo quando a comunicação com a IA ou com o n8n estiver indisponível.

A IA complementa o monitoramento, mas não substitui as regras locais de controle.

---

## 🧠 Inteligência Artificial: Machine Learning supervisionado

Uma das principais funcionalidades do Smart Bus Gate é sua integração com um modelo de **Machine Learning supervisionado**.

Utilizamos o algoritmo `DecisionTreeClassifier`, da biblioteca Scikit-learn, para classificar situações operacionais a partir dos dados recebidos dos sensores.

O modelo é treinado em Python utilizando dados sintéticos e disponibilizado por meio de uma API FastAPI.

### 📥 Dados utilizados pelo modelo

| Variável | Descrição |
|---|---|
| `distancia_cm` | Distância medida pelo ultrassônico |
| `movimento` | Informação do sensor PIR |
| `tempo_parado` | Tempo de permanência identificado |

### 🏷️ Classificações do modelo

| Classificação | Descrição |
|---|---|
| `AREA_LIVRE` | Área sem veículo na zona de aproximação |
| `APROXIMACAO` | Veículo detectado a até 20 cm por menos de 30 segundos |
| `POSSIVEL_OBSTRUCAO` | Veículo detectado a até 20 cm por 30 segundos ou mais |
| `ANOMALIA` | Distância inválida ou fora da faixa simulada |

As regras de geração dos dados sintéticos são:

- Distância menor ou igual a 0 cm ou maior ou igual a 400 cm: `ANOMALIA`.
- Distância válida de até 20 cm e tempo maior ou igual a 30 segundos: `POSSIVEL_OBSTRUCAO`.
- Distância válida de até 20 cm e tempo inferior a 30 segundos: `APROXIMACAO`.
- Demais distâncias válidas: `AREA_LIVRE`.

> 📚 O PIR é uma das entradas do modelo, mas as regras utilizadas para gerar os rótulos sintéticos dependem de distância e tempo. A avaliação com esses dados não equivale à validação em condições reais de um terminal.

### 🔄 Fluxo da IA

```text
📏 Sensor ultrassônico + 👀 Sensor PIR
                  |
                  v
              🟦 ESP32
                  |
                  | JSON via HTTPS
                  v
          ☁️ Cloudflare Tunnel
                  |
                  v
            🚀 API FastAPI
                  |
                  v
        🧠 DecisionTreeClassifier
                  |
                  | Classificação
                  v
              🟦 ESP32
```

### 📤 Exemplo real de envio

Durante nossos testes, o ESP32 enviou:

```json
{
  "distancia_cm": 13.11975,
  "movimento": false,
  "tempo_parado": 0
}
```

### ✅ Resultado obtido

```text
HTTP IA: 200
CLASSIFICACAO DA IA: APROXIMACAO
```

🎉 **Integração ESP32 + IA validada com sucesso em 09/11/2026!**

O código HTTP `200` confirmou que a requisição foi processada e que o ESP32 recebeu a classificação `APROXIMACAO`.

---

## 🌐 Integração com FastAPI

A FastAPI é responsável por disponibilizar o modelo de Machine Learning para consulta.

O ESP32 envia os dados dos sensores à rota:

```http
POST /prever
```

A API recebe os dados, consulta o modelo treinado e devolve a classificação correspondente.

### 🔌 Endpoints disponíveis

| Método | Endpoint | Função | Autenticação |
|---|---|---|---|
| GET | `/` | Verificar funcionamento da API | Não |
| POST | `/prever` | Classificar dados dos sensores | Sim |
| GET | `/historico` | Consultar até 100 classificações recentes | Sim |
| GET | `/ultima-classificacao` | Consultar a classificação mais recente | Sim |

### 🔐 Autenticação

A comunicação utiliza uma chave de API enviada pelo cabeçalho HTTP:

```http
x-api-key: SUA_CHAVE_PRIVADA
```

A API verifica essa chave utilizando a variável de ambiente `SMART_BUS_API_KEY`.

As credenciais devem permanecer em arquivos locais, sem publicação no GitHub.

### 🗂️ Histórico das classificações

A API armazena temporariamente as últimas 100 classificações, incluindo a data e hora em UTC.

Os registros podem ser consultados pelas rotas `/historico` e `/ultima-classificacao`.

> ⚠️ O histórico é armazenado em memória e será perdido quando a aplicação for reiniciada.

### ☁️ Cloudflare Tunnel

Para permitir que o ESP32 simulado acesse a API executada localmente, utilizamos o Cloudflare Tunnel.

O serviço disponibiliza uma URL HTTPS pública e temporária, direcionada à FastAPI em execução no computador.

Como essa URL pode mudar, é necessário atualizar a configuração do ESP32 ao reiniciar o túnel.

---

## 🔗 Integração Interdisciplinar — Maker Lab + RPA

Um dos diferenciais do **Smart Bus Gate** é a integração entre as disciplinas de **Maker Lab e Robótica** e **Robotic Process Automation (RPA)**, desenvolvidas para a Global Solution da FIAP. 🚌💚

Em vez de desenvolver soluções independentes, decidimos conectar as duas entregas para demonstrar como diferentes tecnologias podem trabalhar juntas dentro do ecossistema do **Pátio Monitor**.

### 🤝 Por que integramos as duas disciplinas?

O **Maker Lab** é responsável pela parte física e inteligente da solução, envolvendo sensores, ESP32, controle automatizado da cancela e comunicação com o modelo de Machine Learning.

Já a disciplina de **RPA** complementa esse funcionamento ao receber os eventos gerados pelo protótipo e automatizar seus registros e alertas por meio do n8n.

Dessa forma, conseguimos demonstrar um fluxo integrado:

**📏 Sensores → 🟦 ESP32 → 🔄 n8n → 📊 Registros operacionais**

Enquanto o **Maker Lab identifica situações e executa ações**, o **RPA organiza e registra os acontecimentos**, contribuindo para a rastreabilidade das operações.

### ✅ Resultado da integração

Durante nossos testes, conseguimos enviar eventos do ESP32 simulado ao n8n e confirmar seu registro na tabela de auditoria.

Essa integração demonstra como o Smart Bus Gate pode contribuir para o Pátio Monitor, conectando a automação física ao processamento de informações operacionais.

> 💡 **Nosso objetivo foi ir além de uma cancela automatizada, criando um protótipo capaz de gerar informações úteis para uma plataforma de monitoramento inteligente.**

---

## 🏗️ Arquitetura geral

```text
                  🚌 SMART BUS GATE
                        |
                   📏 Sensores IoT
                        |
                        v
                      🟦 ESP32
                        |
            +-----------+-----------+
            |                       |
            v                       v
     ☁️ Cloudflare             🔄 n8n / RPA
            |                       |
            v                       v
       🚀 FastAPI              Auditoria e alertas
            |                       |
            v                       v
       🧠 Modelo IA               Data Tables
            |                       |
            v                       v
     Classificação ao ESP32    Registros operacionais
                                    |
                                    v
                           🌐 Portal Pátio Monitor
                             (integração futura)
```

O Smart Bus Gate representa uma fonte complementar de eventos para o Pátio Monitor.

O portal principal também prevê informações operacionais e visualização 3D dos veículos.

A integração direta do protótipo com o portal ainda é uma etapa futura.

---

## 💻 Como executar o projeto

Nesta seção, explicamos como preparar o ambiente, treinar o modelo de Inteligência Artificial, iniciar a API e executar o Smart Bus Gate no Wokwi.

As instruções consideram o sistema Windows com PowerShell.

### 📋 1. Pré-requisitos

Antes de começar, tenha instalado ou disponível:

- Python
- Visual Studio Code ou editor equivalente
- Git
- Cloudflared
- Navegador com acesso ao Wokwi
- Arquivos do projeto disponíveis no repositório

Para reproduzir a integração com RPA, também é necessário configurar um webhook compatível no n8n.

### 📥 2. Clonar o repositório

Abra o terminal e execute:

```powershell
git clone URL_DO_REPOSITORIO
cd Smart-Bus-Gate
```

Substitua `URL_DO_REPOSITORIO` pelo endereço real do GitHub.

Caso o nome da pasta seja diferente, utilize o nome criado durante a clonagem.

### 🐍 3. Preparar o ambiente virtual

Dentro da pasta do projeto, execute:

```powershell
python -m venv .venv
```

Em seguida, ative o ambiente:

```powershell
.\.venv\Scripts\Activate.ps1
```

Se a ativação funcionar, o terminal apresentará algo semelhante a:

```text
(.venv) PS C:\Smart-Bus-Gate>
```

> ⚠️ Se o PowerShell bloquear a ativação, utilize o Prompt de Comando (CMD) e execute `.venv\Scripts\activate.bat`.

### 📦 4. Instalar as dependências

O projeto utiliza bibliotecas Python para executar a API, treinar o modelo e processar os dados.

Crie um arquivo chamado `requirements.txt` na pasta principal do projeto com o seguinte conteúdo:

```text
fastapi
uvicorn
scikit-learn
pandas
joblib
python-dotenv
```

Com o ambiente virtual ativado, execute:

```powershell
python -m pip install -r requirements.txt
```

Esse comando instalará automaticamente as dependências necessárias.

> 📌 Para maior reprodutibilidade, recomendamos fixar as versões das bibliotecas após conferir as versões utilizadas nos testes.

### 🧠 5. Treinar o modelo de Machine Learning

O treinamento é realizado pelo arquivo:

```text
treinar_modelo.py
```

O script utiliza dados sintéticos que representam situações de aproximação, área livre, possível obstrução e anomalia.

#### 🏋️ Como executar

Abra o terminal na pasta onde está `treinar_modelo.py` e execute:

```powershell
python treinar_modelo.py
```

O script realiza automaticamente:

1. Geração dos dados sintéticos.
2. Separação dos dados em 75% para treinamento e 25% para teste.
3. Treinamento do algoritmo `DecisionTreeClassifier`.
4. Avaliação utilizando acurácia e relatório de classificação.
5. Execução de 12 cenários específicos de validação.
6. Salvamento do modelo, caso todos os cenários sejam aprovados.

O treinamento utiliza os parâmetros:

```python
DecisionTreeClassifier(
    random_state=42,
    max_depth=8
)
```

#### 💾 Arquivo gerado

Quando todos os testes são aprovados, o script salva:

```text
modelo_smart_bus_gate.pkl
```

O arquivo é criado na mesma pasta de `treinar_modelo.py`.

O terminal deverá apresentar mensagens semelhantes a:

```text
=== AVALIACAO DO MODELO ===
Acuracia: ...

=== TESTES DE VALIDACAO ===
[OK] Distancia=120 cm | PIR=0 | Tempo=0s ...
[OK] Distancia=13 cm | PIR=0 | Tempo=0s ...

MODELO VALIDADO!
Modelo salvo com sucesso!
Arquivo: ...
```

> ⚠️ Se algum dos 12 cenários falhar, o script não salva nem substitui o modelo anterior.

O arquivo `modelo_smart_bus_gate.pkl` deve permanecer na mesma pasta de `api_ia.py`, pois é nesse caminho que a API procura o modelo.

### 🔐 6. Configurar a autenticação da API

O projeto utiliza uma chave privada para autenticar as requisições.

Crie um arquivo chamado `.env` na mesma pasta de `api_ia.py` e adicione:

```env
SMART_BUS_API_KEY=SUA_CHAVE_PRIVADA_AQUI
```

Substitua `SUA_CHAVE_PRIVADA_AQUI` por uma chave de sua escolha.

O ESP32 deverá enviar exatamente a mesma chave no cabeçalho:

```http
x-api-key: SUA_CHAVE_PRIVADA_AQUI
```

> 🔒 Nunca publique o arquivo `.env` ou as credenciais privadas do ESP32 no GitHub. Utilize arquivos de exemplo sem chaves reais para facilitar a reprodução.

### 🚀 7. Iniciar a FastAPI

Abra o terminal na pasta onde está `api_ia.py` e execute:

```powershell
python -m uvicorn api_ia:app --host 127.0.0.1 --port 8000
```

Mantenha esse terminal aberto durante a execução do projeto.

Se a API iniciar corretamente, aparecerá uma mensagem semelhante a:

```text
Uvicorn running on http://127.0.0.1:8000
Application startup complete.
```

#### 🌐 Verificando o funcionamento

Abra no navegador:

http://127.0.0.1:8000/

A API deverá retornar:

```json
{
  "sistema": "Smart Bus Gate",
  "status": "API funcionando",
  "modelo": "DecisionTreeClassifier",
  "versao": "1.2.0"
}
```

Também é possível acessar a documentação interativa:

http://127.0.0.1:8000/docs

> ℹ️ A versão `1.2.0` corresponde à versão declarada da nossa aplicação, e não à versão da biblioteca FastAPI.

### 🧪 8. Testar a classificação localmente

Antes de integrar o ESP32, podemos verificar se a API está classificando corretamente.

Abra outro terminal PowerShell e execute:

```powershell
$headers = @{
    "x-api-key" = "SUA_CHAVE_PRIVADA"
}

$body = @{
    distancia_cm = 13.11975
    movimento = $false
    tempo_parado = 0
} | ConvertTo-Json

Invoke-RestMethod `
    -Uri "http://127.0.0.1:8000/prever" `
    -Method Post `
    -Headers $headers `
    -ContentType "application/json" `
    -Body $body
```

Substitua `SUA_CHAVE_PRIVADA` pela chave configurada no `.env`.

A resposta deverá conter:

```json
{
  "dispositivo": "SMART_BUS_GATE_01",
  "classificacao": "APROXIMACAO",
  "distancia_cm": 13.11975,
  "movimento": false,
  "tempo_parado": 0
}
```

Essa classificação foi observada nos testes realizados pelo grupo.

### ☁️ 9. Iniciar o Cloudflare Tunnel

Com a FastAPI ainda em execução, abra outro terminal e execute:

```powershell
cloudflared tunnel --url http://127.0.0.1:8000
```

O Cloudflare disponibilizará um endereço HTTPS temporário, semelhante a:

```text
https://SEU-TUNEL.trycloudflare.com
```

Para acessar a classificação da IA, utilize a URL completa:

```text
https://SEU-TUNEL.trycloudflare.com/prever
```

O endereço pode mudar quando o túnel for reiniciado.

> 📌 Mantenha os terminais da FastAPI e do Cloudflare abertos enquanto o ESP32 estiver consultando a API.

### 🔌 10. Configurar e executar o Wokwi

1. Abra o projeto no Wokwi.
2. Carregue `sketch.ino` e `diagram.json`, caso esteja reproduzindo a partir do repositório.
3. Atualize no código do ESP32 a URL HTTPS atual do Cloudflare, incluindo `/prever`.
4. Configure no ESP32 a mesma chave utilizada em `SMART_BUS_API_KEY`.
5. Configure um webhook próprio do n8n, caso deseje reproduzir a auditoria.
6. Inicie a simulação.
7. Altere a distância do sensor HC-SR04 e o estado do PIR.
8. Observe os resultados no Serial Monitor.

🔗 **Projeto Wokwi:**

https://wokwi.com/projects/480155608742885377

> ⚠️ Os nomes exatos das variáveis e arquivos de configuração do ESP32 devem ser conferidos no `sketch.ino` antes da publicação definitiva.

### ✅ 11. Verificar os resultados esperados

| Situação simulada | Comportamento esperado |
|---|---|
| Distância ≤ 20 cm | Cancela abre |
| Distância > 30 cm e PIR sem movimento por 3 segundos | Cancela fecha |
| Proximidade por pelo menos 30 segundos | Possível obstrução |
| Requisição válida para `/prever` | HTTP 200 e classificação |
| Chave incorreta ou ausente | HTTP 401 |
| API ou túnel indisponível | Controle local da cancela permanece independente |

#### 📟 Exemplo do Serial Monitor

Em nosso teste de integração, obtivemos:

```text
HTTP IA: 200
CLASSIFICACAO DA IA: APROXIMACAO
```

Isso confirma que o ESP32 conseguiu enviar dados à API e receber uma classificação produzida pelo modelo.

### 🛠️ 12. Solução de problemas comuns

| Problema | O que verificar |
|---|---|
| `SMART_BUS_API_KEY nao configurada no .env` | Criar o `.env` na mesma pasta de `api_ia.py` |
| `FileNotFoundError` para o modelo | Executar `treinar_modelo.py` e verificar o arquivo `.pkl` |
| `ModuleNotFoundError` | Ativar o ambiente virtual e instalar as dependências |
| HTTP 401 | Conferir a chave enviada em `x-api-key` |
| API não responde | Verificar se o Uvicorn está em execução |
| Falha HTTPS no Wokwi | Conferir a URL e o Cloudflare Tunnel |
| Evento não aparece no n8n | Verificar o webhook e a configuração do workflow |

---

## 🧪 Testes e validações

### ✅ Testes de integração realizados

| Teste | Resultado |
|---|---|
| Acesso à FastAPI pelo Cloudflare | ✅ Validado |
| Envio de dados do ESP32 à IA | ✅ HTTP 200 |
| Classificação pela árvore de decisão | ✅ `APROXIMACAO` |
| Comunicação ESP32 → n8n | ✅ Validada |
| Registro na Data Table de auditoria | ✅ Confirmado |
| Processamento em lote no n8n | ✅ Testado separadamente |
| Tratamento de falhas HTTP no n8n | ✅ Testado separadamente |
| Integração com portal web | ⏳ Pendente |

### 📸 Evidências recomendadas

Para demonstrar o funcionamento do projeto, recomendamos apresentar:

- Captura do circuito no Wokwi.
- Simulação da abertura e do fechamento da cancela.
- Serial Monitor exibindo HTTP 200 e classificação da IA.
- Terminal da FastAPI recebendo requisições.
- Registro de auditoria no n8n.
- Demonstração da situação de possível obstrução.

---

## 🔒 Segurança e boas práticas

Durante o desenvolvimento, adotamos uma separação entre o código-fonte e as credenciais de autenticação.

Para manter essa organização:

- Não publique arquivos `.env` com chaves reais.
- Não publique `secrets.h` com credenciais privadas.
- Utilize arquivos de exemplo para facilitar a configuração.
- Não trate o Cloudflare Tunnel temporário como uma implantação de produção.
- Mantenha a lógica básica de controle da cancela no ESP32.
- Utilize arquivos de modelo `.pkl` somente de fontes confiáveis.

### 📂 Exemplo de `.gitignore`

```gitignore
.venv/
__pycache__/
.env
secrets.h
*.pyc
```

> 📌 O arquivo `modelo_smart_bus_gate.pkl` pode ser reproduzido pelo script de treinamento. Caso seja incluído no repositório, deve corresponder ao modelo validado pela equipe.

---

## 🌱 Impacto e evolução

O Smart Bus Gate contribui para demonstrar como a tecnologia pode apoiar a modernização de operações de transporte.

O projeto está relacionado principalmente aos seguintes Objetivos de Desenvolvimento Sustentável:

- 🏗️ **ODS 9 — Indústria, Inovação e Infraestrutura**
- 🏙️ **ODS 11 — Cidades e Comunidades Sustentáveis**

### 🚀 Próximos passos

Como evolução da solução, pretendemos:

- Ampliar os testes dos sensores e da cancela.
- Aprimorar o modelo utilizando dados representativos de situações reais.
- Expandir os registros operacionais.
- Integrar os eventos ao portal Pátio Monitor.
- Desenvolver novas funcionalidades de monitoramento e visualização.

---

## 🎥 Demonstração

📹 **Vídeo demonstrativo:** 

🔗 **Projeto Wokwi:** https://wokwi.com/projects/480155608742885377

🌐 **Portal Pátio Monitor:** 

---

## 💚 Considerações finais

O **Smart Bus Gate** demonstra a integração entre **IoT, Inteligência Artificial, APIs e automação de processos**.

Com sensores conectados ao ESP32, conseguimos realizar o controle local de uma cancela, enviar informações para um modelo de Machine Learning e registrar eventos por meio do n8n.

Os testes de integração confirmaram a comunicação entre o ESP32 simulado, a FastAPI e o sistema de auditoria.

Embora ainda existam etapas de validação e integração com o portal web, o protótipo já demonstra como diferentes tecnologias podem trabalhar em conjunto para apoiar o monitoramento operacional.

**🚌 Smart Bus Gate + Pátio Monitor — Transformando eventos em inteligência operacional!**

---

Desenvolvido por **Camila do Prado Padalino**, **Felipe Cavalcante Bressane** e **Gabriel Teixeira Machado**.
