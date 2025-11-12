#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <raylib.h>

#define MAX_MUNICOES 6
#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 700
#define MAX_NOME 30
#define TEMPO_ESCOLHA 2.0f

typedef enum {
    CARREGAR,
    ATIRAR,
    DEFENDER,
    NENHUMA
} Acao;

typedef enum {
    MENU_PRINCIPAL,
    ENTRADA_NOME,
    ESPERANDO_007,
    ESCOLHENDO_ACAO,
    PROCESSANDO_TURNO,
    JOGANDO
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
    bool defendendo;
    bool atirando;
    bool carregando;
} Jogador;

typedef struct {
    Jogador jogador;
    Jogador computador;
    int rodada;
    EstadoJogo estado;
    char mensagem[256];
    float tempoMensagem;
    float tempoContagem;
    int contagemNum;
    char nomeInput[MAX_NOME];
    int inputIndex;
    bool jogadorEscolheu;
} Jogo;

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

const char* acaoParaString(Acao acao) {
    switch (acao) {
        case CARREGAR: return "CARREGOU";
        case ATIRAR: return "ATIROU";
        case DEFENDER: return "DEFENDEU";
        case NENHUMA: return "NAO FEZ NADA";
        default: return "---";
    }
}

void definirMensagem(Jogo *jogo, const char *msg) {
    strcpy(jogo->mensagem, msg);
    jogo->tempoMensagem = 3.0f;
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
    
    return CARREGAR;
}

