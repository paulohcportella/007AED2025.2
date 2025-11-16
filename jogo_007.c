#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <raylib.h>
#include <math.h>

#define MAX_MUNICOES 6
#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 700
#define MAX_NOME 30
#define TEMPO_ESCOLHA 2.0f
#define ARQUIVO_RANKING "ranking_007.dat"
#define MUNICOES_SUPER_TIRO 3

typedef enum {
    CARREGAR,
    ATIRAR,
    DEFENDER,
    SUPER_TIRO,
    NENHUMA
} Acao;

typedef enum {
    TIRO_NORMAL,
    TIRO_SUPER
} TipoTiro;

typedef enum {
    MENU_PRINCIPAL,
    ENTRADA_NOME,
    REGRAS,
    ESPERANDO_007,
    ESCOLHENDO_ACAO,
    MOSTRANDO_ACOES,
    PROCESSANDO_TURNO,
    PAUSADO,
    SCORE
} EstadoJogo;

typedef struct {
    int balas[MAX_MUNICOES];
    int topo;
} Pilha;

void criarPilha(Pilha *p);
int push(Pilha *p, int bala);
int pop(Pilha *p);
int isEmpty(Pilha *p);
int tamanho(Pilha *p);

typedef struct NodeRanking {
    char nome[MAX_NOME];
    int pontos;
    int vitorias;
    int derrotas;
    int rodadas;
    struct NodeRanking *prox;
    struct NodeRanking *ant;
} NodeRanking;

void inserirRanking(NodeRanking **head, const char *nome, int pontos, int vitorias, int derrotas, int rodadas);
void limparRanking(NodeRanking **head);
int tamanhoRanking(NodeRanking *head);
void salvarRanking(NodeRanking *head);
void carregarRanking(NodeRanking **head);

typedef struct {
    char nome[MAX_NOME];
    int vida;
    Pilha municoes;
    Acao acaoEscolhida;
    float posX;
    float posY;
    bool defendendo;
    bool atirando;
    bool carregando;
    bool superTiro;
    int contadorCarregamentos;
} Jogador;

typedef struct {
    float x, y;
    float velocidade;
    bool ativa;
    bool acertou;
    TipoTiro tipo;
    float animacao;
} Bala;

typedef struct {
    Jogador jogador;
    Jogador computador;
    Bala bala;
    int rodada;
    EstadoJogo estado;
    EstadoJogo estadoAntesPausa;
    char mensagem[256];
    float tempoMensagem;
    float tempoContagem;
    int contagemNum;
    int pontuacaoFinal;
    NodeRanking *rankingHead;
    char nomeInput[MAX_NOME];
    int inputIndex;
    bool jogadorEscolheu;
    Texture2D spriteEspiaoDir;
    Texture2D spriteEspiaoEsq;
    Texture2D spriteCPU;
} Jogo;

typedef struct {
    char nome[MAX_NOME];
    int pontos;
    int vitorias;
    int derrotas;
    int rodadas;
} PlayerStats;

void inicializarJogo(Jogo *jogo);
void resetarJogo(Jogo *jogo);
Acao decidirAcaoComputador(Jogo *jogo);
void processarTurno(Jogo *jogo);
void desenharMenu(Jogo *jogo);
void desenharEntradaNome(Jogo *jogo);
void desenharRegras(Jogo *jogo);
void desenharJogo(Jogo *jogo);
void desenharPausa(Jogo *jogo);
void desenharScore(Jogo *jogo);
void desenharPersonagem(Jogo *jogo, float x, float y, Color cor, bool defendendo, bool atirando, bool carregando, bool superTiro, bool olharEsquerda);
void desenharCoracao(int x, int y, bool cheio);
void desenharBala(Jogo *jogo);
void atualizarBala(Jogo *jogo);
const char* acaoParaString(Acao acao);
void definirMensagem(Jogo *jogo, const char *msg);
int calcularPontuacao(Jogo *jogo);
void insertionSort(PlayerStats stats[], int n);
bool podeUsarSuperTiro(Jogador *jogador);

void criarPilha(Pilha *p) {
    p->topo = -1;
}

int push(Pilha *p, int bala) {
    if (p->topo >= MAX_MUNICOES - 1) return 0;
    p->topo++;
    p->balas[p->topo] = bala;
    return 1;
}

int pop(Pilha *p) {
    if (p->topo < 0) return 0;
    p->topo--;
    return 1;
}

int isEmpty(Pilha *p) {
    return (p->topo < 0);
}

int tamanho(Pilha *p) {
    return p->topo + 1;
}

void inserirRanking(NodeRanking **head, const char *nome, int pontos, int vitorias, int derrotas, int rodadas) {
    NodeRanking *novo = (NodeRanking*)malloc(sizeof(NodeRanking));
    if (novo != NULL) {
        strcpy(novo->nome, nome);
        novo->pontos = pontos;
        novo->vitorias = vitorias;
        novo->derrotas = derrotas;
        novo->rodadas = rodadas;
        novo->prox = *head;
        novo->ant = NULL;
        
        if (*head != NULL) {
            (*head)->ant = novo;
        }
        *head = novo;
    }
}

