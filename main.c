#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<stdbool.h>
/*
    Nome: Fernando Augusto Serafim
    Nome: João Pedro Carvalho de Alencar
    Instituição: UNESP
*/

typedef  struct CliF {
    int CodCli; //4
    int CodF; //4
    char NomeCli[50]; //50
    char NomeF[50]; //50
    char Genero[50]; //50
}   ClienteInfo;

typedef struct REmove{
    int CodCli;
    int CodF;
} REMOVE;

bool Verifica_Existencia_Cliente(FILE** ListaClientes, REMOVE registro_remover);
int Verifica_Espacos_Vazios(FILE** ListaClientes, int TAM, int* current_size);
int verifica_Registros(FILE** ListaClientes);
int Verifica_Fim_Insere(FILE** Removidos);
int Byte_Offset_Remove(FILE** Aux, FILE* Remove, FILE* ListaClientes, FILE* insere);
int VerificaEC(int counter, FILE* Remove, FILE* ListaClientes);
int True_Size(char *buffer, int tamanho);
int Conta_SM();
void Designa_Abertura(FILE** ListaClientes, FILE** insere, FILE** Remove);
void Menu_Opcoes();
void Clientes_Excluidos(FILE** ListaClientes, FILE** auxiliar);
void Compacta_Registros(FILE* ListaClientes);
void Fecha_Arquivos(FILE** Remove, FILE** insere, FILE** ListaClientes);
void Insere_Registro(FILE* insere, FILE** ListaClientes);
void Remove_Registro(FILE* ListaClientes, FILE* removeR, FILE* insere);
void  Verifica_Repeticoes(FILE** ListaClientes,FILE** insere, int BOS);