void processarTurno(Jogo *jogo) {
    Acao acaoJogador = jogo->jogador.acaoEscolhida;
    Acao acaoComputador = jogo->computador.acaoEscolhida;
    
    jogo->jogador.defendendo = false;
    jogo->jogador.atirando = false;
    jogo->jogador.carregando = false;
    jogo->computador.defendendo = false;
    jogo->computador.atirando = false;
    jogo->computador.carregando = false;
    
    char msg[256];
    sprintf(msg, "%s %s | CPU %s", 
            jogo->jogador.nome, 
            acaoParaString(acaoJogador),
            acaoParaString(acaoComputador));
    definirMensagem(jogo, msg);
    
    if (acaoJogador == CARREGAR) {
        push(&jogo->jogador.municoes, 1);
        jogo->jogador.carregando = true;
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
    
    if (acaoComputador == ATIRAR && !isEmpty(&jogo->computador.municoes)) {
        pop(&jogo->computador.municoes);
        jogo->computador.atirando = true;
        cpuAtirou = true;
    }
    
    if (jogadorAtirou && !jogo->computador.defendendo) {
        jogo->computador.vida--;
    }
    
    if (cpuAtirou && !jogo->jogador.defendendo) {
        jogo->jogador.vida--;
    }
}

void inicializarJogo(Jogo *jogo) {
    strcpy(jogo->jogador.nome, "Jogador");
    jogo->jogador.vida = 3;
    criarPilha(&jogo->jogador.municoes);
    jogo->jogador.acaoEscolhida = NENHUMA;
    jogo->jogador.defendendo = false;
    jogo->jogador.atirando = false;
    jogo->jogador.carregando = false;
    
    strcpy(jogo->computador.nome, "CPU");
    jogo->computador.vida = 3;
    criarPilha(&jogo->computador.municoes);
    jogo->computador.acaoEscolhida = NENHUMA;
    jogo->computador.defendendo = false;
    jogo->computador.atirando = false;
    jogo->computador.carregando = false;
    
    jogo->rodada = 0;
    jogo->estado = MENU_PRINCIPAL;
    jogo->tempoMensagem = 0;
    jogo->tempoContagem = 0;
    jogo->contagemNum = 0;
    jogo->jogadorEscolheu = false;
    strcpy(jogo->mensagem, "");
    strcpy(jogo->nomeInput, "");
    jogo->inputIndex = 0;
}

void resetarJogo(Jogo *jogo) {
    jogo->jogador.vida = 3;
    criarPilha(&jogo->jogador.municoes);
    jogo->jogador.acaoEscolhida = NENHUMA;
    jogo->jogador.defendendo = false;
    jogo->jogador.atirando = false;
    jogo->jogador.carregando = false;
    
    jogo->computador.vida = 3;
    criarPilha(&jogo->computador.municoes);
    jogo->computador.acaoEscolhida = NENHUMA;
    jogo->computador.defendendo = false;
    jogo->computador.atirando = false;
    jogo->computador.carregando = false;
    
    jogo->rodada = 0;
    jogo->tempoContagem = 0;
    jogo->contagemNum = 0;
    jogo->jogadorEscolheu = false;
    strcpy(jogo->mensagem, "");
    jogo->tempoMensagem = 0;
}

void desenharMenu(Jogo *jogo) {
    (void)jogo;
    ClearBackground((Color){20, 20, 30, 255});
    
    DrawText("JOGO 007", SCREEN_WIDTH/2 - 180, 80, 80, GOLD);
    DrawText("Duelo de Agentes Secretos", SCREEN_WIDTH/2 - 220, 170, 30, LIGHTGRAY);
    
    DrawRectangle(SCREEN_WIDTH/2 - 200, 280, 400, 70, DARKGREEN);
    DrawText("1 - JOGAR", SCREEN_WIDTH/2 - 80, 305, 30, WHITE);
    
    DrawRectangle(SCREEN_WIDTH/2 - 200, 370, 400, 70, MAROON);
    DrawText("ESC - SAIR", SCREEN_WIDTH/2 - 80, 395, 30, WHITE);
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
    
    DrawText("Pressione ENTER", SCREEN_WIDTH/2 - 100, 450, 22, LIGHTGRAY);
}

void desenharJogo(Jogo *jogo) {
    ClearBackground((Color){35, 35, 45, 255});
    
    DrawText(TextFormat("RODADA %d", jogo->rodada), SCREEN_WIDTH/2 - 80, 20, 30, YELLOW);
    
    if (jogo->estado == ESPERANDO_007) {
        int tamanhoTexto = 120;
        const char *texto = "007";
        if (jogo->contagemNum == 1) texto = "0...";
        else if (jogo->contagemNum == 2) texto = "00...";
        else if (jogo->contagemNum == 3) texto = "007!";
        
        DrawText(texto, SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 - 100, tamanhoTexto, RED);
    }
    
    DrawText(TextFormat("%s | Vida: %d | Balas: %d", 
             jogo->jogador.nome, jogo->jogador.vida, tamanho(&jogo->jogador.municoes)), 
             50, 100, 22, WHITE);
    
    DrawText(TextFormat("CPU | Vida: %d | Balas: %d", 
             jogo->computador.vida, tamanho(&jogo->computador.municoes)), 
             700, 100, 22, WHITE);
    
    if (jogo->estado == ESCOLHENDO_ACAO && !jogo->jogadorEscolheu) {
        DrawText(TextFormat("Tempo: %.1f s", jogo->tempoContagem), SCREEN_WIDTH/2 - 70, 60, 24, RED);
        
        DrawRectangle(SCREEN_WIDTH/2 - 250, 580, 150, 50, DARKGREEN);
        DrawText("Q-Carregar", SCREEN_WIDTH/2 - 235, 595, 16, WHITE);
        
        DrawRectangle(SCREEN_WIDTH/2 - 75, 580, 150, 50, RED);
        DrawText("W-Atirar", SCREEN_WIDTH/2 - 55, 595, 16, WHITE);
        
        DrawRectangle(SCREEN_WIDTH/2 + 100, 580, 150, 50, BLUE);
        DrawText("E-Defender", SCREEN_WIDTH/2 + 110, 595, 16, WHITE);
    }
    
    if (jogo->tempoMensagem > 0) {
        DrawRectangle(SCREEN_WIDTH/2 - 350, 300, 700, 70, (Color){0, 0, 0, 230});
        DrawText(jogo->mensagem, SCREEN_WIDTH/2 - 330, 315, 26, YELLOW);
    }
    
    if (jogo->jogador.vida <= 0 || jogo->computador.vida <= 0) {
        if (jogo->jogador.vida > 0) {
            DrawText("VITORIA!", SCREEN_WIDTH/2 - 120, 400, 60, GREEN);
        } else {
            DrawText("DERROTA!", SCREEN_WIDTH/2 - 120, 400, 60, RED);
        }
        DrawText("Pressione O para Menu", SCREEN_WIDTH/2 - 150, 500, 24, WHITE);
    }
}

int main(void) {
    srand((unsigned int)time(NULL));
    
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Jogo 007 - Commit 2");
    SetTargetFPS(60);
    
    Jogo jogo;
    inicializarJogo(&jogo);
    
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        
        if (jogo.tempoMensagem > 0) {
            jogo.tempoMensagem -= dt;
        }
        
        switch (jogo.estado) {
            case MENU_PRINCIPAL:
                if (IsKeyPressed(KEY_ONE)) {
                    jogo.estado = ENTRADA_NOME;
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
                
                if (IsKeyPressed(KEY_BACKSPACE) && jogo.inputIndex > 0) {
                    jogo.inputIndex--;
                    jogo.nomeInput[jogo.inputIndex] = '\0';
                }
                
                if (IsKeyPressed(KEY_ENTER) && strlen(jogo.nomeInput) > 0) {
                    strcpy(jogo.jogador.nome, jogo.nomeInput);
                    resetarJogo(&jogo);
                    jogo.estado = ESPERANDO_007;
                    jogo.tempoContagem = 1.0f;
                    jogo.contagemNum = 1;
                }
            } break;
                
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
                }
                
                jogo.tempoContagem -= dt;
                if (jogo.tempoContagem <= 0) {
                    if (!jogo.jogadorEscolheu) {
                        jogo.jogador.acaoEscolhida = NENHUMA;
                    }
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
                        if (IsKeyPressed(KEY_O)) {
                            jogo.estado = MENU_PRINCIPAL;
                            inicializarJogo(&jogo);
                        }
                    } else {
                        jogo.estado = ESPERANDO_007;
                        jogo.tempoContagem = 1.0f;
                        jogo.contagemNum = 1;
                    }
                }
                break;
        }
        
        BeginDrawing();
        
        switch (jogo.estado) {
            case MENU_PRINCIPAL: desenharMenu(&jogo); break;
            case ENTRADA_NOME: desenharEntradaNome(&jogo); break;
            default: desenharJogo(&jogo); break;
        }
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}