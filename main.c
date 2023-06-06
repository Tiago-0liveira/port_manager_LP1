#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define CONTENTOR_MIN_WEIGHT 500
#define MAX_BUFFER_LENGTH 255
#define ERROR_FNF "ERROR: could not open file\n"
#define ERROR_FFE "ERROR: file format is not recognized\n"
#define MENU "+---- MENU\n| move [-g grua] [-d ponto] [-p pilha] [-D ponto] [-P pilha] [-n numero_de_contentores]\n| show [-d ponto] [-e embarc]\n| where [embarc]\n| navigate [-e embarc] [-d ponto]\n| load [-e embarc] [-p pilha] [-c contentor:peso]\n| weight [embarc]\n| save [filename]\n| help \n| quit \n+----\n"
#define INPUT_MARKER "> "
#define MATRICULA_SIZE 5
#define COMMAND_MAX_ARGS_NUM 6
#define COMMAND_NAME_MAX_LEN 10
#define COMAND_MAX_ARG_LEN 50
#define COMMAND_INPUT_MAX_LEN 100
#define MAX_EMBARCACOES_NUM 10
#define CODIGO_CONTENTOR_SIZE 4
#define MAX_EMBARCACAO_PILHAS_CONTENTORES 6
#define FLAG_VALUE_MAX_LEN 10
#define COMMAND_INVALID "ERROR: invalid command\n"
#define COMMAND_SUCCESS "SUCCESS: operation concluded\n"
#define GRUA_A_MAX_CONTENTORES 1
#define GRUA_B_MAX_CONTENTORES 4
#define FLAG_VALUE_TMP_POINTER_SIZE (2 + MAX_EMBARCACOES_NUM + MATRICULA_SIZE)

//#define ESTALEIRO_BONITO
#ifdef ESTALEIRO_BONITO
    #define ESTALEIRO_BONITO_HASTE "|"
    #define ESTALEIRO_BONITO_BASE "-----"
#endif

const char *VALID_COMMANDS[] = {"move", "show", "where", "navigate", "load", "weight", "save", "help", "quit"};
const char *VALID_COMMANDS_FLAGS[] = { "gdpDPn", "de", "", "ed", "epc"};

typedef char Matricula[MATRICULA_SIZE];
typedef char Codigo_contentor[CODIGO_CONTENTOR_SIZE];

typedef struct _contentor
{
	int peso;
	Codigo_contentor id;
} Contentor;

typedef struct stack_node {
    Contentor *contentor;
    struct stack_node* next;
} stack_node;

typedef struct _stack_contentor
{
	stack_node* topo;
} stack_contentor;

typedef struct _embarcacao
{
	Matricula matricula;
	stack_contentor *pilhas_contentores[MAX_EMBARCACAO_PILHAS_CONTENTORES];
} Embarcacao;

typedef struct _estaleiro
{
	Embarcacao *embarcacoes[MAX_EMBARCACOES_NUM];
} Estaleiro;

typedef struct _flag
{
	char name;
	void *value;
    int is_num;
} Flag;

typedef struct _command
{
	char *name;
	int num_flags;
	Flag flags[COMMAND_MAX_ARGS_NUM];
	int error;
    char *arg;
} Command;

int startsWith(const char *str, const char *pre)
{
    size_t lenpre, lenstr;
    lenpre = strlen(pre), lenstr = strlen(str);
    return lenstr < lenpre ? 0 : strncmp(pre, str, lenpre) == 0;
}

int valid_contentor_code(char *c_code)
{
    if (strlen(c_code) != (CODIGO_CONTENTOR_SIZE - 1))
        return 0;
    for (int i = 0; i < 2; i++)
        if (c_code[i] < 'A' || c_code[i] > 'Z')
            return 0;
    if (c_code[2] < '0' || c_code[2] > '9')
        return 0;
    return 1;
}

int valid_matricula(char *matricula)
{
    int i;
    if (strlen(matricula) != (MATRICULA_SIZE-1))
        return 0;
    i = 0;
    while (matricula[i] != '\0') {
        if (matricula[i] < 'A' || matricula[i] > 'Z')
            return 0;
        i++;
    }
    return 1;
}

Flag *getFlag(Command *command, char flag_name)
{
    int i;
    for (i = 0; i < command->num_flags; i++)
    {
        if (command->flags[i].name == flag_name)
            return &command->flags[i];
    }
    return NULL;
}

int set_flag_value(Command *cmd, char flag_name, void *dest)
{
    int i;
    for (i = 0; i < cmd->num_flags; i++)
    {
        Flag *flag;
        flag = &cmd->flags[i];
        if (flag->name == flag_name)
        {
            if (flag->is_num)
            {
                *((int*)dest) = *((int*)flag->value);
                return 1;
            }
            else
            {
                strcpy((char*)dest, (char*)flag->value);
                return 2;
            }
        }
    }
    return 0;
}