void limparRanking(NodeRanking **head) {
    while (*head != NULL) {
        NodeRanking *aux = *head;
        *head = (*head)->prox;
        free(aux);
    }
}

int tamanhoRanking(NodeRanking *head) {
    int count = 0;
    while (head != NULL) {
        count++;
        head = head->prox;
    }
    return count;
}

void salvarRanking(NodeRanking *head) {
    FILE *arquivo = fopen(ARQUIVO_RANKING, "wb");
    if (arquivo == NULL) return;
    
    int total = tamanhoRanking(head);
    fwrite(&total, sizeof(int), 1, arquivo);
    
    NodeRanking *atual = head;
    while (atual != NULL) {
        fwrite(atual->nome, sizeof(char), MAX_NOME, arquivo);
        fwrite(&atual->pontos, sizeof(int), 1, arquivo);
        fwrite(&atual->vitorias, sizeof(int), 1, arquivo);
        fwrite(&atual->derrotas, sizeof(int), 1, arquivo);
        fwrite(&atual->rodadas, sizeof(int), 1, arquivo);
        atual = atual->prox;
    }
    
    fclose(arquivo);
}

void carregarRanking(NodeRanking **head) {
    FILE *arquivo = fopen(ARQUIVO_RANKING, "rb");
    if (arquivo == NULL) return;
    
    int total;
    if (fread(&total, sizeof(int), 1, arquivo) != 1) {
        fclose(arquivo);
        return;
    }
    
    for (int i = 0; i < total; i++) {
        char nome[MAX_NOME];
        int pontos, vitorias, derrotas, rodadas;
        
        if (fread(nome, sizeof(char), MAX_NOME, arquivo) != MAX_NOME) break;
        if (fread(&pontos, sizeof(int), 1, arquivo) != 1) break;
        if (fread(&vitorias, sizeof(int), 1, arquivo) != 1) break;
        if (fread(&derrotas, sizeof(int), 1, arquivo) != 1) break;
        if (fread(&rodadas, sizeof(int), 1, arquivo) != 1) break;
        
        inserirRanking(head, nome, pontos, vitorias, derrotas, rodadas);
    }
    
    fclose(arquivo);
}

void insertionSort(PlayerStats stats[], int n) {
    int i, j;
    PlayerStats key;
    
    for (i = 1; i < n; i++) {
        key = stats[i];
        j = i - 1;
        
        while (j >= 0 && (stats[j].pontos < key.pontos || 
               (stats[j].pontos == key.pontos && stats[j].vitorias < key.vitorias))) {
            stats[j + 1] = stats[j];
            j = j - 1;
        }
        stats[j + 1] = key;
    }
}

void inicializarJogo(Jogo *jogo) {
    strcpy(jogo->jogador.nome, "Jogador");
    jogo->jogador.vida = 3;
    criarPilha(&jogo->jogador.municoes);
    jogo->jogador.acaoEscolhida = NENHUMA;
    jogo->jogador.posX = 200;
    jogo->jogador.posY = 400;
    jogo->jogador.defendendo = false;
    jogo->jogador.atirando = false;
    jogo->jogador.carregando = false;
    jogo->jogador.superTiro = false;
    jogo->jogador.contadorCarregamentos = 0;
    
    strcpy(jogo->computador.nome, "CPU");
    jogo->computador.vida = 3;
    criarPilha(&jogo->computador.municoes);
    jogo->computador.acaoEscolhida = NENHUMA;
    jogo->computador.posX = 950;
    jogo->computador.posY = 400;
    jogo->computador.defendendo = false;
    jogo->computador.atirando = false;
    jogo->computador.carregando = false;
    jogo->computador.superTiro = false;
    jogo->computador.contadorCarregamentos = 0;
    
    jogo->bala.ativa = false;
    jogo->bala.acertou = false;
    jogo->bala.tipo = TIRO_NORMAL;
    jogo->bala.animacao = 0;
    
    jogo->rodada = 0;
    jogo->estado = MENU_PRINCIPAL;
    jogo->estadoAntesPausa = MENU_PRINCIPAL;
    jogo->tempoMensagem = 0;
    jogo->tempoContagem = 0;
    jogo->contagemNum = 0;
    jogo->pontuacaoFinal = 0;
    jogo->rankingHead = NULL;
    jogo->jogadorEscolheu = false;
    strcpy(jogo->mensagem, "");
    strcpy(jogo->nomeInput, "");
    jogo->inputIndex = 0;
    
    carregarRanking(&jogo->rankingHead);
}

