#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

/*
 * Controle da impressora Elgin (i8 / i9) em C, usando a DLL E1_Impressora01.dll.
 *
 * A DLL precisa estar na RAIZ do projeto (na mesma pasta de onde o programa e executado),
 * junto com os arquivos XMLSAT.xml e CANC_SAT.xml.
 *
 * IMPORTANTE: a DLL precisa ter a mesma arquitetura do compilador:
 *   - compilador de 32 bits (ex.: MinGW do Code::Blocks)  -> use a DLL da pasta x86
 *   - compilador de 64 bits (ex.: MSYS2 mingw64)          -> use a DLL da pasta x64
 */

/* ===================== Variaveis globais do programa ===================== */

static HMODULE hDll = NULL;

static int conectada = 0;                 /* 0 = fechada, 1 = aberta */
static int tipoConexao = 1;               /* 1 = USB, 2 = Serial, 3 = Rede... */
static char modeloImpressora[50] = "i9";  /* "i8" ou "i9" */
static char portaConexao[50] = "USB";
static int parametroExtra = 0;

/* Chave do QR Code usada no cancelamento SAT */
static const char *CHAVE_QRCODE_CANC =
    "Q5DLkpdRijIRGY6YSSNsTWK1TztHL1vD0V1Jc4spo/CEUqICEb9SFy82ym8EhBRZjbh3btsZhF+sjHqEMR159i4agru9x6KsepK/"
    "q0E2e5xlU5cv3m1woYfgHyOkWDNcSdMsS6bBh2Bpq6s89yJ9Q6qh/J8YHi306ce9Tqb/drKvN2XdE5noRSS32TAWuaQEVd7u+"
    "TrvXlOQsE3fHR1D5f1saUwQLPSdIv01NF6Ny7jZwjCwv1uNDgGZONJdlTJ6p0ccqnZvuE70aHOI09elpjEO6Cd+orI7XHHrFCwhFhAcbalc+"
    "ZfO5b/+vkyAHS6CYVFCDtYR9Hi5qgdk31v23w==";

/* ===================== Ligacao com a DLL ===================== */

/* Cada typedef descreve a "forma" de um comando da DLL: o que devolve e quais parametros recebe */
typedef int (*TpAbreConexao)(int tipo, const char *modelo, const char *conexao, int param);
typedef int (*TpFechaConexao)(void);
typedef int (*TpTexto)(const char *dados, int posicao, int estilo, int tamanho);
typedef int (*TpCorte)(int avanco);
typedef int (*TpQRCode)(const char *dados, int tamanho, int nivelCorrecao);
typedef int (*TpCodBarras)(int tipo, const char *dados, int altura, int largura, int HRI);
typedef int (*TpAvancaPapel)(int linhas);
typedef int (*TpStatus)(int param);
typedef int (*TpGavetaElgin)(int pino, int ti, int tf);
typedef int (*TpGaveta)(int pino, int ti, int tf);
typedef int (*TpSinalSonoro)(int qtd, int tempoInicio, int tempoFim);
typedef int (*TpSemParametro)(void);
typedef int (*TpPosicao)(int posicao);
typedef int (*TpXmlSat)(const char *dados, int param);
typedef int (*TpXmlCancSat)(const char *dados, const char *assQRCode, int param);

/* Um ponteiro para cada comando (prefixo "cmd_"). Depois de carregarBiblioteca(), e so chamar. */
static TpAbreConexao cmd_AbreConexao;
static TpFechaConexao cmd_FechaConexao;
static TpTexto cmd_Texto;
static TpCorte cmd_Corte;
static TpQRCode cmd_QRCode;
static TpCodBarras cmd_CodBarras;
static TpAvancaPapel cmd_AvancaPapel;
static TpStatus cmd_Status;
static TpGavetaElgin cmd_GavetaElgin;
static TpGaveta cmd_Gaveta;
static TpSinalSonoro cmd_SinalSonoro;
static TpSemParametro cmd_ModoPagina;
static TpSemParametro cmd_LimpaBufferModoPagina;
static TpSemParametro cmd_ImprimeModoPagina;
static TpSemParametro cmd_ModoPadrao;
static TpPosicao cmd_PosicaoHorizontal;
static TpPosicao cmd_PosicaoVertical;
static TpXmlSat cmd_XmlSat;
static TpXmlCancSat cmd_XmlCancSat;