int char_in_str(const char *str, char c)
{
    unsigned long long int i;
    for (i = 0; i < strlen(str); i++)
    {
        if (str[i] == c)
            return 1;
    }
    return 0;
}

int isValidCommandName(char *command_name)
{
	unsigned long long int i;
	for (i = 0; i < sizeof(VALID_COMMANDS) / sizeof(char*); i++)
	{
		if (strcmp(command_name, VALID_COMMANDS[i]) == 0)
			return 1;
	}
	return 0;
}

Contentor *parse_contentor(char *contentor_str)
{
    char *tmp3;
    Contentor *c;

    tmp3 = strchr(contentor_str, ':');
    if (tmp3 == NULL) return NULL;
    if ((tmp3 - (char *) contentor_str) != (CODIGO_CONTENTOR_SIZE - 1)) return NULL;
    c = malloc(sizeof(Contentor));
    if (c == NULL) return NULL;
    if (sscanf(contentor_str, "%3s:%d", c->id, &c->peso) != 2 || c->peso < 0 || c->peso < CONTENTOR_MIN_WEIGHT || !valid_contentor_code(c->id)) return NULL;
    return c;
}

int isValidArg(Command *command, char arg_name, int is_num)
{
    int i, i_val;
    void *tmp;
    char *s_val;
    tmp = malloc(sizeof(char) * FLAG_VALUE_MAX_LEN);
    if (tmp == NULL) return 0;
    i = set_flag_value(command, arg_name, tmp);
    if (i == 0) return 0;
    if (i == 1) {
        if (!is_num){
            free(tmp);
            return 0;
        }
        i_val = *((int*)tmp);
    } else if (i == 2) {
        if (is_num) {
            free(tmp);
            return 0;
        }
        s_val = (char*)tmp;
    }
    switch (arg_name) {
        case 'g':
            if (strlen(s_val) > 1 || (*s_val != 'A' && *s_val != 'B')) return 0;
            break;
        case 'd':
        case 'D':
            if (i_val < 0 || i_val >= MAX_EMBARCACOES_NUM) return 0;
            break;
        case 'p':
        case 'P':
            if (i_val < 0 || i_val >= MAX_EMBARCACAO_PILHAS_CONTENTORES) return 0;
            break;
        case 'n':
            if (i_val < 0) return 0;
            break;
        case 'e':
            if (!valid_matricula(s_val)) return 0;
            break;
        case 'c': {
            if (parse_contentor(s_val) == NULL) return 0;
            break;
        }
        default:
            return 0;
    }
    free(tmp);
    return 1;
}

int isValidCommand(Command *command)
{
    if (strcmp(command->name, "move") == 0) {
        int has1, has2;
        if (command->num_flags < 4 || command->num_flags > 6 || strlen(command->arg) > 0) return 0;

        if (!isValidArg(command, 'n', 1)) return 0;
        if (!isValidArg(command, 'g', 0)) return 0;
        has1 = isValidArg(command, 'd', 1);
        has2 = isValidArg(command, 'D', 1);
        if ((!has2 && !has1) || !(has2 && getFlag(command, 'D') != NULL) || !(has1 && getFlag(command, 'd') != NULL)) return 0;
        has1 = isValidArg(command, 'p', 1);
        has2 = isValidArg(command, 'P', 1);
        if ((!has2 && !has1) || !(has2 && getFlag(command, 'P') != NULL) || !(has1 && getFlag(command, 'p') != NULL)) return 0;
    }
    else if (strcmp(command->name, "show") == 0) {
        if (command->num_flags > 1 || strlen(command->arg) > 0) return 0;
        if (command->num_flags > 0 &&
            !isValidArg(command, 'e', 0) &&
            !isValidArg(command, 'd', 1))
                return 0;
    }
    else if (strcmp(command->name, "navigate") == 0) {
        if (command->num_flags != 2) return 0;
        if (!isValidArg(command, 'd', 1) || !isValidArg(command, 'e', 0)) return 0;
    }
    else if (strcmp(command->name, "load") == 0) {
        if (command->num_flags != 3) return 0;
        if (!isValidArg(command, 'p', 1) ||
            !isValidArg(command, 'e', 0) ||
            !isValidArg(command, 'c', 0))
                return 0;
    }
    else if (strcmp(command->name, "where")  == 0 || strcmp(command->name, "weight") == 0)
    {
        if (command->num_flags != 0 || !valid_matricula(command->arg)) return 0;
    }
    else if (strcmp(command->name, "save") == 0)
    {
        if (command->num_flags != 0 || (command->arg != NULL && strlen(command->arg) < 1)) return 0;
    }
    return 1;
}