void resetarJogo(Jogo *jogo) {
    jogo->jogador.vida = 3;
    criarPilha(&jogo->jogador.municoes);
    jogo->jogador.acaoEscolhida = NENHUMA;
    jogo->jogador.defendendo = false;
    jogo->jogador.atirando = false;
    jogo->jogador.carregando = false;
    jogo->jogador.superTiro = false;
    jogo->jogador.contadorCarregamentos = 0;
    
    jogo->computador.vida = 3;
    criarPilha(&jogo->computador.municoes);
    jogo->computador.acaoEscolhida = NENHUMA;
    jogo->computador.defendendo = false;
    jogo->computador.atirando = false;
    jogo->computador.carregando = false;
    jogo->computador.superTiro = false;
    jogo->computador.contadorCarregamentos = 0;
    
    jogo->bala.ativa = false;
    jogo->bala.acertou = false;
    jogo->bala.tipo = TIRO_NORMAL;
    jogo->bala.animacao = 0;
    
    jogo->rodada = 0;
    jogo->tempoContagem = 0;
    jogo->contagemNum = 0;
    jogo->jogadorEscolheu = false;
    strcpy(jogo->mensagem, "");
    jogo->tempoMensagem = 0;
    jogo->pontuacaoFinal = 0;
}

const char* acaoParaString(Acao acao) {
    switch (acao) {
        case CARREGAR: return "CARREGOU";
        case ATIRAR: return "ATIROU";
        case SUPER_TIRO: return "SUPER TIRO!";
        case DEFENDER: return "DEFENDEU";
        case NENHUMA: return "NAO FEZ NADA";
        default: return "---";
    }
}

void definirMensagem(Jogo *jogo, const char *msg) {
    strcpy(jogo->mensagem, msg);
    jogo->tempoMensagem = 3.0f;
}

bool podeUsarSuperTiro(Jogador *jogador) {
    return (tamanho(&jogador->municoes) >= MUNICOES_SUPER_TIRO);
}

Acao decidirAcaoComputador(Jogo *jogo) {
    int municoesJogador = tamanho(&jogo->jogador.municoes);
    int municoesComputador = tamanho(&jogo->computador.municoes);
    
    if (!isEmpty(&jogo->computador.municoes)) {
        if (municoesJogador > 0 && rand() % 100 < 40) {
            return DEFENDER;
        }
        if (rand() % 100 < 80) {
            return ATIRAR;
        }
    }
    
    if (municoesComputador < 2) {
        return CARREGAR;
    }
    
    if (municoesJogador >= 3) {
        if (rand() % 100 < 60) {
            return DEFENDER;
        }
    }
    
    return CARREGAR;
}

int calcularPontuacao(Jogo *jogo) {
    int pontos = 0;
    
    if (jogo->jogador.vida > 0) {
        pontos += 1000;
        pontos += jogo->jogador.vida * 200;
        pontos += tamanho(&jogo->jogador.municoes) * 50;
        
        if (jogo->rodada <= 5) pontos += 500;
        else if (jogo->rodada <= 10) pontos += 300;
        else if (jogo->rodada <= 15) pontos += 100;
    } else {
        pontos = jogo->rodada * 10;
    }
    
    return pontos;
}

void atualizarBala(Jogo *jogo) {
    if (jogo->bala.ativa) {
        jogo->bala.x += jogo->bala.velocidade;
        jogo->bala.animacao += 0.1f;
        
        if (jogo->bala.velocidade > 0) {
            if (jogo->bala.x >= jogo->computador.posX - 30) {
                jogo->bala.ativa = false;
                if (!jogo->computador.defendendo || jogo->bala.tipo == TIRO_SUPER) {
                    jogo->bala.acertou = true;
                }
            }
        } else {
            if (jogo->bala.x <= jogo->jogador.posX + 30) {
                jogo->bala.ativa = false;
                if (!jogo->jogador.defendendo) {
                    jogo->bala.acertou = true;
                }
            }
        }
    }
}

