# 🔫 Jogo 007 - Duelo de Reflexo e Estratégia 💥

Um emocionante jogo de duelo rápido e estratégico, implementado em C com a biblioteca **raylib**, onde você enfrenta a CPU em um combate de ação e reação.

## 📝 Sobre o Jogo

O **Jogo 007** é um duelo por turnos onde a gestão de recursos (munições) e a rapidez na escolha de ações são cruciais. Você e a CPU têm um tempo limitado para decidir entre carregar sua arma, atirar, defender-se ou lançar um poderoso Super Tiro.

---

## 🕹️ Ações e Comandos

Você tem apenas **2.0 segundos** por rodada para escolher sua ação.

| Ação | Tecla | Efeito | Requisito de Munição |
| :--- | :---: | :--- | :--- |
| **Carregar** | `Q` | Adiciona **1 munição** à sua pilha. | Nenhuma |
| **Atirar** | `W` | Dispara **1 bala** no adversário. | 1 ou mais |
| **Defender** | `E` | Bloqueia um **Tiro Normal** do adversário nesta rodada. | Nenhuma |
| **Super Tiro** | `R` | Dispara um tiro que **ignora a defesa** do adversário, garantindo acerto de dano. | 3 ou mais |

---

## 🎲 Regras do Jogo

### 1. Vida e Munição
* Cada jogador (você e a CPU) começa com **3 vidas**.
* A munição máxima que você pode carregar é **6 balas**.
* Um tiro normal acerta se o oponente não estiver defendendo.
* O Super Tiro acerta sempre, mesmo se o oponente estiver defendendo.

### 2. Turnos e Contagem
* O jogo é dividido em rodadas rápidas, marcadas pela contagem **"007!"**.
* Você deve escolher sua ação rapidamente, pois o tempo de escolha é limitado.
* Se você não escolher uma ação a tempo, a ação padrão será **NENHUMA** (não carrega, não atira, não defende).

### 3. Vencer o Jogo
* O jogo termina quando a vida de um dos jogadores chega a zero.
* Sua pontuação final é calculada com base na sua sobrevivência e no número de rodadas.

---

## 👨‍💻 Equipe de Desenvolvimento

Este projeto foi desenvolvido pela seguinte equipe:

* **Rodrigo Tenório**
* **Gabriel Ferraz**
* **Paulo Portella**
* **Ícaro Barros**

---

## 🚀 Como Jogar

1.  **Menu Principal:**
    * Pressione **1** para iniciar o jogo.
    * Pressione **2** para visualizar as regras.
    * Pressione **ESC** para fechar o jogo.
    * Pressione **O** para retornar a tela inicial.
2.  **Entrada de Nome:** Digite seu nome e pressione **ENTER**.
3.  **Duelo:** Utilize as teclas `Q`, `W`, `E` e `R` para escolher sua ação antes que o tempo acabe.
4.  **Fim do Jogo:**
    * Na tela de pontuação (`SCORE`), pressione **ENTER** para recomeçar o duelo (entrando o nome novamente) ou **p** para pausar o jogo ,e **O** para voltar ao Menu Principal.

---

## Video do jogo funcionando.

* [![Vídeo de demonstração](https://img.youtube.com/vi/3scZY5xjT50/0.jpg)](https://youtu.be/3scZY5xjT50)