int getCommandIndex(char *command_name)
{
    unsigned long long int i;
    for (i = 0; i < sizeof(VALID_COMMANDS) / sizeof(char*); i++)
    {
        if (strcmp(command_name, VALID_COMMANDS[i]) == 0)
            return (int) i;
    }
    return -1;
}

int command_can_have_flags(char *command_name)
{
    int i;
    i = getCommandIndex(command_name);
    if (i == -1 || i == 2 || i > 4)
        return 0;
    return 1;
}

int is_valid_flag(char *command_name, char flag_name)
{
    int i;
    i = getCommandIndex(command_name);
    if (i == -1)
        return 0;
    return char_in_str(VALID_COMMANDS_FLAGS[i], flag_name);
}

int str_is_num(char *str) {
    int i;
    i = 0;
    if (str[0] == '-')
        i++;
    for (; i < (int) strlen(str); i++) {
        if (!isdigit(str[i]))
            return 0;
    }
    return 1;
}

Flag *cria_flag(void)
{
    Flag *flag;
    flag = (Flag*)malloc(sizeof(Flag));
    if (flag == NULL) return NULL;
    flag->is_num = 0;
    return flag;
}

Flag *cria_flag_com_nome(char nome)
{
    Flag *flag;
    flag = cria_flag();
    if (flag == NULL) return NULL;
    flag->name = nome;
    return flag;
}

Command *cria_command(void)
{
    Command *command;
    command = (Command*)malloc(sizeof(Command));
    if (command == NULL) return NULL;
    command->name = (char*)malloc(sizeof(char) * COMMAND_NAME_MAX_LEN);
    command->arg = (char*)malloc(sizeof(char) * COMAND_MAX_ARG_LEN);
    command->num_flags = 0;
    command->error = 0;
    strcpy(command->arg, "");
    return command;
}

stack_contentor *criar_stack_contentor(void)
{
	stack_contentor *stack = (stack_contentor*)malloc(sizeof(stack_contentor));
	stack->topo = NULL;
	return stack;
}

Embarcacao *cria_embarcacao(void)
{
    Embarcacao *embarcacao = (Embarcacao*)malloc(sizeof(Embarcacao));
    embarcacao->matricula[0] = 0;

    for (int i = 0; i < MAX_EMBARCACAO_PILHAS_CONTENTORES; i++)
    {
        embarcacao->pilhas_contentores[i] = criar_stack_contentor();
    }
    return embarcacao;
}

int stack_contentor_vazia(stack_contentor *stack)
{
    if (stack == NULL) return 1;

	return (stack->topo == NULL);
}

void empilhar(stack_contentor *stack, Contentor *contentor)
{
	stack_node *novoNo = (stack_node*)malloc(sizeof(stack_node));
    if (novoNo == NULL) return;
	novoNo->contentor = contentor;
	novoNo->next = stack->topo;
	stack->topo = novoNo;
}

Contentor *desempilhar(stack_contentor *stack)
{
	stack_node *temp;
	Contentor *contentorDesempilhado;

	if (stack_contentor_vazia(stack))
		return NULL;

	temp = stack->topo;
	contentorDesempilhado = temp->contentor;
	stack->topo = temp->next;
	free(temp);
	return contentorDesempilhado;
}

int tamanho_stack_contentor(stack_contentor *stack)
{
	int tamanho = 0;
	stack_node *tmp;

    if (stack_contentor_vazia(stack) == 1) return 0;
	tmp = stack->topo;
    if (tmp != NULL) tamanho++;
	while (tmp != NULL && tmp->next != NULL)
	{
		tmp = tmp->next;
		tamanho++;
	}

	return tamanho;
}

int num_digits(int num)
{
    int digits = 0;
    if (num < 10) return 1;
    while (num != 0)
    {
        num /= 10;
        digits++;
    }
    return digits;
}

Contentor *get_contentor_at_index(stack_contentor *stack, int index)
{
    stack_node *tmp;
    if (tamanho_stack_contentor(stack) <= index)
        return NULL;
    index = tamanho_stack_contentor(stack) - index - 1;
    tmp = stack->topo;
    while (tmp != NULL)
    {
        if (index == 0)
            return tmp->contentor;
        tmp = tmp->next;
        index--;
    }
    return NULL;
}

char *str_contentor(Contentor *contentor)
{
	char *str;
	str = (char*)malloc(sizeof(char) * (3 + num_digits(contentor->peso) + 1));
    if (str == NULL) return NULL;
	sprintf(str, "%s:%d", contentor->id, contentor->peso);
	return str;
}