void processarTurno(Jogo *jogo) {
    Acao acaoJogador = jogo->jogador.acaoEscolhida;
    Acao acaoComputador = jogo->computador.acaoEscolhida;
    
    jogo->jogador.defendendo = false;
    jogo->jogador.atirando = false;
    jogo->jogador.carregando = false;
    jogo->jogador.superTiro = false;
    jogo->computador.defendendo = false;
    jogo->computador.atirando = false;
    jogo->computador.carregando = false;
    jogo->computador.superTiro = false;
    
    char msg[256];
    sprintf(msg, "%s %s | CPU %s", 
            jogo->jogador.nome, 
            acaoParaString(acaoJogador),
            acaoParaString(acaoComputador));
    definirMensagem(jogo, msg);
    
    if (acaoJogador == CARREGAR) {
        push(&jogo->jogador.municoes, 1);
        jogo->jogador.carregando = true;
        jogo->jogador.contadorCarregamentos++;
    } else {
        if (acaoJogador != NENHUMA) {
            jogo->jogador.contadorCarregamentos = 0;
        }
    }
    
    if (acaoComputador == CARREGAR) {
        push(&jogo->computador.municoes, 1);
        jogo->computador.carregando = true;
    }
    
    if (acaoJogador == DEFENDER) {
        jogo->jogador.defendendo = true;
    }
    
    if (acaoComputador == DEFENDER) {
        jogo->computador.defendendo = true;
    }
    
    bool jogadorAtirou = false;
    bool cpuAtirou = false;
    
    if (acaoJogador == ATIRAR && !isEmpty(&jogo->jogador.municoes)) {
        pop(&jogo->jogador.municoes);
        jogo->jogador.atirando = true;
        jogadorAtirou = true;
    }
    
    if (acaoJogador == SUPER_TIRO && tamanho(&jogo->jogador.municoes) >= MUNICOES_SUPER_TIRO) {
        for (int i = 0; i < MUNICOES_SUPER_TIRO; i++) {
            pop(&jogo->jogador.municoes);
        }
        jogo->jogador.superTiro = true;
        jogadorAtirou = true;
        jogo->jogador.contadorCarregamentos = 0;
    }
    
    if (acaoComputador == ATIRAR && !isEmpty(&jogo->computador.municoes)) {
        pop(&jogo->computador.municoes);
        jogo->computador.atirando = true;
        cpuAtirou = true;
    }
    
    if (jogadorAtirou) {
        if (!jogo->computador.defendendo || acaoJogador == SUPER_TIRO) {
            jogo->computador.vida--;
        }
    }
    
    if (cpuAtirou && !jogo->jogador.defendendo) {
        jogo->jogador.vida--;
    }
    
    if (jogadorAtirou) {
        jogo->bala.x = jogo->jogador.posX + 50;
        jogo->bala.y = jogo->jogador.posY;
        jogo->bala.velocidade = 15.0f;
        jogo->bala.ativa = true;
        jogo->bala.tipo = (acaoJogador == SUPER_TIRO) ? TIRO_SUPER : TIRO_NORMAL;
        jogo->bala.animacao = 0;
    } else if (cpuAtirou) {
        jogo->bala.x = jogo->computador.posX - 50;
        jogo->bala.y = jogo->computador.posY;
        jogo->bala.velocidade = -15.0f;
        jogo->bala.ativa = true;
        jogo->bala.tipo = TIRO_NORMAL;
        jogo->bala.animacao = 0;
    }
}

void desenharCoracao(int x, int y, bool cheio) {
    Color cor = cheio ? RED : DARKGRAY;
    
    DrawCircle(x - 6, y - 3, 8, cor);
    DrawCircle(x + 6, y - 3, 8, cor);
    
    Vector2 pontos[4] = {
        {(float)(x - 12), (float)(y - 2)},
        {(float)x, (float)(y + 10)},
        {(float)(x + 12), (float)(y - 2)},
        {(float)x, (float)(y - 8)}
    };
    
    DrawTriangle(pontos[0], pontos[1], (Vector2){(float)x, (float)(y - 2)}, cor);
    DrawTriangle(pontos[2], pontos[1], (Vector2){(float)x, (float)(y - 2)}, cor);
}

void desenharBala(Jogo *jogo) {
    if (!jogo->bala.ativa) return;
    
    if (jogo->bala.tipo == TIRO_SUPER) {
        float raio = 15 + sin(jogo->bala.animacao * 5) * 3;
        
        DrawCircle((int)jogo->bala.x, (int)jogo->bala.y, raio, (Color){255, 100, 0, 255});
        DrawCircle((int)jogo->bala.x, (int)jogo->bala.y, raio * 0.7f, (Color){255, 200, 0, 255});
        DrawCircle((int)jogo->bala.x, (int)jogo->bala.y, raio * 0.4f, (Color){255, 255, 100, 255});
        
        for (int i = 0; i < 8; i++) {
            float angulo = (jogo->bala.animacao + i * 0.785f);
            float dist = raio + sin(angulo * 3) * 5;
            float fx = jogo->bala.x + cos(angulo) * dist;
            float fy = jogo->bala.y + sin(angulo) * dist;
            DrawCircle((int)fx, (int)fy, 5, (Color){255, 150, 0, 180});
        }
        
        DrawCircle((int)(jogo->bala.x - jogo->bala.velocidade * 2), (int)jogo->bala.y, 8, (Color){255, 100, 0, 100});
        
    } else {
        DrawCircle((int)jogo->bala.x, (int)jogo->bala.y, 8, ORANGE);
        DrawCircle((int)jogo->bala.x, (int)jogo->bala.y, 5, YELLOW);
    }
}

void desenharPersonagem(Jogo *jogo, float x, float y, Color cor, bool defendendo, bool atirando, bool carregando, bool superTiro, bool olharEsquerda) {
    Texture2D sprite;
    bool flipHorizontal = false;
    
    if (olharEsquerda) {
        sprite = jogo->spriteCPU;
        flipHorizontal = true;
    } else {
        sprite = jogo->spriteEspiaoDir;
    }
    
    float alturaDesejada = 120.0f;
    float escala = alturaDesejada / sprite.height;
    float largura = sprite.width * escala;
    float altura = sprite.height * escala;
    
    Rectangle source = {0, 0, (float)sprite.width, (float)sprite.height};
    if (flipHorizontal) {
        source.width = -source.width;
    }
    
    Rectangle dest = {x, y, largura, altura};
    Vector2 origin = {largura/2, altura/2};
    
    DrawTexturePro(sprite, source, dest, origin, 0, WHITE);
    
    if (defendendo) {
        DrawCircle((int)x, (int)y, 60, (Color){100, 100, 255, 150});
        DrawCircleLines((int)x, (int)y, 60, BLUE);
        DrawCircleLines((int)x, (int)y, 58, BLUE);
    }
    
    (void)cor;
    (void)atirando;
    (void)carregando;
    (void)superTiro;
}

