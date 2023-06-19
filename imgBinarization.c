//aluno: Arthur Souza Santos Romão a2257793
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned short u_int2;

typedef struct Node
{
    u_int2 row;
    u_int2 col;
    u_int2 value;
    struct Node *right;
    struct Node *down;
} Node;

typedef struct
{
    u_int2 rows;
    u_int2 cols;
    Node *start;
    Node *last;
} SparseMatrix;

SparseMatrix *createSparseMatrix(int rows, u_int2 cols);
void insertNode(SparseMatrix *matrix, u_int2 row, u_int2 col, u_int2 value);
void freeSparseMatrix(SparseMatrix *matrix);
void thresholdLocal(SparseMatrix *matrix, u_int2 threshold);
void saveAsPgmFile(SparseMatrix *matrix, const char *filename);
SparseMatrix *loadImage(const char *filename);
int findLocalThreshold(SparseMatrix *matrix);

int main()
{

    char inputFilename[100];
    const char *outputFilename = "output.pgm";
    u_int2 threshold;

    printf("Digite o nome do arquivo de entrada (com o formato do arquivo EX: \"moca.pgm\") : ");
    scanf("%s", inputFilename);

    SparseMatrix *matrix = loadImage(inputFilename);
    if (matrix == NULL)
    {
        return 1;
    }

    threshold = findLocalThreshold(matrix);
    thresholdLocal(matrix, threshold);
    saveAsPgmFile(matrix, outputFilename);

    freeSparseMatrix(matrix);

    printf("Imagem processada com sucesso!\n");

    return 0;
}

SparseMatrix *createSparseMatrix(int rows, u_int2 cols)
{
    SparseMatrix *matrix = (SparseMatrix *)malloc(sizeof(SparseMatrix));
    matrix->rows = rows;
    matrix->cols = cols;
    matrix->start = NULL;
    matrix->last = NULL;
    return matrix;
}

void insertNode(SparseMatrix *matrix, u_int2 row, u_int2 col, u_int2 value)
{
    Node *newNode = (Node *)malloc(sizeof(Node));
    newNode->row = row;
    newNode->col = col;
    newNode->value = value;
    newNode->right = newNode->down = NULL;

    if (matrix->start == NULL)
    {
        matrix->start = newNode;
        matrix->last = newNode;
    }
    else
    {
        matrix->last->right = newNode;
        matrix->last = newNode;
    }
}

void freeSparseMatrix(SparseMatrix *matrix)
{
    Node *temp = matrix->start;
    while (temp != NULL)
    {
        Node *prev = temp;
        temp = temp->right;
        free(prev);
    }
    free(matrix);
}

void thresholdLocal(SparseMatrix *matrix, u_int2 threshold)
{
    Node *temp = matrix->start;
    while (temp != NULL)
    {
        if (temp->value >= threshold)
        {
            temp->value = 255;
        }
        else
        {
            temp->value = 0;
        }
        temp = temp->right;
    }
}

void saveAsPgmFile(SparseMatrix *matrix, const char *filename)
{
    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        printf("Erro ao criar arquivo %s\n", filename);
        return;
    }

    fprintf(file, "P2\n");
    fprintf(file, "%hu %hu\n", matrix->cols, matrix->rows);
    fprintf(file, "255\n");

    Node *temp = matrix->start;
    u_int2 currentRow = 0;
    u_int2 currentCol = 0;

    while (currentRow < matrix->rows)
    {
        if (temp != NULL && temp->row == currentRow && temp->col == currentCol)
        {
            fprintf(file, "%hu\n", temp->value);
            temp = temp->right;
            currentCol++;
        }
        else
        {
            fprintf(file, "255\n");
            currentCol++;
        }

        if (currentCol >= matrix->cols)
        {
            currentCol = 0;
            currentRow++;
        }
    }

    fclose(file);
}

SparseMatrix *loadImage(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Erro ao abrir arquivo %s\n", filename);
        return NULL;
    }

    char fileID[3];
    fscanf(file, "%2s", fileID);
    if (fileID[0] != 'P' || fileID[1] != '2')
    {
        printf("Formato de arquivo inválido.\n");
        fclose(file);
        return NULL;
    }

    u_int2 cols, rows, maxValue;
    fscanf(file, "%hu %hu", &cols, &rows);
    fscanf(file, "%hu", &maxValue);

    SparseMatrix *matrix = createSparseMatrix(rows, cols);

    u_int2 value;
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            fscanf(file, "%hu", &value);
            if (value != 255)
            {
                insertNode(matrix, i, j, value);
            }
        }
    }

    fclose(file);
    return matrix;
}

int findLocalThreshold(SparseMatrix *matrix)
{
    u_int2 histogram[256] = {0};

    Node *temp = matrix->start;
    while (temp != NULL)
    {
        histogram[temp->value]++;
        temp = temp->right;
    }

    u_int2 maxPeak1 = 0;
    u_int2 maxPeak2 = 0;
    u_int2 maxPeak1Index = 0;
    u_int2 maxPeak2Index = 0;

    for (int i = 0; i < 256; i++)
    {
        if (histogram[i] > maxPeak1)
        {
            maxPeak2 = maxPeak1;
            maxPeak2Index = maxPeak1Index;
            maxPeak1 = histogram[i];
            maxPeak1Index = i;
        }
        else if (histogram[i] > maxPeak2)
        {
            maxPeak2 = histogram[i];
            maxPeak2Index = i;
        }
    }

    if (maxPeak1Index == 0 || maxPeak2Index == 0)
    {
        printf("Imagem binária.\nValor do threshold: 255\n");
        printf("\nHistograma: ");
        for(int i = 0; i < 255;i++) printf("%d ",histogram[i]);
        return 255;
    }

    u_int2 max = maxPeak1Index > maxPeak2Index ? maxPeak1Index : maxPeak2Index;
    u_int2 min = maxPeak1Index < maxPeak2Index ? maxPeak1Index : maxPeak2Index;

    int threshold = 0;
    u_int2 counter = 0;

    printf("\nIndex do primeiro pico: %d\nIndex do segundo pico: %d\n", min, max);

    for (u_int2 i = min; i < max; i++)
    {
        threshold += i;
        counter++;
    }

    threshold = threshold / counter;

    printf("Valor do threshold: %d\n", threshold);
    printf("\nHistograma: ");
        for(int i = 0; i < 255;i++) printf("%d ",histogram[i]);
    return threshold;
}