int main()
{
    //CONTADOR PARA VERIFICAR QUANTAS VEZES TIVEMOS CASE 2
    FILE *ListaClientes, *insere, *Remove;
    int opcao = -1;

    Designa_Abertura(&ListaClientes, &insere, &Remove);

	while(opcao != 0){
        Menu_Opcoes();
        scanf("%d", &opcao);
     switch(opcao){
         case 1:{
                 Insere_Registro(insere, &ListaClientes);
                 break;
                }
         case 2:
                {
                 Remove_Registro(ListaClientes, Remove, insere);
                 break;
                }
         case 3:
                {
                Compacta_Registros(ListaClientes);
                break;
                }
         case 0:
                 printf("Encerrando...\n");
                 Fecha_Arquivos(&Remove, &insere, &ListaClientes);
                 break;
        }
    }
    return 0;
}
void Menu_Opcoes(){
    printf("1) Inserir.\n");
    printf("2) Remover.\n");
    printf("3) Compactar.\n");
    printf("0) Sair.\n");
    printf("Opcao: ");
}
int Verifica_Espacos_Vazios(FILE** ListaClientes, int TAM, int* current_size){

    int tamanho; //VERIFICADOR PARA COMPARAR COM TAM
    int BOS_PROXIMO; //O QUE TÁ DEPOIS DO CARACTERE DO MARCADOR
    int BOS; //O BYTE OFFSET DO REGISTRO COM TAMANHO SUFICIENTE

        fread(&BOS, sizeof(int), 1, *ListaClientes);

        if(BOS == -1) return 0; //lista vazia e passo recursivo
        else{
               fseek(*ListaClientes, BOS - 4, SEEK_CUR); //POIS JÁ ANDEI 4 BYTES NA LEITURA
               fread(&tamanho, sizeof(int), 1, *ListaClientes); //ARMAZENO O TAMANHO DESSE REGISTRO
               if(tamanho > TAM){
                    fseek(*ListaClientes, 1, SEEK_CUR); //pulo o byte do '*'
                        fread(&BOS_PROXIMO, sizeof(int), 1, *ListaClientes);
                            fseek(*ListaClientes, 0, SEEK_SET);
                            fwrite(&BOS_PROXIMO, sizeof(int), 1, *ListaClientes);
                        *current_size = tamanho;
                    return BOS;
               }else BOS = Verifica_Espacos_Vazios(ListaClientes, TAM, current_size);
        }

    return BOS;
}
void Insere_Registro(FILE* insere,FILE** ListaClientes){

    FILE* Registros_Inseridos;
    ClienteInfo cliente;
    char buffer[160];
    char ch = ' ';
    char simbolo = '+';
    int recebe, retorno, BOS;
    int previous_size, aux;

    if( (Registros_Inseridos = fopen("Auxiliar_Inseridos.bin", "a+b") ) == NULL){
        printf("Erro ao abrir arquivo!\n");
        exit(1);
    }
    if(ftell(Registros_Inseridos) > 0){
           fclose(Registros_Inseridos);
        if( (Registros_Inseridos = fopen("Auxiliar_Inseridos.bin", "r+b") ) == NULL){
        printf("Erro ao abrir arquivo!\n");
        exit(1);
        }
    }
    BOS = Verifica_Fim_Insere(&Registros_Inseridos);//RETORNA QUANTOS REG. JÁ FORAM INSERIDOS
    fseek(insere, 0, SEEK_END); //4 registros de 160 bytes
    if(ftell(insere) == BOS){
        printf("Nao ha mais registros a serem inseridos...\n\n");
        return;
    }
    fseek(insere, BOS, SEEK_SET);
     fread(&cliente, sizeof(ClienteInfo), 1, insere); //LEIO A POSIÇÃO "BOS" DO INSERE
        sprintf(buffer,"%d#%d#%s#%s#%s#",cliente.CodCli, cliente.CodF,
            cliente.NomeCli,cliente.NomeF,cliente.Genero);
                recebe = strlen(buffer);
                    fseek(*ListaClientes, 0, 0);
                        retorno = Verifica_Espacos_Vazios(ListaClientes, recebe, &previous_size);

        if(retorno == 0){ //LISTA VAZIA
            fseek(*ListaClientes, 0, SEEK_END);
                fwrite(&recebe, sizeof(recebe), 1, *ListaClientes);
                    fwrite(buffer, recebe, 1, *ListaClientes);
                        fwrite(&simbolo, sizeof(char), 1, Registros_Inseridos);
            printf("\nREGISTRO INSERIDO COM SUCESSO\n\n");
        }else{ //LISTA COM ELEMENTOS
                char Lixo_Memoria;

                fseek(*ListaClientes, retorno, SEEK_SET);
                    fwrite(&previous_size, sizeof(int), 1, *ListaClientes);
                        fwrite(buffer, recebe, 1, *ListaClientes);
                            aux = previous_size - recebe; //qtd de bytes que devo deixar em branco
                                Lixo_Memoria = aux;
                                //fwrite(&ch, sizeof(char), aux, *ListaClientes);
                do  //VERIFICAR COM A VERÔNICA
                {
                    fwrite(&ch, sizeof(char), 1, *ListaClientes);
                    aux = aux - 1;
                }while(aux > 0);
                fseek(*ListaClientes, - aux - 2, SEEK_CUR);
                    fwrite(&Lixo_Memoria, sizeof(char), 1, *ListaClientes);
                        fseek(*ListaClientes, 0, SEEK_END);
                            fwrite(&simbolo, sizeof(char), 1, Registros_Inseridos);
                                printf("\nREGISTRO INSERIDO COM SUCESSO\n\n");
            }
        fclose(Registros_Inseridos);
}
int Verifica_Fim_Insere(FILE** Registros){

   char buffer;
   int counter = 0;
    fseek(*Registros, 0, SEEK_SET);
    while(fread(&buffer, sizeof(char), 1, *Registros)){
        fseek(*Registros, -1, SEEK_CUR);
        if(fgetc(*Registros) == '+') counter++;
    }
   return counter * sizeof(ClienteInfo);
}
int Verifica_Registros(FILE** ListaClientes){
//ESSA FUNÇÃO VAI RETORNAR O BYTE OFFSET DO PRÓXIMO REGISTRO QUE DEVEMOS SELECIONAR
//DE "INSERE.BIN" E INSERIR EM "LISTACLIENTES.BIN"
   int BOS = 0;
   int contador = -1; //ELE RETORNA QUANTOS REGISTROS EU CONSEGUI EFETIVAMENTE LER

   fseek(*ListaClientes, 4, SEEK_SET);

   do{
        fseek(*ListaClientes, BOS, SEEK_CUR);
        contador = contador + 1;
     }while( fread(&BOS, sizeof(int), 1, *ListaClientes) );

   return contador * sizeof(ClienteInfo);
}
void Designa_Abertura(FILE** ListaClientes, FILE** insere, FILE** Remove){

    int BOS = 0;
    int header = -1;

    if( (*ListaClientes = fopen("ListaClientes.bin", "a+b") ) == NULL){
            printf("ERRO AO ABRIR O ARQUIVO\n");
            exit(1);
    }

    if((*Remove = fopen("remove.bin", "r+b")) == NULL){
            printf("ERRO AO ABRIR O ARQUIVO\n");
        exit(1);

    }

    if((*insere = fopen("insere.bin", "r+b")) == NULL){
            printf("ERRO AO ABRIR O ARQUIVO\n");
        exit(1);
    }

    fseek(*ListaClientes, 0, SEEK_END);

    if(ftell(*ListaClientes) > 0){
        fclose(*ListaClientes);
            if( (*ListaClientes = fopen("ListaClientes.bin", "r+b") ) == NULL){
                printf("ERRO AO ABRIR O ARQUIVO\n");
                return;
            }
                fseek(*ListaClientes, 0, SEEK_SET);
            BOS = Verifica_Registros(ListaClientes); //SÓ PARA GARANTIR QUE NÃO TEM REPETIÇÃO
        fseek(*insere, BOS, SEEK_SET); //MOVI O INSERE PARA O ÚLTIMO INSERIDO
    }

    fseek(*ListaClientes, 0, SEEK_END);
        if(ftell(*ListaClientes) == 0)
            fwrite(&header, sizeof(int), 1, *ListaClientes);
                fseek(*ListaClientes, 0, SEEK_END);
}
void Remove_Registro(FILE* ListaClientes, FILE* removeR, FILE* insere){
    FILE* auxiliar;
    char char_estrela = '+';
    char char_eC = '&';
    REMOVE registro_remover;
    int Retorno;

    if( (auxiliar = fopen("Auxiliar_Removidos.bin", "a+b") ) == NULL){
        printf("Erro ao abrir arquivo!\n");
        exit(1);
    }
   if(ftell(ListaClientes) > 0){
        fclose(ListaClientes);
        if( (ListaClientes = fopen("ListaClientes.bin", "r+b") ) == NULL){
            printf("Erro ao abrir arquivo!\n");
            exit(1);
        }
   }
   fseek(ListaClientes, 0, SEEK_END);
   if( ftell(ListaClientes) == sizeof(int) ){
        printf("Nenhum registro inserido!\n\n");
        return;
   }

   fseek(removeR, 0, SEEK_END);
    Retorno = Byte_Offset_Remove(&auxiliar, removeR, ListaClientes, insere);

    if(Retorno == -1){
        printf("Nao e possivel realizar mais remocoes...\n\n");
        return;
    }else{
            fseek(removeR, Retorno * sizeof(REMOVE), SEEK_SET);
              fread(&registro_remover, sizeof(REMOVE), 1, removeR);
                 fseek(removeR, 0, SEEK_END);
        if(!(Verifica_Existencia_Cliente(&ListaClientes, registro_remover))){
            fclose(auxiliar);
            if( (auxiliar = fopen("Auxiliar_Removidos.bin", "a+b") ) == NULL){
            printf("Erro ao abrir arquivo!\n");
            exit(1);
            }
            if(Retorno * sizeof(REMOVE) < ftell(removeR))
                {
                    fwrite(&char_eC, sizeof(char), 1, auxiliar);
                }
        }else{
            if(Retorno * sizeof(REMOVE) < ftell(removeR))
               {
                   fwrite(&char_estrela, sizeof(char), 1, auxiliar);
               }
        }
    }

   fclose(auxiliar);
}
int Byte_Offset_Remove(FILE** Aux, FILE* Remove, FILE* ListaClientes, FILE* insere){
//CONVERSÃO DE "&" PARA "+"; QND TUDO FOR "+", NÃO TEM MAIS COMO REMOVER ALGO
    char buffer = ' ', ch = '+';
    int counter = 0, retorno = 0, recebe = -1, counter_SM = 0;

    fseek(*Aux, 0, SEEK_END);
    if(ftell(*Aux) > 0){
        fclose(*Aux);
        if( (*Aux = fopen("Auxiliar_Removidos.bin", "r+b") ) == NULL){
            printf("Erro ao abrir arquivo!\n");
            exit(1);
        }
    }
    retorno = Conta_SM(); //VERIFICANDO QUANTAS INSERÇÕES FIZEMOS
        fseek(*Aux, 0, SEEK_SET);
            fseek(insere, 0, SEEK_END);
                fseek(Remove, 0, SEEK_END);
    while(fread(&buffer, sizeof(char), 1, *Aux)){
        counter++;
        if(buffer == '+') counter_SM++;
        if(retorno * sizeof(ClienteInfo) == ftell(insere))
        {
            if(buffer == '&'){
            recebe = VerificaEC(counter, Remove, ListaClientes);
            if(recebe == 0 && (retorno * sizeof(ClienteInfo) == ftell(insere)) ){ //retorno * sizeof(ClienteInfo) == ftell(insere) é redundante (tem em cima já)
                   fseek(*Aux, counter - 1, SEEK_SET);
                        fwrite(&ch, sizeof(ch), 1, *Aux);// "&" --> "+"
                if(counter * sizeof(REMOVE) < ftell(Remove)) return counter;
                else break;
               }//TROCA QUANDO NÃO TEM MAIS O QUE PROCURAR
            if(recebe > 0){
                    fseek(*Aux, counter - 1, SEEK_SET);
                    fwrite(&ch, sizeof(char), 1, *Aux);// "&" --> "+"
                        return counter;
            }//TROCA QUANDO CONSEGUIU UMA CHAVE PARA O "&"
            }
        }
    }
    if (counter_SM * sizeof(REMOVE) == ftell(Remove)) return -1;
    return counter;
}
int Conta_SM(){

    FILE* auxiliar_inseridos = NULL;
    char ch = ' ';
    int counter = 0;

    if((auxiliar_inseridos = fopen("Auxiliar_Inseridos.bin", "a+b")) == NULL){
        printf("Erro na abertura do arquivo...\n\n");
        exit(1);
    }

    while(fread(&ch, sizeof(char), 1, auxiliar_inseridos)){
        if(ch == '+') counter++;
    }

    fclose(auxiliar_inseridos);
    return counter;
}
int VerificaEC(int counter, FILE* Remove, FILE* ListaClientes){
//FUNÇÃO QUE VARRE LISTA CLIENTES VERIFICANDO SE A CHAVE DO "&" CONSEGUE EXCLUIR
//COUNTER É A POSIÇÃO DA CHAVE NO REGISTRO REMOVE.

    REMOVE leitura = {0,0};
    int Tamanho = 0;
    char CodCli = ' ', CodF = ' ', CodCli2 = ' ', CodF2 = ' ';

    fseek(Remove, counter * sizeof(REMOVE), SEEK_SET);
        fread(&leitura, sizeof(REMOVE), 1, Remove);
            fseek(ListaClientes, 4, SEEK_SET);

     while(fread(&Tamanho, sizeof(int), 1, ListaClientes)){
        if(fgetc(ListaClientes) == '*') fseek(ListaClientes, Tamanho - 1, SEEK_CUR);
        else{
             fseek(ListaClientes, -1, SEEK_CUR);
             CodCli = fgetc(ListaClientes);
                fseek(ListaClientes, sizeof(char), SEEK_CUR);
                  CodF = fgetc(ListaClientes);
                    itoa(leitura.CodF, &CodF2, 10);
                      itoa(leitura.CodCli, &CodCli2, 10);
            if(CodCli == CodCli2 && CodF == CodF2) return counter;
            else fseek(ListaClientes, Tamanho - 3, SEEK_CUR);
        }
     }

    return 0;//Só chega nesse ponto se não encontrou nenhuma exclusão possível
}
bool Verifica_Existencia_Cliente(FILE** ListaClientes, REMOVE registro_remover){

    char CodCli2;
    char CodF2;
    char CodCli, CodF;
    int BOS;
    char ch = '*';
    int Tamanho_Registro, header;

    itoa(registro_remover.CodF, &CodF2, 10);
        itoa(registro_remover.CodCli, &CodCli2, 10);
            rewind(*ListaClientes);
                fread(&header, sizeof(int), 1, *ListaClientes);
                    fread(&Tamanho_Registro, sizeof(int), 1, *ListaClientes);
    do
    {
        if(fgetc(*ListaClientes) == ch)
        fseek(*ListaClientes, Tamanho_Registro - 1, SEEK_CUR);
        else{
             fseek(*ListaClientes, -1, SEEK_CUR);
                CodCli = fgetc(*ListaClientes); //+1 byte
                    fseek(*ListaClientes, sizeof(char), SEEK_CUR);//+1 byte
                        CodF = fgetc(*ListaClientes); //+1 byte
        if((CodCli2 == CodCli) && (CodF2 == CodF)){
                    fseek(*ListaClientes, -3, SEEK_CUR); //4 bytes
                        fwrite(&ch, sizeof(char), 1, *ListaClientes);//1 byte
                            fwrite(&header, sizeof(int), 1, *ListaClientes); //4 bytes
                                break;
        }else fseek(*ListaClientes, Tamanho_Registro - 3, SEEK_CUR);
      }
        if(fgetc(*ListaClientes) == EOF){
            printf("O REGISTRO PEDIDO NÃO FOI ENCONTRADO\n\n");
            return false;
        }else fseek(*ListaClientes, -1, SEEK_CUR);

    }while(fread(&Tamanho_Registro, sizeof(int), 1, *ListaClientes));

    BOS = (ftell(*ListaClientes) - 9);
        fseek(*ListaClientes, 0, 0);
            fwrite(&BOS, sizeof(int), 1, *ListaClientes);//Atualizo o header
                    fseek(*ListaClientes, 0, SEEK_END);
                        printf("REGISTRO REMOVIDO COM SUCESSO\n\n");
                            return true;
}
void Compacta_Registros(FILE* ListaClientes){

	FILE *arquivoCompactado;
    char buffer[160];
	int tamanho = 0, header = -1;
	int callback = 0;

	if((arquivoCompactado = fopen("compactados.bin", "ab")) == NULL){
		printf("Erro ao abrir o arquivo!\n");
	}

    fwrite(&header, sizeof(int), 1, arquivoCompactado);
        fseek(ListaClientes, 4, SEEK_SET);

	while(fread(&tamanho, sizeof(int), 1, ListaClientes)){
		if(fgetc(ListaClientes) == '*') fseek(ListaClientes, tamanho - 1, SEEK_CUR);
		else{
                fseek(ListaClientes, -1, SEEK_CUR);
	            fread(buffer, sizeof(char), tamanho, ListaClientes);
	            callback = True_Size(buffer, tamanho);
	            fwrite(&callback, sizeof(int), 1, arquivoCompactado);
	            fwrite(buffer, sizeof(char), callback, arquivoCompactado);
		}
	}

    fclose(ListaClientes);
        fclose(arquivoCompactado);
            remove("ListaClientes.bin");
        rename("compactados.bin", "ListaClientes.bin");
    printf("Arquivo compactado com sucesso...\n\n");
}
int True_Size(char *buffer, int tamanho){

 int counter = 0;
 char size;
 int converter = 0;

    for(int i = 0; i<tamanho; i++){
        if(buffer[i] == '#') counter++;
        if(counter == 5 && buffer[i+1] != '\0'){
             size = buffer[i+1];
             converter = atoi(&size);
             return tamanho - converter;
             //return tamanho - (size - 48);
        }
    }
    return tamanho;
}
void Fecha_Arquivos(FILE** Remove, FILE** insere, FILE** ListaClientes){
    fclose(*Remove);
    fclose(*insere);
    fclose(*ListaClientes);
}