void desenharMenu(Jogo *jogo) {
    (void)jogo;
    ClearBackground((Color){20, 20, 30, 255});
    
    DrawText("JOGO 007", SCREEN_WIDTH/2 - 180, 80, 80, GOLD);
    DrawText("Duelo de Agentes Secretos", SCREEN_WIDTH/2 - 220, 170, 30, LIGHTGRAY);
    
    DrawRectangle(SCREEN_WIDTH/2 - 200, 280, 400, 70, DARKGREEN);
    DrawText("1 - JOGAR", SCREEN_WIDTH/2 - 80, 305, 30, WHITE);
    
    DrawRectangle(SCREEN_WIDTH/2 - 200, 370, 400, 70, DARKBLUE);
    DrawText("2 - REGRAS", SCREEN_WIDTH/2 - 90, 395, 30, WHITE);
    
    DrawRectangle(SCREEN_WIDTH/2 - 200, 460, 400, 70, MAROON);
    DrawText("ESC - SAIR", SCREEN_WIDTH/2 - 80, 485, 30, WHITE);
    
    DrawText("Estruturas: PILHA + LISTA DUPLA | Algoritmo: INSERTION SORT", 
             SCREEN_WIDTH/2 - 300, 640, 18, GRAY);
}

void desenharEntradaNome(Jogo *jogo) {
    ClearBackground((Color){25, 25, 35, 255});
    
    DrawText("Digite seu nome:", SCREEN_WIDTH/2 - 150, 250, 35, YELLOW);
    
    DrawRectangle(SCREEN_WIDTH/2 - 200, 320, 400, 60, DARKGRAY);
    DrawRectangleLines(SCREEN_WIDTH/2 - 200, 320, 400, 60, GOLD);
    
    if (strlen(jogo->nomeInput) > 0) {
        DrawText(jogo->nomeInput, SCREEN_WIDTH/2 - 180, 335, 30, WHITE);
    } else {
        DrawText("...", SCREEN_WIDTH/2 - 10, 335, 30, GRAY);
    }
    
    DrawText("Pressione ENTER para continuar", SCREEN_WIDTH/2 - 180, 450, 22, LIGHTGRAY);
    DrawText("O - Menu Principal", SCREEN_WIDTH/2 - 100, 500, 20, GRAY);
}

void desenharRegras(Jogo *jogo) {
    (void)jogo;
    ClearBackground((Color){25, 25, 35, 255});
    
    DrawText("REGRAS DO JOGO 007", SCREEN_WIDTH/2 - 200, 30, 45, GOLD);
    
    int y = 90;
    DrawText("OBJETIVO: Reduzir a vida do oponente a ZERO (3 coracoes)", 100, y, 21, YELLOW);
    y += 45;
    
    DrawText("COMO FUNCIONA:", 100, y, 23, YELLOW);
    y += 30;
    DrawText("1. O jogo anuncia '007' com contagem", 120, y, 20, WHITE);
    y += 28;
    DrawText("2. Voce tem 2 SEGUNDOS para escolher UMA acao", 120, y, 20, WHITE);
    y += 28;
    DrawText("3. Se NAO escolher, fica VULNERAVEL (sem defesa!)", 120, y, 20, RED);
    y += 45;
    
    DrawText("ACOES (PILHA de municoes):", 100, y, 23, YELLOW);
    y += 30;
    DrawText("Q - CARREGAR: +1 bala (maximo 6)", 120, y, 20, WHITE);
    y += 28;
    DrawText("W - ATIRAR: Usa 1 bala e ataca", 120, y, 20, WHITE);
    y += 28;
    DrawText("E - DEFENDER: Bloqueia ataques (escudo azul)", 120, y, 20, WHITE);
    y += 28;
    DrawText("R - SUPER TIRO: Usa 3 balas, BOLA DE FOGO que IGNORA ESCUDO!", 120, y, 20, ORANGE);
    y += 45;
    
    DrawText("SUPER TIRO:", 100, y, 23, ORANGE);
    y += 30;
    DrawText("- Aparece quando voce tem 3+ balas", 120, y, 20, WHITE);
    y += 28;
    DrawText("- Usa 3 balas de uma vez", 120, y, 20, WHITE);
    y += 28;
    DrawText("- PASSA PELO ESCUDO DO INIMIGO!", 120, y, 20, RED);
    y += 45;
    
    DrawText("P = PAUSAR | O = Menu Principal", 100, y, 19, LIGHTGRAY);
    
    DrawRectangle(SCREEN_WIDTH/2 - 150, 620, 300, 50, DARKGREEN);
    DrawText("O - Menu", SCREEN_WIDTH/2 - 50, 635, 20, WHITE);
}

