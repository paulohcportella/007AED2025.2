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
    PROCESSANDO_TURNO,
    JOGANDO,
    SCORE
} EstadoJogo;

typedef struct {
    int balas[MAX_MUNICOES];
    int topo;
} Pilha;

typedef struct NodeRanking {
    char nome[MAX_NOME];
    int pontos;
    struct NodeRanking *prox;
    struct NodeRanking *ant;
} NodeRanking;

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
    char mensagem[256];
    float tempoMensagem;
    float tempoContagem;
    int contagemNum;
    int pontuacaoFinal;
    NodeRanking *rankingHead;
    char nomeInput[MAX_NOME];
    int inputIndex;
    bool jogadorEscolheu;
} Jogo;

void criarPilha(Pilha *p) { p->topo = -1; }
int push(Pilha *p, int bala) { if (p->topo >= MAX_MUNICOES - 1) return 0; p->topo++; p->balas[p->topo] = bala; return 1; }
int pop(Pilha *p) { if (p->topo < 0) return 0; p->topo--; return 1; }
int isEmpty(Pilha *p) { return (p->topo < 0); }
int tamanho(Pilha *p) { return p->topo + 1; }

void inserirRanking(NodeRanking **head, const char *nome, int pontos) {
    NodeRanking *novo = (NodeRanking*)malloc(sizeof(NodeRanking));
    if (novo != NULL) {
        strcpy(novo->nome, nome);
        novo->pontos = pontos;
        novo->prox = *head;
        novo->ant = NULL;
        if (*head != NULL) (*head)->ant = novo;
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
    int municoesComputador = tamanho(&jogo->computador.municoes);
    
    if (!isEmpty(&jogo->computador.municoes)) {
        if (rand() % 100 < 80) return ATIRAR;
    }
    
    if (municoesComputador < 2) return CARREGAR;
    
    return CARREGAR;
}

int calcularPontuacao(Jogo *jogo) {
    int pontos = 0;
    if (jogo->jogador.vida > 0) {
        pontos += 1000;
        pontos += jogo->jogador.vida * 200;
        if (jogo->rodada <= 5) pontos += 500;
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
    sprintf(msg, "%s %s | CPU %s", jogo->jogador.nome, acaoParaString(acaoJogador), acaoParaString(acaoComputador));
    definirMensagem(jogo, msg);
    
    if (acaoJogador == CARREGAR) {
        push(&jogo->jogador.municoes, 1);
        jogo->jogador.carregando = true;
    }
    if (acaoComputador == CARREGAR) {
        push(&jogo->computador.municoes, 1);
        jogo->computador.carregando = true;
    }
    if (acaoJogador == DEFENDER) jogo->jogador.defendendo = true;
    if (acaoComputador == DEFENDER) jogo->computador.defendendo = true;
    
    bool jogadorAtirou = false;
    bool cpuAtirou = false;
    
    if (acaoJogador == ATIRAR && !isEmpty(&jogo->jogador.municoes)) {
        pop(&jogo->jogador.municoes);
        jogo->jogador.atirando = true;
        jogadorAtirou = true;
    }
    
    if (acaoJogador == SUPER_TIRO && tamanho(&jogo->jogador.municoes) >= MUNICOES_SUPER_TIRO) {
        for (int i = 0; i < MUNICOES_SUPER_TIRO; i++) pop(&jogo->jogador.municoes);
        jogo->jogador.superTiro = true;
        jogadorAtirou = true;
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

void inicializarJogo(Jogo *jogo) {
    strcpy(jogo->jogador.nome, "Jogador");
    jogo->jogador.vida = 3;
    criarPilha(&jogo->jogador.municoes);
    jogo->jogador.posX = 200;
    jogo->jogador.posY = 400;
    
    strcpy(jogo->computador.nome, "CPU");
    jogo->computador.vida = 3;
    criarPilha(&jogo->computador.municoes);
    jogo->computador.posX = 950;
    jogo->computador.posY = 400;
    
    jogo->bala.ativa = false;
    jogo->bala.tipo = TIRO_NORMAL;
    jogo->bala.animacao = 0;
    
    jogo->rodada = 0;
    jogo->estado = MENU_PRINCIPAL;
    jogo->rankingHead = NULL;
    strcpy(jogo->nomeInput, "");
    jogo->inputIndex = 0;
}

void desenharCoracao(int x, int y, bool cheio) {
    Color cor = cheio ? RED : DARKGRAY;
    DrawCircle(x - 6, y - 3, 8, cor);
    DrawCircle(x + 6, y - 3, 8, cor);
}

void desenharBala(Jogo *jogo) {
    if (!jogo->bala.ativa) return;
    
    if (jogo->bala.tipo == TIRO_SUPER) {
        float raio = 15 + sin(jogo->bala.animacao * 5) * 3;
        DrawCircle((int)jogo->bala.x, (int)jogo->bala.y, raio, (Color){255, 100, 0, 255});
        DrawCircle((int)jogo->bala.x, (int)jogo->bala.y, raio * 0.7f, (Color){255, 200, 0, 255});
        for (int i = 0; i < 8; i++) {
            float angulo = (jogo->bala.animacao + i * 0.785f);
            float dist = raio + sin(angulo * 3) * 5;
            float fx = jogo->bala.x + cos(angulo) * dist;
            float fy = jogo->bala.y + sin(angulo) * dist;
            DrawCircle((int)fx, (int)fy, 5, (Color){255, 150, 0, 180});
        }
    } else {
        DrawCircle((int)jogo->bala.x, (int)jogo->bala.y, 8, ORANGE);
        DrawCircle((int)jogo->bala.x, (int)jogo->bala.y, 5, YELLOW);
    }
}

void desenharPersonagem(float x, float y, Color cor, bool defendendo, bool atirando, bool carregando, bool superTiro, bool olharEsquerda) {
    DrawCircle((int)x, (int)y - 40, 25, cor);
    DrawRectangle((int)x - 15, (int)y - 15, 30, 50, cor);
    
    if (defendendo) {
        DrawCircle((int)x, (int)y, 35, (Color){100, 100, 255, 150});
        DrawCircleLines((int)x, (int)y, 35, BLUE);
    } else if (superTiro) {
        if (olharEsquerda) {
            DrawRectangle((int)x - 60, (int)y - 10, 50, 10, cor);
            DrawRectangle((int)x - 80, (int)y - 10, 25, 8, ORANGE);
        } else {
            DrawRectangle((int)x + 10, (int)y - 10, 50, 10, cor);
            DrawRectangle((int)x + 60, (int)y - 10, 25, 8, ORANGE);
        }
    } else if (atirando) {
        if (olharEsquerda) {
            DrawRectangle((int)x - 50, (int)y - 10, 40, 8, cor);
            DrawRectangle((int)x - 70, (int)y - 8, 20, 5, DARKGRAY);
        } else {
            DrawRectangle((int)x + 10, (int)y - 10, 40, 8, cor);
            DrawRectangle((int)x + 50, (int)y - 8, 20, 5, DARKGRAY);
        }
    } else if (carregando) {
        DrawRectangle((int)x - 10, (int)y - 40, 8, 30, cor);
        DrawRectangle((int)x + 2, (int)y - 40, 8, 30, cor);
        DrawRectangle((int)x - 5, (int)y - 45, 10, 15, DARKGRAY);
    } else {
        DrawRectangle((int)x - 25, (int)y, 15, 8, cor);
        DrawRectangle((int)x + 10, (int)y, 15, 8, cor);
    }
    
    DrawRectangle((int)x - 10, (int)y + 35, 8, 30, cor);
    DrawRectangle((int)x + 2, (int)y + 35, 8, 30, cor);
}

void desenharMenu(Jogo *jogo) {
    (void)jogo;
    ClearBackground((Color){20, 20, 30, 255});
    DrawText("JOGO 007", SCREEN_WIDTH/2 - 180, 80, 80, GOLD);
    DrawRectangle(SCREEN_WIDTH/2 - 200, 280, 400, 70, DARKGREEN);
    DrawText("1 - JOGAR", SCREEN_WIDTH/2 - 80, 305, 30, WHITE);
    DrawRectangle(SCREEN_WIDTH/2 - 200, 370, 400, 70, DARKBLUE);
    DrawText("2 - REGRAS", SCREEN_WIDTH/2 - 90, 395, 30, WHITE);
    DrawRectangle(SCREEN_WIDTH/2 - 200, 460, 400, 70, MAROON);
    DrawText("ESC - SAIR", SCREEN_WIDTH/2 - 80, 485, 30, WHITE);
}

void desenharEntradaNome(Jogo *jogo) {
    ClearBackground((Color){25, 25, 35, 255});
    DrawText("Digite seu nome:", SCREEN_WIDTH/2 - 150, 250, 35, YELLOW);
    DrawRectangle(SCREEN_WIDTH/2 - 200, 320, 400, 60, DARKGRAY);
    if (strlen(jogo->nomeInput) > 0) {
        DrawText(jogo->nomeInput, SCREEN_WIDTH/2 - 180, 335, 30, WHITE);
    }
    DrawText("ENTER para continuar", SCREEN_WIDTH/2 - 120, 450, 20, LIGHTGRAY);
}

void desenharRegras(Jogo *jogo) {
    (void)jogo;
    ClearBackground((Color){25, 25, 35, 255});
    DrawText("REGRAS DO JOGO 007", SCREEN_WIDTH/2 - 200, 30, 45, GOLD);
    int y = 100;
    DrawText("Q - CARREGAR | W - ATIRAR | E - DEFENDER", 100, y, 20, WHITE);
    y += 35;
    DrawText("R - SUPER TIRO (usa 3 balas, ignora escudo!)", 100, y, 20, ORANGE);
    y += 50;
    DrawText("Voce tem 2 segundos para escolher", 100, y, 20, RED);
    DrawRectangle(SCREEN_WIDTH/2 - 150, 620, 300, 50, DARKGREEN);
    DrawText("O - Menu", SCREEN_WIDTH/2 - 50, 635, 20, WHITE);
}

void desenharScore(Jogo *jogo) {
    ClearBackground((Color){15, 15, 25, 255});
    bool vitoria = (jogo->jogador.vida > 0);
    if (vitoria) {
        DrawText("VITORIA!", SCREEN_WIDTH/2 - 140, 100, 65, GREEN);
    } else {
        DrawText("DERROTA!", SCREEN_WIDTH/2 - 140, 100, 65, RED);
    }
    DrawText(TextFormat("Pontuacao: %d", jogo->pontuacaoFinal), SCREEN_WIDTH/2 - 140, 200, 40, GOLD);
    DrawText(TextFormat("Rodadas: %d", jogo->rodada), SCREEN_WIDTH/2 - 90, 260, 25, WHITE);
    DrawText("ENTER - Jogar | O - Menu", SCREEN_WIDTH/2 - 150, 500, 24, WHITE);
}

void desenharJogo(Jogo *jogo) {
    ClearBackground((Color){35, 35, 45, 255});
    DrawText(TextFormat("RODADA %d", jogo->rodada), SCREEN_WIDTH/2 - 80, 20, 30, YELLOW);
    
    if (jogo->estado == ESPERANDO_007) {
        const char *texto = "007";
        if (jogo->contagemNum == 1) texto = "0...";
        else if (jogo->contagemNum == 2) texto = "00...";
        else if (jogo->contagemNum == 3) texto = "007!";
        DrawText(texto, SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 - 100, 120, RED);
    }
    
    DrawRectangle(50, 70, 350, 150, (Color){50, 35, 25, 200});
    DrawText(jogo->jogador.nome, 120, 80, 24, GOLD);
    DrawText("Vidas:", 70, 110, 20, WHITE);
    for (int i = 0; i < 3; i++) desenharCoracao(150 + i * 35, 120, i < jogo->jogador.vida);
    DrawText(TextFormat("Balas: %d/%d", tamanho(&jogo->jogador.municoes), MAX_MUNICOES), 70, 140, 20, WHITE);
    if (podeUsarSuperTiro(&jogo->jogador)) {
        DrawText("SUPER!", 250, 140, 18, ORANGE);
    }
    
    desenharPersonagem(jogo->jogador.posX, jogo->jogador.posY, BLUE, jogo->jogador.defendendo, jogo->jogador.atirando, jogo->jogador.carregando, jogo->jogador.superTiro, false);
    
    DrawRectangle(800, 70, 350, 150, (Color){50, 35, 25, 200});
    DrawText("CPU", 970, 80, 24, GOLD);
    DrawText("Vidas:", 820, 110, 20, WHITE);
    for (int i = 0; i < 3; i++) desenharCoracao(900 + i * 35, 120, i < jogo->computador.vida);
    DrawText(TextFormat("Balas: %d/%d", tamanho(&jogo->computador.municoes), MAX_MUNICOES), 820, 140, 20, WHITE);
    
    desenharPersonagem(jogo->computador.posX, jogo->computador.posY, RED, jogo->computador.defendendo, jogo->computador.atirando, jogo->computador.carregando, jogo->computador.superTiro, true);
    
    DrawText("VS", SCREEN_WIDTH/2 - 35, 300, 50, YELLOW);
    desenharBala(jogo);
    
    if (jogo->estado == ESCOLHENDO_ACAO && !jogo->jogadorEscolheu) {
        DrawText(TextFormat("Tempo: %.1f s", jogo->tempoContagem), SCREEN_WIDTH/2 - 70, 60, 24, RED);
        DrawRectangle(SCREEN_WIDTH/2 - 280, 580, 100, 50, DARKGREEN);
        DrawText("Q-Carregar", SCREEN_WIDTH/2 - 270, 595, 14, WHITE);
        DrawRectangle(SCREEN_WIDTH/2 - 160, 580, 90, 50, RED);
        DrawText("W-Atirar", SCREEN_WIDTH/2 - 150, 595, 14, WHITE);
        DrawRectangle(SCREEN_WIDTH/2 - 50, 580, 100, 50, BLUE);
        DrawText("E-Defender", SCREEN_WIDTH/2 - 40, 595, 14, WHITE);
        if (podeUsarSuperTiro(&jogo->jogador)) {
            DrawRectangle(SCREEN_WIDTH/2 + 70, 580, 100, 50, ORANGE);
            DrawText("R-SUPER", SCREEN_WIDTH/2 + 80, 595, 14, WHITE);
        }
    }
    
    if (jogo->tempoMensagem > 0) {
        DrawRectangle(SCREEN_WIDTH/2 - 350, 240, 700, 60, (Color){0, 0, 0, 230});
        DrawText(jogo->mensagem, SCREEN_WIDTH/2 - 330, 255, 24, YELLOW);
    }
}

int main(void) {
    srand((unsigned int)time(NULL));
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Jogo 007 - Commit 4");
    SetTargetFPS(60);
    
    Jogo jogo;
    inicializarJogo(&jogo);
    
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        
        if (jogo.tempoMensagem > 0) jogo.tempoMensagem -= dt;
        if (jogo.bala.ativa) atualizarBala(&jogo);
        
        switch (jogo.estado) {
            case MENU_PRINCIPAL:
                if (IsKeyPressed(KEY_ONE)) jogo.estado = ENTRADA_NOME;
                if (IsKeyPressed(KEY_TWO)) jogo.estado = REGRAS;
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
                if (IsKeyPressed(KEY_BACKSPACE) && jogo.inputIndex > 0) {
                    jogo.inputIndex--;
                    jogo.nomeInput[jogo.inputIndex] = '\0';
                }
                if (IsKeyPressed(KEY_O)) jogo.estado = MENU_PRINCIPAL;
                if (IsKeyPressed(KEY_ENTER) && strlen(jogo.nomeInput) > 0) {
                    strcpy(jogo.jogador.nome, jogo.nomeInput);
                    jogo.estado = ESPERANDO_007;
                    jogo.tempoContagem = 1.0f;
                    jogo.contagemNum = 1;
                }
            } break;
            case REGRAS:
                if (IsKeyPressed(KEY_O)) jogo.estado = MENU_PRINCIPAL;
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
                break;
            case ESCOLHENDO_ACAO:
                if (!jogo.jogadorEscolheu) {
                    if (IsKeyPressed(KEY_Q)) { jogo.jogador.acaoEscolhida = CARREGAR; jogo.jogadorEscolheu = true; }
                    if (IsKeyPressed(KEY_W)) { jogo.jogador.acaoEscolhida = ATIRAR; jogo.jogadorEscolheu = true; }
                    if (IsKeyPressed(KEY_E)) { jogo.jogador.acaoEscolhida = DEFENDER; jogo.jogadorEscolheu = true; }
                    if (IsKeyPressed(KEY_R) && podeUsarSuperTiro(&jogo.jogador)) { jogo.jogador.acaoEscolhida = SUPER_TIRO; jogo.jogadorEscolheu = true; }
                }
                jogo.tempoContagem -= dt;
                if (jogo.tempoContagem <= 0) {
                    if (!jogo.jogadorEscolheu) jogo.jogador.acaoEscolhida = NENHUMA;
                    jogo.estado = PROCESSANDO_TURNO;
                }
                break;
            case PROCESSANDO_TURNO:
                processarTurno(&jogo);
                jogo.tempoContagem = 2.0f;
                jogo.estado = JOGANDO;
                break;
            case JOGANDO:
                jogo.tempoContagem -= dt;
                if (jogo.tempoContagem <= 0) {
                    if (jogo.jogador.vida <= 0 || jogo.computador.vida <= 0) {
                        jogo.pontuacaoFinal = calcularPontuacao(&jogo);
                        inserirRanking(&jogo.rankingHead, jogo.jogador.nome, jogo.pontuacaoFinal);
                        jogo.estado = SCORE;
                    } else {
                        jogo.estado = ESPERANDO_007;
                        jogo.tempoContagem = 1.0f;
                        jogo.contagemNum = 1;
                    }
                }
                break;
            case SCORE:
                if (IsKeyPressed(KEY_ENTER)) {
                    jogo.inputIndex = 0;
                    strcpy(jogo.nomeInput, "");
                    jogo.estado = ENTRADA_NOME;
                }
                if (IsKeyPressed(KEY_O)) jogo.estado = MENU_PRINCIPAL;
                break;
        }
        
        BeginDrawing();
        switch (jogo.estado) {
            case MENU_PRINCIPAL: desenharMenu(&jogo); break;
            case ENTRADA_NOME: desenharEntradaNome(&jogo); break;
            case REGRAS: desenharRegras(&jogo); break;
            case SCORE: desenharScore(&jogo); break;
            default: desenharJogo(&jogo); break;
        }
        EndDrawing();
    }
    
    limparRanking(&jogo.rankingHead);
    CloseWindow();
    return 0;
}