int get_peso_stack(stack_contentor *stack)
{
    int peso = 0;
    stack_node *tmp;

    tmp = stack->topo;
    while (tmp != NULL)
    {
        peso += tmp->contentor->peso;
        tmp = tmp->next;
    }
    return peso;
}

Command* command_from_str(char* raw_command)
{
    Command* command;
    char* token;
    Flag *flag;

    command = cria_command();
    if (command == NULL) return NULL;
    sscanf(raw_command, "%s", command->name);
    if (!isValidCommandName(command->name))
    {
        command->error = 1;
        return command;
    }

    token = strtok(raw_command, " ");
    token = strtok(NULL, " ");
    if (!command_can_have_flags(command->name))
    {
        if (token != NULL){
            strcpy(command->arg, token);
        }
        return command;
    }
    while (token != NULL && command->num_flags <= COMMAND_MAX_ARGS_NUM)
    {
        if (startsWith(token, "-") && is_valid_flag(command->name, token[1]))
        {
            if (getFlag(command, token[1]))/* duplicate arg */
            {
                command->error = 1;
                return command;
            }
            flag = cria_flag_com_nome(token[1]);
            token = strtok(NULL, " ");
            if (token == NULL) {
                command->error = 1;
                return command;
            } else if (str_is_num(token)) {
                long l = strtol(token, NULL, 10);
                int* valuePtr;
                if (l > INT_MAX || l < INT_MIN || l < 0) {
                    command->error = 1;
                    return command;
                }
                valuePtr = (int*)malloc(sizeof(int));
                if (valuePtr == NULL) {
                    command->error = 1;
                    return command;
                }
                *valuePtr = (int)l;
                flag->is_num = 1;
                flag->value = (void*)valuePtr;
            }
            else
                flag->value = token;

            command->num_flags++;
            command->flags[command->num_flags-1] = *flag;
            token = strtok(NULL, " ");
            continue;
        }
        strcpy(command->arg, token);
        token = strtok(NULL, " ");
    }

    return command;
}

int embarcacao_is_null(Embarcacao *emb)
{
    return (int) strlen(emb->matricula) == 0;
}

int embarcacao_vazia(Embarcacao *emb)
{
    if (embarcacao_is_null(emb))
        return 1;
    for (int i = 0; i < MAX_EMBARCACAO_PILHAS_CONTENTORES; i++)
    {
        if (stack_contentor_vazia(emb->pilhas_contentores[i]) == 0)
            return 0;
    }
    return 1;
}

Contentor **getContentoresFromPilha(Embarcacao *emb, int pilha_index, int num_contentores)
{
    Contentor **contentores;
    int i;
    contentores = (Contentor**)malloc(sizeof(Contentor**) * num_contentores);
    if (contentores == NULL) return NULL;
    for (i = num_contentores-1; i >= 0; i--)
    {
        contentores[i] = desempilhar(emb->pilhas_contentores[pilha_index]);
    }
    return contentores;
}

void move_contentores(Estaleiro *pEstaleiro, int grua_max_move, int emb_index_p, int emb_index_des, int pile_index_p, int pile_index_des, int num_contentores) {
    Contentor **contentores;
    int i, iterations, j;
    iterations = num_contentores / grua_max_move;
    if (num_contentores % grua_max_move != 0)
        iterations++;
    for (i = 0; i < iterations; i++)
    {
        if (num_contentores < grua_max_move)
            grua_max_move = num_contentores;
        contentores = getContentoresFromPilha(pEstaleiro->embarcacoes[emb_index_p], pile_index_p, grua_max_move);
        if (contentores == NULL) return;
        for (j = 0; j < grua_max_move && contentores[j] != NULL; j++) {
            empilhar(pEstaleiro->embarcacoes[emb_index_des]->pilhas_contentores[pile_index_des], contentores[j]);
            free(contentores[j]);
        }
        free(contentores);
        num_contentores -= grua_max_move;

    }
}

int get_embarcacao_str_size(Embarcacao *embarcacao)
{
    int size, i;
    stack_node *tmp;

    size = MATRICULA_SIZE + 3;//d%d %s\n
    for (i = 0; i < MAX_EMBARCACAO_PILHAS_CONTENTORES; i++)
    {
        if (tamanho_stack_contentor(embarcacao->pilhas_contentores[i]) == 0)
            continue;
        size += 4 + num_digits(i) + num_digits(tamanho_stack_contentor(embarcacao->pilhas_contentores[i])); //p%d %d
        tmp = embarcacao->pilhas_contentores[i]->topo;
        while (tmp != NULL)
        {
            size += 4 + num_digits(tmp->contentor->peso); //%3s:%d...
            tmp = tmp->next;
            if (tmp != NULL)
                size++;
        }
        size++;
    }
    return size;
}

