# Fanorona em Rede (RPC)

Implementação do jogo de tabuleiro **Fanorona** (jogo tradicional de dois jogadores
originário de Madagascar) para dois jogadores em máquinas diferentes, comunicando via
**RPC** (invocação remota de métodos, no estilo RMI do Java). Cada lado expõe uma
interface remota (`PeerRemote`) com um método por ação do jogo (`remoteMove`,
`remoteChat`, `remoteResign`, ...); quem quer jogar invoca esse método diretamente
através de um *stub* (`BoardStub`), que serializa a chamada e a entrega, do outro lado,
a um *skeleton* (`BoardSkeleton`) que a executa direto no tabuleiro remoto — sem montar
mensagens genéricas nem checar um campo de "tipo". O transporte por baixo continua
sendo TCP (`QTcpServer`/`QTcpSocket`), do mesmo jeito que o próprio RMI do Java também
usa sockets escondidos por baixo da chamada remota. Feito em C++/Qt Widgets.

O projeto está na pasta [`Fanorona/`](./Fanorona).

## Funcionalidades

- Tabuleiro 5×9 com as regras oficiais do Fanorona: movimento simples, captura por
  aproximação, captura por afastamento e sequência de capturas (cadeia) na mesma
  jogada.
- Partida em rede entre dois jogadores (um hospeda, o outro entra).
- Controle de turno, com sorteio de quem começa.
- Chat de texto durante a partida.
- Desistência.
- Indicação de vencedor ao final.

## Pré-requisitos

- [Qt](https://www.qt.io/download) 6.x (módulos **Widgets** e **Network**) — também
  funciona com Qt 5.x.
- Um compilador C++17 (o kit MinGW que acompanha o instalador do Qt já resolve, no
  Windows).
- CMake 3.16+ (o Qt Creator já inclui um).
- Qt Creator (recomendado) ou qualquer IDE/terminal que rode CMake.

## Como compilar e rodar

### Opção 1 — Qt Creator (recomendado)

1. Abra o Qt Creator.
2. **File → Open Project** e selecione `Fanorona/CMakeLists.txt`.
3. Escolha o kit Desktop (Qt + MinGW ou o compilador da sua plataforma) e configure o
   projeto.
4. Clique em **Run** (▶) para compilar e executar.

### Opção 2 — linha de comando

A partir da pasta `Fanorona/`:

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

O executável gerado fica em `build/Fanorona.exe` (Windows) ou `build/Fanorona` (Linux/macOS).

## Como jogar uma partida em rede

Para testar sozinho, basta abrir **duas instâncias** do executável — na mesma máquina
ou em duas máquinas diferentes na mesma rede.

> No Qt Creator, clicar em **Run** de novo reinicia a instância já aberta em vez de
> abrir uma segunda. Para ter as duas rodando ao mesmo tempo, rode a segunda instância
> direto pelo executável compilado (ex.: dando duplo clique nele, ou chamando-o por um
> terminal com o Qt no PATH — no Windows, use o atalho "Qt \<versão\> (MinGW ... bits)"
> que o instalador do Qt cria no menu Iniciar).

**Jogador 1 (host):**
1. Abra o jogo, digite seu nome e clique em **Hospedar Partida**.
2. A tela vai mostrar o endereço IP da sua máquina e a porta em que o jogo está
   escutando — anote os dois para passar ao outro jogador.

**Jogador 2 (cliente):**
1. Abra o jogo (em outra janela/máquina), digite seu nome e clique em
   **Entrar em Partida**.
2. Digite o IP e a porta informados pelo host (se for na mesma máquina, o endereço de
   loopback local — `127.0.0.1` — funciona no lugar do IP).
3. Clique em **Conectar**.

Assim que a conexão for estabelecida, as duas janelas sincronizam automaticamente e a
partida começa, com o turno inicial sorteado entre os dois.

> Se a conexão falhar entre duas máquinas diferentes, verifique se o Firewall do
> Windows não está bloqueando o executável (aceite o aviso de permissão de rede na
> primeira execução) e se as duas máquinas estão na mesma rede local.

## Jogando

- Clique em uma peça sua para ver os destinos possíveis destacados no tabuleiro.
- Clique em um destino destacado para mover. Se o movimento permitir mais de uma opção
  (simples, captura por aproximação ou por afastamento), um diálogo pede para você
  escolher.
- Depois de uma captura, se for possível continuar capturando com a mesma peça, o
  botão **Finalizar Jogada** fica disponível — você pode continuar a cadeia ou parar
  por ali.
- **Desistir** encerra a partida a favor do adversário a qualquer momento.
- O chat, à direita, funciona durante toda a partida.