void desenharPausa(Jogo *jogo) {
    desenharJogo(jogo);
    
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){0, 0, 0, 180});
    
    DrawText("JOGO PAUSADO", SCREEN_WIDTH/2 - 180, 250, 50, YELLOW);
    
    DrawRectangle(SCREEN_WIDTH/2 - 200, 350, 400, 70, DARKGREEN);
    DrawText("P - Continuar Jogando", SCREEN_WIDTH/2 - 140, 375, 24, WHITE);
    
    DrawRectangle(SCREEN_WIDTH/2 - 200, 450, 400, 70, DARKBLUE);
    DrawText("O - Menu Principal", SCREEN_WIDTH/2 - 120, 475, 24, WHITE);
}

void desenharJogo(Jogo *jogo) {
    ClearBackground((Color){35, 35, 45, 255});
    
    DrawRectangle(0, 550, SCREEN_WIDTH, 150, (Color){90, 70, 50, 255});
    
    DrawText(TextFormat("RODADA %d", jogo->rodada), SCREEN_WIDTH/2 - 80, 20, 30, YELLOW);
    
    if (jogo->estado == ESPERANDO_007) {
        int tamanhoTexto = 120;
        const char *texto = "007";
        if (jogo->contagemNum == 1) texto = "0...";
        else if (jogo->contagemNum == 2) texto = "00...";
        else if (jogo->contagemNum == 3) texto = "007!";
        
        DrawText(texto, SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 - 100, tamanhoTexto, RED);
    }
    
    DrawRectangle(50, 70, 350, 180, (Color){50, 35, 25, 200});
    DrawText(jogo->jogador.nome, 120, 80, 24, GOLD);
    
    DrawText("Vidas:", 70, 110, 22, WHITE);
    for (int i = 0; i < 3; i++) {
        desenharCoracao(150 + i * 35, 120, i < jogo->jogador.vida);
    }
    
    DrawText(TextFormat("Balas: %d/%d", tamanho(&jogo->jogador.municoes), MAX_MUNICOES), 
             70, 150, 22, WHITE);
    
    if (podeUsarSuperTiro(&jogo->jogador)) {
        DrawText("SUPER TIRO!", 70, 180, 20, ORANGE);
        DrawCircle(250, 188, 5, ORANGE);
    }
    
    desenharPersonagem(jogo, jogo->jogador.posX, jogo->jogador.posY, BLUE, 
                       jogo->jogador.defendendo, jogo->jogador.atirando, 
                       jogo->jogador.carregando, jogo->jogador.superTiro, false);
    
    DrawRectangle(800, 70, 350, 180, (Color){50, 35, 25, 200});
    DrawText("CPU", 970, 80, 24, GOLD);
    
    DrawText("Vidas:", 820, 110, 22, WHITE);
    for (int i = 0; i < 3; i++) {
        desenharCoracao(900 + i * 35, 120, i < jogo->computador.vida);
    }
    
    DrawText(TextFormat("Balas: %d/%d", tamanho(&jogo->computador.municoes), MAX_MUNICOES), 
             820, 150, 22, WHITE);
    
    desenharPersonagem(jogo, jogo->computador.posX, jogo->computador.posY, RED, 
                       jogo->computador.defendendo, jogo->computador.atirando, 
                       jogo->computador.carregando, jogo->computador.superTiro, true);
    
    DrawText("VS", SCREEN_WIDTH/2 - 35, 300, 50, YELLOW);
    
    desenharBala(jogo);
    
    if (jogo->estado == ESCOLHENDO_ACAO && !jogo->jogadorEscolheu) {
        DrawText(TextFormat("Tempo: %.1f s", jogo->tempoContagem), SCREEN_WIDTH/2 - 70, 60, 24, RED);
        
        DrawRectangle(SCREEN_WIDTH/2 - 330, 580, 120, 50, DARKGREEN);
        DrawText("Q-Carregar", SCREEN_WIDTH/2 - 320, 595, 16, WHITE);
        
        DrawRectangle(SCREEN_WIDTH/2 - 190, 580, 100, 50, RED);
        DrawText("W-Atirar", SCREEN_WIDTH/2 - 180, 595, 16, WHITE);
        
        DrawRectangle(SCREEN_WIDTH/2 - 70, 580, 110, 50, BLUE);
        DrawText("E-Defender", SCREEN_WIDTH/2 - 60, 595, 16, WHITE);
        
        if (podeUsarSuperTiro(&jogo->jogador)) {
            DrawRectangle(SCREEN_WIDTH/2 + 60, 580, 130, 50, ORANGE);
            DrawText("R-SUPER", SCREEN_WIDTH/2 + 75, 595, 16, WHITE);
            DrawCircle(SCREEN_WIDTH/2 + 120, 605, 8, YELLOW);
        }
    } else if (jogo->jogadorEscolheu) {
        DrawText("Acao escolhida!", SCREEN_WIDTH/2 - 100, 600, 24, GREEN);
    }
    
    if (jogo->tempoMensagem > 0) {
        DrawRectangle(SCREEN_WIDTH/2 - 350, 240, 700, 70, (Color){0, 0, 0, 230});
        DrawText(jogo->mensagem, SCREEN_WIDTH/2 - 330, 255, 26, YELLOW);
    }
    
    DrawText("P - Pausar | O - Menu", 20, 670, 18, LIGHTGRAY);
}