char *str_embarcacao(Embarcacao *emb, int emb_idx)
{
    char *str, *target;
    int i, z;
    str = (char*)malloc(sizeof(char) * (get_embarcacao_str_size(emb) + 1));
    if (str == NULL) return NULL;
    target = str;
    if (embarcacao_is_null(emb)) {
        target += sprintf(target, "d%d embarcacao vazia\n", emb_idx);
        return str;
    }
    else target += sprintf(target, "d%d %s\n", emb_idx, emb->matricula);
    for (i = 0; i < MAX_EMBARCACAO_PILHAS_CONTENTORES; i++)
    {
        if (tamanho_stack_contentor(emb->pilhas_contentores[i]) == 0)
            continue;
        target += sprintf(target, "\tp%d %d", i, tamanho_stack_contentor(emb->pilhas_contentores[i]));
        for (z = 0; z < tamanho_stack_contentor(emb->pilhas_contentores[i]); z++)
        {
            Contentor *c;
            c = get_contentor_at_index(emb->pilhas_contentores[i], z);
            target += sprintf(target, " %s:%d", c->id, c->peso);
        }
        target += sprintf(target, "\n");
    }

    return str;
}

#ifdef ESTALEIRO_BONITO
int get_tamanho_stack_maior(Embarcacao *emb) {
    int tamanho = 0;
    int tmp;

    for (int i = 0; i < MAX_EMBARCACAO_PILHAS_CONTENTORES; i++) {
        if ((tmp = tamanho_stack_contentor(emb->pilhas_contentores[i])) > tamanho)
            tamanho = tmp;
    }
    return tamanho;
}

char *str_embarcacao_bonita(Embarcacao *emb, int emb_idx)
{
    char *res, *s;
    int j, z;
    int biggest_stack_size_index;
    res = (char*)malloc(sizeof(char) * 1000);

    if (embarcacao_is_null(emb))
    {
        sprintf(res, "Embarcacao i=%d nao existe\n", emb_idx);
        return res;
    } else if (embarcacao_vazia(emb))
    {
        strcpy(res, "Embarcacao ");
        strcat(res, emb->matricula);
        strcat(res, " vazia\n");
        return res;
    }
    biggest_stack_size_index = get_tamanho_stack_maior(emb) - 1;
    strcpy(res, "\n");
    for (j = biggest_stack_size_index; j >= 0; j--)
    {
        strcat(res, "  ");
        for (z = 0; z < MAX_EMBARCACAO_PILHAS_CONTENTORES; z++)
        {
            Contentor *c;
            if ((c = get_contentor_at_index(emb->pilhas_contentores[z], j)) != NULL)
                strcat(res, c->id);
            else
                strcat(res, "   ");
            strcat(res, "   ");
        }
        strcat(res, "\n");
    }
    for (z = 0; z < MAX_EMBARCACAO_PILHAS_CONTENTORES; z++)
    {
        strcat(res, ESTALEIRO_BONITO_HASTE);
        strcat(res, ESTALEIRO_BONITO_BASE);
    }
    strcat(res, ESTALEIRO_BONITO_HASTE);
    strcat(res, "\n");
    strcat(res, "   ");
    for (z = 0; z < MAX_EMBARCACAO_PILHAS_CONTENTORES; z++)
    {
        char *ss;
        ss = malloc(7);
        sprintf(ss, "%d     ", z);
        strcat(res, ss);
    }
    strcat(res, "\n");
    s = malloc(FLAG_VALUE_TMP_POINTER_SIZE);
    sprintf(s, "d%d %s\n", emb_idx, emb->matricula);
    strcat(res, s);
    free(s);
    return res;
}

void print_estaleiro_bonito(Estaleiro *pEstaleiro)
{
    int i;

    printf("\n");
    for (i = 0; i < MAX_EMBARCACOES_NUM; i++)
    {
        printf("%s", str_embarcacao_bonita(pEstaleiro->embarcacoes[i], i));
    }
    printf("\n");
}
#endif

Embarcacao *get_embarcacao_by_name(Estaleiro *pEstaleiro, char *matricula)
{
    int i;
    for (i = 0; i < MAX_EMBARCACOES_NUM; i++)
    {
        if (embarcacao_is_null(pEstaleiro->embarcacoes[i]))
            continue;
        if (strcmp(pEstaleiro->embarcacoes[i]->matricula, matricula) == 0)
            return pEstaleiro->embarcacoes[i];
    }
    return NULL;
}