/* Procura um comando dentro da DLL. Devolve 1 se achou, 0 se nao achou. */
static int buscarComando(void *destino, const char *nomeNaDll) {
    FARPROC endereco = GetProcAddress(hDll, nomeNaDll);
    if (endereco == NULL) {
        printf("Comando '%s' nao encontrado na DLL.\n", nomeNaDll);
        return 0;
    }
    *(FARPROC *)destino = endereco;
    return 1;
}

/* Carrega a DLL da raiz do projeto e todos os comandos. Devolve 1 se deu certo. */
static int carregarBiblioteca(void) {
    hDll = LoadLibraryA("E1_Impressora01.dll");
    if (hDll == NULL) {
        printf("Nao foi possivel carregar E1_Impressora01.dll (erro %lu).\n", GetLastError());
        printf("Confira: a DLL esta na raiz do projeto? Ela tem a mesma arquitetura (32/64 bits) do compilador?\n");
        return 0;
    }

    int tudoOk = 1;
    /* O texto entre aspas e o nome REAL do comando na DLL e nao pode ser alterado */
    tudoOk &= buscarComando(&cmd_AbreConexao, "AbreConexaoImpressora");
    tudoOk &= buscarComando(&cmd_FechaConexao, "FechaConexaoImpressora");
    tudoOk &= buscarComando(&cmd_Texto, "ImpressaoTexto");
    tudoOk &= buscarComando(&cmd_Corte, "Corte");
    tudoOk &= buscarComando(&cmd_AvancaPapel, "AvancaPapel");
    tudoOk &= buscarComando(&cmd_Status, "StatusImpressora");
    tudoOk &= buscarComando(&cmd_QRCode, "ImpressaoQRCode");
    tudoOk &= buscarComando(&cmd_CodBarras, "ImpressaoCodigoBarras");
    tudoOk &= buscarComando(&cmd_XmlSat, "ImprimeXMLSAT");
    tudoOk &= buscarComando(&cmd_XmlCancSat, "ImprimeXMLCancelamentoSAT");
    tudoOk &= buscarComando(&cmd_GavetaElgin, "AbreGavetaElgin");
    tudoOk &= buscarComando(&cmd_Gaveta, "AbreGaveta");
    tudoOk &= buscarComando(&cmd_SinalSonoro, "SinalSonoro");
    tudoOk &= buscarComando(&cmd_ModoPagina, "ModoPagina");
    tudoOk &= buscarComando(&cmd_LimpaBufferModoPagina, "LimpaBufferModoPagina");
    tudoOk &= buscarComando(&cmd_ImprimeModoPagina, "ImprimeModoPagina");
    tudoOk &= buscarComando(&cmd_ModoPadrao, "ModoPadrao");
    tudoOk &= buscarComando(&cmd_PosicaoHorizontal, "PosicaoImpressaoHorizontal");
    tudoOk &= buscarComando(&cmd_PosicaoVertical, "PosicaoImpressaoVertical");

    /* se algum comando nao foi encontrado, libera a DLL antes de devolver falha */
    if (!tudoOk) {
        FreeLibrary(hDll);
        hDll = NULL;
    }
    return tudoOk;
}

/* ===================== Funcoes auxiliares ===================== */

/* Mostra a mensagem e le uma linha de texto digitada (sem o ENTER no final) */
static void lerTexto(const char *pergunta, char *saida, int limite) {
    printf("%s", pergunta);
    if (fgets(saida, limite, stdin) == NULL) {
        saida[0] = '\0';
        return;
    }
    saida[strcspn(saida, "\r\n")] = '\0';
}