void desenharScore(Jogo *jogo) {
    ClearBackground((Color){15, 15, 25, 255});
    
    bool vitoria = (jogo->jogador.vida > 0);
    
    if (vitoria) {
        DrawText("VITORIA!", SCREEN_WIDTH/2 - 140, 60, 65, GREEN);
    } else {
        DrawText("DERROTA!", SCREEN_WIDTH/2 - 140, 60, 65, RED);
    }
    
    DrawText(TextFormat("Pontuacao: %d", jogo->pontuacaoFinal), 
             SCREEN_WIDTH/2 - 140, 150, 40, GOLD);
    
    DrawText(TextFormat("Rodadas: %d", jogo->rodada), SCREEN_WIDTH/2 - 90, 210, 25, WHITE);
    
    DrawText("RANKING (Insertion Sort)", SCREEN_WIDTH/2 - 180, 270, 28, YELLOW);
    
    int total = tamanhoRanking(jogo->rankingHead);
    if (total > 0) {
        PlayerStats *stats = (PlayerStats*)malloc(sizeof(PlayerStats) * total);
        NodeRanking *atual = jogo->rankingHead;
        int idx = 0;
        
        while (atual != NULL && idx < total) {
            strcpy(stats[idx].nome, atual->nome);
            stats[idx].pontos = atual->pontos;
            stats[idx].vitorias = atual->vitorias;
            stats[idx].derrotas = atual->derrotas;
            stats[idx].rodadas = atual->rodadas;
            atual = atual->prox;
            idx++;
        }
        
        insertionSort(stats, total);
        
        int mostrar = (total < 5) ? total : 5;
        int y = 320;
        for (int i = 0; i < mostrar; i++) {
            DrawText(TextFormat("%d. %s - %d pts", i+1, stats[i].nome, stats[i].pontos),
                     SCREEN_WIDTH/2 - 200, y, 22, WHITE);
            y += 35;
        }
        
        free(stats);
    }
    
    DrawRectangle(SCREEN_WIDTH/2 - 160, 560, 320, 50, DARKGREEN);
    DrawText("ENTER - Jogar Novamente", SCREEN_WIDTH/2 - 140, 575, 20, WHITE);
    
    DrawRectangle(SCREEN_WIDTH/2 - 160, 625, 320, 50, DARKBLUE);
    DrawText("O - Menu", SCREEN_WIDTH/2 - 50, 640, 20, WHITE);
}