int get_index_by_embarcacao(Estaleiro *pEstaleiro, Embarcacao *emb)
{
    int i;
    for (i = 0; i < MAX_EMBARCACOES_NUM; i++)
    {
        if (embarcacao_is_null(pEstaleiro->embarcacoes[i]))
            continue;
        if (pEstaleiro->embarcacoes[i] == emb)
            return i;
    }
    return -1;
}

char **str_estaleiro(Estaleiro *pEstaleiro)
{
    char **str;
    int i;
    str = malloc(sizeof(char*) * MAX_EMBARCACOES_NUM);
    for (i = 0; i < MAX_EMBARCACOES_NUM; i++)
    {
        if (embarcacao_is_null(pEstaleiro->embarcacoes[i])) {
            str[i] = (char *) malloc(sizeof(char) * 1);
            strcpy(str[i], "");
            continue;
        }
        if (embarcacao_vazia(pEstaleiro->embarcacoes[i])) {
            str[i] = (char *) malloc(sizeof(char) * (MATRICULA_SIZE - 1 + 3 + 1));
            sprintf(str[i], "d%d %s\n", i, pEstaleiro->embarcacoes[i]->matricula);
            continue;
        }
        str[i] = (char*)malloc(sizeof(char) * (get_embarcacao_str_size(pEstaleiro->embarcacoes[i]) + 1));
        strcpy(str[i], str_embarcacao(pEstaleiro->embarcacoes[i], i));
    }
    return str;
}

int get_peso_embarcacao(Embarcacao *emb)
{
    int i, res;
    for (i = 0,res = 0; i < MAX_EMBARCACAO_PILHAS_CONTENTORES; i++)
    {
        if (emb->pilhas_contentores[i] == NULL)
            continue;
        res += get_peso_stack(emb->pilhas_contentores[i]);
    }
    return res;
}

int contentor_id_exists_in_stack(stack_contentor *stack, Codigo_contentor c_id)
{
    int i;
    for (i = 0; i < tamanho_stack_contentor(stack); i++)
    {
        if (strcmp(get_contentor_at_index(stack, i)->id, c_id) == 0)
            return 1;
    }
    return 0;
}

int contentor_id_exists_in_estaleiro(Estaleiro *pEstaleiro, Codigo_contentor c_id)
{
    int i, j;
    for (i = 0; i < MAX_EMBARCACOES_NUM; i++)
    {
        if (embarcacao_is_null(pEstaleiro->embarcacoes[i]))
            continue;
        for (j = 0; j < MAX_EMBARCACAO_PILHAS_CONTENTORES; j++)
        {
            if (pEstaleiro->embarcacoes[i]->pilhas_contentores[j] == NULL)
                continue;
            if (contentor_id_exists_in_stack(pEstaleiro->embarcacoes[i]->pilhas_contentores[j], c_id))
                return 1;
        }
    }
    return 0;
}

