# 🖨️ Controle da Impressora Elgin i9 em C

**(Coloque aqui o nome do seu projeto)**

## 📘 Sobre o Projeto

Este projeto foi desenvolvido por alunos com o objetivo de criar uma aplicação em **linguagem C** capaz de controlar uma impressora térmica **Elgin i8 / i9**, usando a biblioteca de comandos da própria Elgin (`E1_Impressora01.dll`).

O programa roda no terminal (console), pede um login e mostra um menu com as operações da impressora: abrir e fechar a conexão, imprimir texto, QR Code, código de barras, cupons SAT, abrir a gaveta e emitir sinal sonoro.

O trabalho faz parte da disciplina de **(nome da disciplina)**.

## 🧰 Tecnologias Utilizadas

- Linguagem C
- Compilador para Windows, de 32 ou 64 bits (por exemplo, MinGW do Code::Blocks ou MSYS2 mingw64)
- `E1_Impressora01.dll` (biblioteca da Elgin), carregada em tempo de execução com `LoadLibraryA` e `GetProcAddress` da API do Windows
- Impressora Elgin i8 ou i9

> ⚠️ O programa usa `windows.h`, então só compila e executa no **Windows**.

## 📁 Estrutura do Projeto

```
/
├── main_novo.c            → código-fonte principal
├── E1_Impressora01.dll    → DLL da Elgin (precisa estar na raiz)
├── XMLSAT.xml             → XML de venda SAT, usado na opção 7
└── CANC_SAT.xml           → XML de cancelamento SAT, usado na opção 8
```

A DLL e os dois arquivos XML precisam ficar na **mesma pasta de onde o programa é executado**.

### ⚠️ Arquitetura da DLL

A DLL precisa ter a mesma arquitetura do compilador:

| Compilador | DLL a usar |
|---|---|
| 32 bits (ex.: MinGW do Code::Blocks) | pasta `x86` |
| 64 bits (ex.: MSYS2 mingw64) | pasta `x64` |

## ⚙️ Instalação e Execução

### 🔽 1. Baixar o Projeto do GitHub

1. Acesse o repositório.
2. Clique no botão **Code**.
3. Selecione **Download ZIP**.
4. Aguarde o download.

### 📦 2. Extrair o Arquivo ZIP

1. Vá até o local onde o arquivo foi salvo.
2. Clique com o botão direito → **Extrair aqui / Extract All**.
3. Será criada uma nova pasta com os arquivos.

### 📋 3. Preparar a pasta

Copie para a raiz do projeto (junto do `main_novo.c`):

1. A `E1_Impressora01.dll` da arquitetura correta (x86 ou x64).
2. Os arquivos `XMLSAT.xml` e `CANC_SAT.xml`.

### 🔨 4. Compilar

**Pelo terminal** (com o MinGW instalado), dentro da pasta do projeto:

```
gcc main_novo.c -o impressora.exe
```

**Pelo Code::Blocks:** crie um projeto de console em C, adicione o `main_novo.c` e clique em **Build**.

### ▶️ 5. Executar

Rode o programa **a partir da pasta do projeto**, para que ele encontre a DLL e os XMLs:

```
impressora.exe
```

## 🔐 Login

Ao abrir, o programa pede usuário e senha. São permitidas **3 tentativas**; depois disso o acesso é negado e o programa encerra.

- Usuário: `admin`
- Senha: `1234`

## 📋 Menu de Opções

| Opção | Função |
|---|---|
| 1 | Configurar Conexão (tipo, modelo, porta e parâmetro adicional) |
| 2 | Abrir Conexão |
| 3 | Fechar Conexão |
| 4 | Imprimir Texto (com quantidade de cópias) |
| 5 | Imprimir QR Code |
| 6 | Imprimir Código de Barras (CODE 128) |
| 7 | Imprimir XML SAT (lê o `XMLSAT.xml`) |
| 8 | Imprimir XML de Cancelamento SAT (lê o `CANC_SAT.xml`) |
| 9 | Abrir Gaveta (Elgin) |
| 10 | Abrir Gaveta |
| 11 | Sinal Sonoro (1 a 63 apitos) |
| 0 | Sair (fecha a conexão se estiver aberta) |

### Como usar

1. Faça o login.
2. Use a opção **1** para configurar a conexão. Os valores padrão são tipo `1` (USB), modelo `i9`, porta `USB` e parâmetro `0`.
3. Use a opção **2** para abrir a conexão. As opções de impressão, gaveta e sinal sonoro só funcionam com a conexão aberta.
4. Escolha a operação desejada.
5. Use a opção **0** para sair.

Após imprimir texto, QR Code, código de barras ou XML SAT, o programa corta o papel automaticamente.

## 🧪 Testes

- Login correto e login errado (3 tentativas)
- Uso de uma opção de impressão com a conexão fechada
- Configurar e abrir a conexão com a impressora
- Impressão de texto simples, com uma ou várias cópias
- Impressão de QR Code e de código de barras
- Impressão do XML SAT e do XML de cancelamento
- Abertura da gaveta
- Sinal sonoro com quantidade válida (1 a 63) e inválida
- Digitar letras onde se espera um número
- Executar sem a DLL ou sem os XMLs na pasta

## 🛠️ Possíveis Problemas

| Mensagem | O que conferir |
|---|---|
| `Nao foi possivel carregar E1_Impressora01.dll` | A DLL está na raiz do projeto? Ela tem a mesma arquitetura do compilador? |
| `Comando '...' nao encontrado na DLL` | A versão da DLL é compatível com o programa? |
| `Erro ao abrir conexao. Codigo de erro: ...` | A impressora está ligada e conectada? O tipo, o modelo e a porta estão corretos? |
| `Nao foi possivel ler o XMLSAT.xml` (ou `CANC_SAT.xml`) | O arquivo está na pasta de onde o programa é executado? |

## 📚 Documentação Adicional

- Manual da Impressora Elgin i9
- Documentação da biblioteca `E1_Impressora01` da Elgin

## 👥 Autor

- Gustavo Rodrigues 259787