int main(void) {
    srand((unsigned int)time(NULL));

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Jogo 007 - Duelo de Agentes");
    SetTargetFPS(60);

    Jogo jogo;
    inicializarJogo(&jogo);
    
    jogo.spriteEspiaoDir = LoadTexture("assets/espiao_esquerda.png");
    jogo.spriteEspiaoEsq = LoadTexture("assets/espiao_direita.png");
    jogo.spriteCPU = LoadTexture("assets/espiao_cpu.png");

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        if (jogo.tempoMensagem > 0) {
            jogo.tempoMensagem -= dt;
        }

        if (jogo.bala.ativa) {
            atualizarBala(&jogo);
        }

        switch (jogo.estado) {
            case MENU_PRINCIPAL:
                if (IsKeyPressed(KEY_ONE)) {
                    jogo.estado = ENTRADA_NOME;
                }
                if (IsKeyPressed(KEY_TWO)) {
                    jogo.estado = REGRAS;
                }
                break;

            case ENTRADA_NOME: {
                int key = GetCharPressed();
                while (key > 0) {
                    if ((key >= 32) && (key <= 125) && (jogo.inputIndex < MAX_NOME - 1)) {
                        jogo.nomeInput[jogo.inputIndex] = (char)key;
                        jogo.inputIndex++;
                        jogo.nomeInput[jogo.inputIndex] = '\0';
                    }
                    key = GetCharPressed();
                }

                if (IsKeyPressed(KEY_BACKSPACE)) {
                    if (jogo.inputIndex > 0) {
                        jogo.inputIndex--;
                        jogo.nomeInput[jogo.inputIndex] = '\0';
                    }
                }
                
                if (IsKeyPressed(KEY_O)) {
                    jogo.estado = MENU_PRINCIPAL;
                }

                if (IsKeyPressed(KEY_ENTER) && strlen(jogo.nomeInput) > 0) {
                    strcpy(jogo.jogador.nome, jogo.nomeInput);
                    resetarJogo(&jogo);
                    jogo.estado = ESPERANDO_007;
                    jogo.tempoContagem = 1.0f;
                    jogo.contagemNum = 1;
                }
            } break;

            case REGRAS:
                if (IsKeyPressed(KEY_O)) {
                    jogo.estado = MENU_PRINCIPAL;
                }
                break;

            case ESPERANDO_007:
                jogo.tempoContagem -= dt;
                if (jogo.tempoContagem <= 0) {
                    jogo.contagemNum++;
                    jogo.tempoContagem = 1.0f;

                    if (jogo.contagemNum > 3) {
                        jogo.rodada++;
                        jogo.estado = ESCOLHENDO_ACAO;
                        jogo.jogadorEscolheu = false;
                        jogo.jogador.acaoEscolhida = NENHUMA;
                        jogo.computador.acaoEscolhida = decidirAcaoComputador(&jogo);
                        jogo.tempoContagem = TEMPO_ESCOLHA;
                    }
                }

                if (IsKeyPressed(KEY_P)) {
                    jogo.estadoAntesPausa = jogo.estado;
                    jogo.estado = PAUSADO;
                }
                if (IsKeyPressed(KEY_O)) {
                    jogo.estado = MENU_PRINCIPAL;
                }
                break;

            case ESCOLHENDO_ACAO:
                if (!jogo.jogadorEscolheu) {
                    if (IsKeyPressed(KEY_Q)) {
                        jogo.jogador.acaoEscolhida = CARREGAR;
                        jogo.jogadorEscolheu = true;
                    }
                    if (IsKeyPressed(KEY_W)) {
                        jogo.jogador.acaoEscolhida = ATIRAR;
                        jogo.jogadorEscolheu = true;
                    }
                    if (IsKeyPressed(KEY_E)) {
                        jogo.jogador.acaoEscolhida = DEFENDER;
                        jogo.jogadorEscolheu = true;
                    }
                    if (IsKeyPressed(KEY_R) && podeUsarSuperTiro(&jogo.jogador)) {
                        jogo.jogador.acaoEscolhida = SUPER_TIRO;
                        jogo.jogadorEscolheu = true;
                    }
                }

                jogo.tempoContagem -= dt;
                if (jogo.tempoContagem <= 0) {
                    if (!jogo.jogadorEscolheu) {
                        jogo.jogador.acaoEscolhida = NENHUMA;
                    }
                    jogo.estado = PROCESSANDO_TURNO;
                }

                if (IsKeyPressed(KEY_P)) {
                    jogo.estadoAntesPausa = jogo.estado;
                    jogo.estado = PAUSADO;
                }
                if (IsKeyPressed(KEY_O)) {
                    jogo.estado = MENU_PRINCIPAL;
                }
                break;

            case PROCESSANDO_TURNO:
                processarTurno(&jogo);
                jogo.tempoContagem = 2.0f;
                jogo.estado = MOSTRANDO_ACOES;

                if (IsKeyPressed(KEY_P)) {
                    jogo.estadoAntesPausa = jogo.estado;
                    jogo.estado = PAUSADO;
                }
                if (IsKeyPressed(KEY_O)) {
                    jogo.estado = MENU_PRINCIPAL;
                }
                break;

            case MOSTRANDO_ACOES:
                jogo.tempoContagem -= dt;
                if (jogo.tempoContagem <= 0) {
                    if (jogo.jogador.vida <= 0 || jogo.computador.vida <= 0) {
                        jogo.pontuacaoFinal = calcularPontuacao(&jogo);
                        int vit = (jogo.jogador.vida > 0) ? 1 : 0;
                        int der = (jogo.jogador.vida > 0) ? 0 : 1;
                        inserirRanking(&jogo.rankingHead, jogo.jogador.nome,
                                       jogo.pontuacaoFinal, vit, der, jogo.rodada);
                        salvarRanking(jogo.rankingHead);
                        jogo.estado = SCORE;
                    } else {
                        jogo.estado = ESPERANDO_007;
                        jogo.tempoContagem = 1.0f;
                        jogo.contagemNum = 1;
                    }
                }

                if (IsKeyPressed(KEY_P)) {
                    jogo.estadoAntesPausa = jogo.estado;
                    jogo.estado = PAUSADO;
                }
                if (IsKeyPressed(KEY_O)) {
                    jogo.estado = MENU_PRINCIPAL;
                }
                break;

            case PAUSADO:
                if (IsKeyPressed(KEY_P)) {
                    jogo.estado = jogo.estadoAntesPausa;
                }
                if (IsKeyPressed(KEY_O)) {
                    jogo.estado = MENU_PRINCIPAL;
                }
                break;

            case SCORE:
                if (IsKeyPressed(KEY_ENTER)) {
                    jogo.inputIndex = 0;
                    strcpy(jogo.nomeInput, "");
                    jogo.estado = ENTRADA_NOME;
                }
                if (IsKeyPressed(KEY_O)) {
                    jogo.estado = MENU_PRINCIPAL;
                }
                break;
        }

        BeginDrawing();

        switch (jogo.estado) {
            case MENU_PRINCIPAL: desenharMenu(&jogo); break;
            case ENTRADA_NOME:   desenharEntradaNome(&jogo); break;
            case REGRAS:         desenharRegras(&jogo); break;
            case PAUSADO:        desenharPausa(&jogo); break;
            case SCORE:          desenharScore(&jogo); break;
            default:             desenharJogo(&jogo); break;
        }

        EndDrawing();
    }

    salvarRanking(jogo.rankingHead);
    limparRanking(&jogo.rankingHead);
    
    UnloadTexture(jogo.spriteEspiaoDir);
    UnloadTexture(jogo.spriteEspiaoEsq);
    UnloadTexture(jogo.spriteCPU);
    
    CloseWindow();

    return 0;
}