int executeCommand(Command *command, Estaleiro *estaleiro)
{
    if (strcmp(command->name, "move") == 0) {
        int grua_carga_max, ponto_embarque_partida,ponto_embarque_destino ,pile_index_partida,pile_index_destino ,num_contentores, t_stack;

        if (set_flag_value(command, 'n', &num_contentores) != 1) return 0;
        if (set_flag_value(command, 'd', &ponto_embarque_partida) == 0) ponto_embarque_partida = -1;
        if (set_flag_value(command, 'D', &ponto_embarque_destino) == 0) ponto_embarque_destino = -1;
        if (set_flag_value(command, 'p', &pile_index_partida) == 0) pile_index_partida = -1;
        if (set_flag_value(command, 'P', &pile_index_destino) == 0) pile_index_destino = -1;

        if (ponto_embarque_partida == -1) ponto_embarque_partida = ponto_embarque_destino;
        if (ponto_embarque_destino == -1) ponto_embarque_destino = ponto_embarque_partida;
        if (pile_index_partida == -1) pile_index_partida = pile_index_destino;
        if (pile_index_destino == -1) pile_index_destino = pile_index_partida;

        /*printf("ponto_embarque_partida: %d, ponto_embarque_destino: %d, pile_index_partida: %d, pile_index_destino: %d, num_contentores: %d\n",
               ponto_embarque_partida, ponto_embarque_destino, pile_index_partida, pile_index_destino, num_contentores);*/
        if (embarcacao_is_null(estaleiro->embarcacoes[ponto_embarque_partida]) || embarcacao_is_null(estaleiro->embarcacoes[ponto_embarque_destino]))
            return 0;
        t_stack = tamanho_stack_contentor(estaleiro->embarcacoes[ponto_embarque_partida]->pilhas_contentores[pile_index_partida]);

        if (t_stack < num_contentores)
            num_contentores = t_stack;
        switch(((char *) getFlag(command, 'g')->value)[0])
        {
            case 'A':
                grua_carga_max = GRUA_A_MAX_CONTENTORES;
                break;
            case 'B':
                grua_carga_max = GRUA_B_MAX_CONTENTORES;
                break;
            default:
                return 0;
        }
        move_contentores(estaleiro, grua_carga_max, ponto_embarque_partida, ponto_embarque_destino,
                         pile_index_partida, pile_index_destino, num_contentores);

        printf("%s", COMMAND_SUCCESS);
        return 1;
    }
    else if (strcmp(command->name, "show") == 0) {
        if (command->num_flags == 0)
        {
#ifdef ESTALEIRO_BONITO
            print_estaleiro_bonito(estaleiro);
#else
            char **strs;
            int i;
            strs = str_estaleiro(estaleiro);
            for (i = 0; i < MAX_EMBARCACOES_NUM; i++) {
                if (strs[i] != NULL && strlen(strs[i]) > 0)
                    printf("%s", strs[i]);
            }

#endif
        } else {
            void *tmp;
            Embarcacao *emb;
            int emb_idx;
            tmp = malloc(FLAG_VALUE_TMP_POINTER_SIZE);
            if (tmp == NULL) return 0;
            if (set_flag_value(command, 'e', tmp))
            {
                emb = get_embarcacao_by_name(estaleiro, tmp);
                emb_idx = get_index_by_embarcacao(estaleiro, emb);
                if (embarcacao_is_null(emb)) return 0;
#ifdef ESTALEIRO_BONITO
                printf("%s", str_embarcacao_bonita(emb, emb_idx));
#else
                printf("%s", str_embarcacao(emb, emb_idx));
#endif
            }
            else if (set_flag_value(command, 'd', tmp))
            {
                emb = estaleiro->embarcacoes[*(int *) tmp];

                if (embarcacao_is_null(emb))
                    return 0;
                    /*printf("%s", str_embarcacao(estaleiro->embarcacoes[*(int *) tmp], *(int *) tmp));*/
                else
                {
                    emb_idx = get_index_by_embarcacao(estaleiro, emb);
#ifdef ESTALEIRO_BONITO
                    printf("%s", str_embarcacao_bonita(estaleiro->embarcacoes[*(int *) tmp], emb_idx));
#else
                    printf("%s", str_embarcacao(estaleiro->embarcacoes[*(int *) tmp], emb_idx));
#endif
                }
            }
        }
    }
    else if (strcmp(command->name, "where") == 0 || strcmp(command->name, "weight") == 0) {
        Embarcacao *emb;
        emb = get_embarcacao_by_name(estaleiro, command->arg);
        if (emb == NULL)
            return 0;
        if (strcmp(command->name, "where") == 0)
            printf("%d %s\n", get_index_by_embarcacao(estaleiro, emb), emb->matricula);
        else
            printf("%s %d\n", emb->matricula, get_peso_embarcacao(emb));
    }
    else if (strcmp(command->name, "navigate") == 0) {
        Embarcacao *emb;
        char *matricula;
        int *emb_idx;
        matricula = getFlag(command, 'e')->value;
        emb = get_embarcacao_by_name(estaleiro, matricula);
        if (emb == NULL) {
            emb = cria_embarcacao();
            strcpy(emb->matricula, matricula);
        } else {
            estaleiro->embarcacoes[get_index_by_embarcacao(estaleiro, emb)] = cria_embarcacao();
        }
        emb_idx = malloc(sizeof(int));
        if (set_flag_value(command, 'd', emb_idx) != 1 || !embarcacao_is_null(estaleiro->embarcacoes[*emb_idx]))
            return 0;
        estaleiro->embarcacoes[*emb_idx] = emb;
        printf("%s", COMMAND_SUCCESS);
        return 1;
    }
    else if (strcmp(command->name, "load") == 0) {
        char *matricula, *tmp;
        int *pile_idx, emb_idx;
        Contentor *c;

        matricula = malloc(MATRICULA_SIZE);
        set_flag_value(command, 'e', matricula);
        if ((emb_idx = get_index_by_embarcacao(estaleiro, get_embarcacao_by_name(estaleiro, matricula))) == -1) /* nao existe embarcacao com essa matricula */
            return 0;
        pile_idx = malloc(sizeof(int));
        set_flag_value(command, 'p', pile_idx);
        tmp = malloc(FLAG_VALUE_TMP_POINTER_SIZE);
        set_flag_value(command, 'c', tmp);
        c = parse_contentor(tmp);
        if (tmp == NULL || pile_idx == NULL || c == NULL || contentor_id_exists_in_estaleiro(estaleiro, c->id)) return 0;
        empilhar(estaleiro->embarcacoes[emb_idx]->pilhas_contentores[*pile_idx], c);
        printf("%s", COMMAND_SUCCESS);
        return 1;
    }
    else if (strcmp(command->name, "save") == 0) {
        char **stream;
        FILE *fp;
        int i;

        fp = fopen(command->arg, "w+");
        if (fp == NULL)
            return 0;
        stream = str_estaleiro(estaleiro);
        for (i = 0; i < MAX_EMBARCACOES_NUM; i++)
        {
            if (strcmp(stream[i], "") != 0) {
                fprintf(fp, "%s", stream[i]);
            }
        }
        fclose(fp);
        printf("%s", COMMAND_SUCCESS);
    }
    else if (strcmp(command->name, "help") == 0 /*|| strcmp(command->name, "quit") == 0*/) {
        return 2;
    }
    return 1;
}