/* Mostra a mensagem e le um numero inteiro. Se digitar letras, pergunta de novo. */
static int lerNumero(const char *pergunta) {
    char buffer[50];
    char *resto;
    while (1) {
        lerTexto(pergunta, buffer, sizeof(buffer));
        long numero = strtol(buffer, &resto, 10);
        if (buffer[0] != '\0' && *resto == '\0') {
            return (int)numero;
        }
        printf("Digite apenas numeros!\n");
    }
}

/* Le um arquivo inteiro para a memoria. Devolve NULL se nao achar. Quem chama deve dar free(). */
static char *carregarArquivo(const char *nomeArquivo) {
    FILE *f = fopen(nomeArquivo, "rb");
    if (f == NULL) {
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long bytes = ftell(f);
    rewind(f);

    char *texto = (char *)malloc(bytes + 1);
    if (texto == NULL) {
        fclose(f);
        return NULL;
    }
    size_t lidos = fread(texto, 1, bytes, f);
    texto[lidos] = '\0';
    fclose(f);
    return texto;
}

/* Corta o papel (avanca 5) e confere o retorno do comando Corte. */
static void acionarCorte(void) {
    int resultado = cmd_Corte(5);
    if (resultado == 0) {
        printf("Papel cortado.\n");
    } else {
        printf("Erro ao cortar o papel. Codigo de erro: %d\n", resultado);
    }
}

/* ===================== Login ===================== */

/* Pede usuario e senha (3 tentativas). Devolve 1 se acertou e 0 se errou as 3 vezes. */
static int autenticar(void) {
    char nomeUsuario[50], senhaDigitada[50];
    int n;

    printf("\n==================== LOGIN ====================\n");
    for (n = 1; n <= 3; n++) {
        lerTexto("Usuario: ", nomeUsuario, sizeof(nomeUsuario));
        lerTexto("Senha: ", senhaDigitada, sizeof(senhaDigitada));

        /* textos se comparam com strcmp, nunca com == */
        if (strcmp(nomeUsuario, "admin") == 0 && strcmp(senhaDigitada, "1234") == 0) {
            printf("Login realizado com sucesso!\n");
            return 1;
        }

        printf("Usuario ou senha incorretos. Tentativas restantes: %d\n", 3 - n);
    }

    /* chegou aqui: errou as 3 vezes */
    printf("Numero maximo de tentativas atingido.\n");
    return 0;
}

/* ===================== Conexao ===================== */

static void definirParametrosConexao(void) {
    if (!conectada) {
        tipoConexao = lerNumero("Digite o tipo de conexao (1 para USB, 2 para Serial, etc.): ");
        lerTexto("Digite o modelo da impressora: ", modeloImpressora, sizeof(modeloImpressora));
        lerTexto("Digite a porta de conexao (ex: USB): ", portaConexao, sizeof(portaConexao));
        parametroExtra = lerNumero("Digite o parametro adicional (ex: 0 para padrao): ");
        printf("Parametros de conexao configurados com sucesso.\n");
    } else {
        printf("Conexao ja configurada. Pronta para uso.\n");
    }
}

static void conectar(void) {
    if (!conectada) {
        int resultado = cmd_AbreConexao(tipoConexao, modeloImpressora, portaConexao, parametroExtra);
        if (resultado == 0) {
            conectada = 1;
            printf("Conexao aberta com sucesso.\n");
        } else {
            printf("Erro ao abrir conexao. Codigo de erro: %d\n", resultado);
        }
    } else {
        printf("Conexao ja esta aberta.\n");
    }
}

static void desconectar(void) {
    /* so fecha se estiver aberta */
    if (!conectada) {
        printf("Conexao ja esta fechada.\n");
        return;
    }

    int resultado = cmd_FechaConexao();
    if (resultado == 0) {
        conectada = 0;
        printf("Conexao fechada com sucesso.\n");
    } else {
        printf("Erro ao fechar conexao. Codigo de erro: %d\n", resultado);
    }
}

/* ===================== Gaveta e sinal sonoro ===================== */

static void acionarGavetaElgin(void) {
    if (!conectada) {
        printf("Conexao fechada. Abra a conexao antes (opcao 2).\n");
        return;
    }

    int resultado = cmd_GavetaElgin(1, 50, 50);
    if (resultado == 0) {
        printf("Gaveta (Elgin) aberta!\n");
    } else {
        printf("Erro ao abrir a gaveta (Elgin). Codigo: %d\n", resultado);
    }
}

static void acionarGaveta(void) {
    if (!conectada) {
        printf("Conexao fechada. Abra a conexao antes (opcao 2).\n");
        return;
    }

    int resultado = cmd_Gaveta(1, 5, 10);
    if (resultado == 0) {
        printf("Gaveta aberta!\n");
    } else {
        printf("Erro ao abrir a gaveta. Codigo: %d\n", resultado);
    }
}

static void tocarBeep(void) {
    if (!conectada) {
        printf("Conexao fechada. Abra a conexao antes (opcao 2).\n");
        return;
    }

    /* a DLL aceita de 1 a 63 apitos */
    int apitos = lerNumero("Quantos apitos (1 a 63)? ");
    if (apitos < 1 || apitos > 63) {
        printf("Quantidade invalida. Digite um numero de 1 a 63.\n");
        return;
    }

    int resultado = cmd_SinalSonoro(apitos, 50, 5);
    if (resultado == 0) {
        printf("Sinal sonoro emitido!\n");
    } else {
        printf("Erro ao emitir o sinal sonoro. Codigo: %d\n", resultado);
    }
}

/* ===================== Impressoes ===================== */

static void imprimirTextoSimples(void) {
    /* 1) confere a conexao ANTES de perguntar qualquer coisa */
    if (!conectada) {
        printf("Conexao fechada. Abra a conexao antes (opcao 2).\n");
        return;
    }

    /* 2) pergunta o que e preciso */
    char conteudo[200];
    lerTexto("Texto a imprimir: ", conteudo, sizeof(conteudo));
    if (conteudo[0] == '\0') {
        printf("Nenhum texto foi digitado. Nada foi impresso.\n");
        return;
    }
    int qtdCopias = lerNumero("Quantas copias? ");
    if (qtdCopias < 1) {
        printf("A quantidade de copias precisa ser 1 ou mais.\n");
        return;
    }

    /* 3) imprime cada copia e 4) confere o retorno (para ImpressaoTexto, erro = retorno < 0) */
    int totalImpressas = 0;
    int c;
    for (c = 1; c <= qtdCopias; c++) {
        int resultado = cmd_Texto(conteudo, 1, 4, 0);
        if (resultado >= 0) {
            printf("Copia %d impressa!\n", c);
            totalImpressas++;
        } else {
            printf("Erro ao imprimir a copia %d. Codigo: %d\n", c, resultado);
            break;  /* para no primeiro erro, para nao repetir a mesma mensagem */
        }
    }

    /* 5) corta o papel se pelo menos uma copia foi impressa */
    if (totalImpressas > 0) {
        acionarCorte();
    }
}

static void imprimirCodigoQR(void) {
    if (!conectada) {
        printf("Conexao fechada. Abra a conexao antes (opcao 2).\n");
        return;
    }

    char conteudo[200];
    lerTexto("Conteudo do QR Code: ", conteudo, sizeof(conteudo));
    if (conteudo[0] == '\0') {
        printf("Nenhum conteudo foi digitado. Nada foi impresso.\n");
        return;
    }

    int resultado = cmd_QRCode(conteudo, 6, 4);
    if (resultado == 0) {
        printf("QR Code impresso!\n");
        acionarCorte();
    } else {
        printf("Erro ao imprimir QR Code. Codigo: %d\n", resultado);
    }
}

static void imprimirBarras(void) {
    if (!conectada) {
        printf("Conexao fechada. Abra a conexao antes (opcao 2).\n");
        return;
    }

    /* tipo 8 = CODE 128 (o texto comeca com {A), altura 100, largura 2, HRI 3 = numero acima e abaixo */
    int resultado = cmd_CodBarras(8, "{A012345678912", 100, 2, 3);
    if (resultado == 0) {
        printf("Codigo de barras impresso!\n");
        acionarCorte();
    } else {
        printf("Erro ao imprimir codigo de barras. Codigo: %d\n", resultado);
    }
}

static void imprimirCupomSAT(void) {
    if (!conectada) {
        printf("Conexao fechada. Abra a conexao antes (opcao 2).\n");
        return;
    }

    /* le o arquivo XML (na raiz do projeto) para a memoria */
    char *conteudoXML = carregarArquivo("XMLSAT.xml");
    if (conteudoXML == NULL) {
        printf("Nao foi possivel ler o XMLSAT.xml\n");
        return;
    }

    int resultado = cmd_XmlSat(conteudoXML, 0);
    free(conteudoXML);  /* devolve a memoria */

    if (resultado == 0) {
        printf("XML SAT impresso!\n");
        acionarCorte();
    } else {
        printf("Erro ao imprimir XML SAT. Codigo: %d\n", resultado);
    }
}

static void imprimirCancelamentoSAT(void) {
    if (!conectada) {
        printf("Conexao fechada. Abra a conexao antes (opcao 2).\n");
        return;
    }

    /* le o arquivo XML (na raiz do projeto) para a memoria */
    char *conteudoXML = carregarArquivo("CANC_SAT.xml");
    if (conteudoXML == NULL) {
        printf("Nao foi possivel ler o CANC_SAT.xml\n");
        return;
    }

    int resultado = cmd_XmlCancSat(conteudoXML, CHAVE_QRCODE_CANC, 0);
    free(conteudoXML);  /* devolve a memoria */

    if (resultado == 0) {
        printf("XML de cancelamento SAT impresso!\n");
    } else {
        printf("Erro ao imprimir XML de cancelamento SAT. Codigo: %d\n", resultado);
    }
}

/* ===================== Menu ===================== */

static void mostrarMenu(void) {
    printf("\n*************************************************\n");
    printf("**************** MENU IMPRESSORA ****************\n");
    printf("*************************************************\n\n");

    printf("1  - Configurar Conexao\n");
    printf("2  - Abrir Conexao\n");
    printf("3  - Fechar Conexao\n");
    printf("4  - Imprimir Texto\n");
    printf("5  - Imprimir QR Code\n");
    printf("6  - Imprimir Codigo de Barras\n");
    printf("7  - Imprimir XML SAT\n");
    printf("8  - Imprimir XML de Cancelamento SAT\n");
    printf("9  - Abrir Gaveta (Elgin)\n");
    printf("10 - Abrir Gaveta\n");
    printf("11 - Sinal Sonoro\n");
    printf("0  - Sair\n");
    printf("--------------------------------------\n");
}

/* ===================== Programa principal ===================== */

int main(void) {
    if (!carregarBiblioteca()) {
        printf("Programa encerrado.\n");
        return 1;
    }

    if (!autenticar()) {
        printf("Acesso negado.\n");
        FreeLibrary(hDll);
        return 1;
    }

    int escolha = -1;
    while (escolha != 0) {
        mostrarMenu();
        escolha = lerNumero("\nDigite a opcao desejada: ");

        switch (escolha) {
            case 1:  definirParametrosConexao(); break;
            case 2:  conectar();                 break;
            case 3:  desconectar();              break;
            case 4:  imprimirTextoSimples();     break;
            case 5:  imprimirCodigoQR();         break;
            case 6:  imprimirBarras();           break;
            case 7:  imprimirCupomSAT();         break;
            case 8:  imprimirCancelamentoSAT();  break;
            case 9:  acionarGavetaElgin();       break;
            case 10: acionarGaveta();            break;
            case 11: tocarBeep();                break;
            case 0:
                /* ao sair, fecha a conexao se ela estiver aberta */
                if (conectada) {
                    desconectar();
                }
                printf("Encerrando o programa...\n");
                break;
            default:
                printf("Opcao invalida.\n");
                break;
        }
    }

    FreeLibrary(hDll);
    return 0;
}