void	read_config_file(char *filename, Estaleiro *estaleiro)
{
	FILE *fp;
	char line[MAX_BUFFER_LENGTH];
	int embarcacao_index;
	int i;

	fp = fopen(filename, "r");
	if (fp == NULL)
	{
		printf(ERROR_FNF);
		exit(1);
	}
	
	while(fgets(line, MAX_BUFFER_LENGTH, fp)) {
		if (startsWith(line, "d")) {
			sscanf(line, "d%d ", &embarcacao_index);
            if (embarcacao_index >= MAX_EMBARCACOES_NUM)
            {
                printf(ERROR_FFE);
                exit(1);
            }
			sscanf(line, "d%d %[^\n]", &embarcacao_index, estaleiro->embarcacoes[embarcacao_index]->matricula);
            if (!valid_matricula(estaleiro->embarcacoes[embarcacao_index]->matricula))
            {
                printf(ERROR_FFE);
                exit(1);
            }
		} else if (startsWith(line, "\tp")) {
            char *token;
            int contentorCount, pile_index, sscanf_r;
            char *contentorString;

            contentorString = (char *) malloc(sizeof(char) * 100);
            if (contentorString == NULL)
                break;
            sscanf_r = sscanf(line, "%*[^p]p%d %d %[^\n]", &pile_index, &contentorCount, contentorString);
            if (sscanf_r == 0 || sscanf_r == 1 || pile_index < 0 || pile_index >= MAX_EMBARCACAO_PILHAS_CONTENTORES)
            {
                printf(ERROR_FFE);
                exit(1);
            }
            token = strtok(contentorString, " ");
            i = 0;
            while (token != NULL && i < contentorCount) {
                Contentor *contentor/* = (Contentor*)malloc(sizeof(Contentor))*/;
                contentor = parse_contentor(token);
                token = strtok(NULL, " ");
                if (contentor == NULL) {
                    printf(ERROR_FFE);
                    free(contentor);
                    exit(1);
                }
                else
                    empilhar(estaleiro->embarcacoes[embarcacao_index]->pilhas_contentores[pile_index], contentor);
                i++;
            }
		}
	}
	fclose(fp);
}

Estaleiro *cria_estaleiro(void)
{
    Estaleiro *es;
    int i;
    es = malloc(sizeof(Estaleiro));
    for (i = 0; i < MAX_EMBARCACOES_NUM; i++)
    {
        es->embarcacoes[i] = cria_embarcacao();
    }
    return es;
}

int main(int argc, char *argv[])
{
	Estaleiro *estaleiro;
	char command[COMMAND_INPUT_MAX_LEN];
	Command *cmd;
    int print_menu;
	
	estaleiro = cria_estaleiro();
	
	if (argc >= 2) {
        read_config_file(argv[1], estaleiro);
    }

    print_menu = -1;
	while (1)
	{
        if (print_menu == -1 || print_menu == 1)
		    printf(MENU);
		printf(INPUT_MARKER);
        fgets(command, COMMAND_INPUT_MAX_LEN, stdin);
        if (strlen(command) > 0 && command[strlen(command) - 1] == '\n')
            command[strlen(command) - 1] = '\0';
		cmd = command_from_str(command);
		if (cmd->error == 1){
			printf(COMMAND_INVALID);
            print_menu = 0;
			continue;
		} else if (strcmp(cmd->name, "quit") == 0) {
			free(estaleiro);
			free(cmd);
            return 0;
		} else if (isValidCommand(cmd)) {
            int res;
            res = executeCommand(cmd, estaleiro);
            if (res == 0 || res == 1) {
                if (res == 0)
                    printf(COMMAND_INVALID);
                print_menu = 0;
            } else if (res == 2) {
                print_menu = 1;
            }
        } else {
            printf(COMMAND_INVALID);
            print_menu = 0;
        }
        if (cmd != NULL) free(cmd);
	}
    return 0;
}
