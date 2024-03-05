#include <stdio.h>
#include <locale.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>
#include <math.h>
#define MAX(a, b) ((a) > (b) ? (a) : (b))
typedef struct
{
    char* surname; 		// фамилия клиента
    char* name;		    // имя клиента
    double amount;		// Сумма на счете (сумма депозита )
}bank;
typedef struct
{
    bank *accounts;
    int capacity, size;
}bank_table;

int interface(int command, bank_table *table);
int add_new_elements(bank_table *table, FILE *stream);
int bank_table_register(bank_table *table, char *surname, char* name, double amount );
int bank_table_construct(bank_table *table);
int bank_table_add(bank_table *table, bank new_account);
void bank_table_print(bank_table *table, FILE* stream);
static void fprint_padding(FILE* stream, char padding_symbol, size_t size);
bank_table bank_table_search(bank_table *table, char* name, char* surname );
bank_table bank_table_search_amount(bank_table *table, double amount);
bank account_copy(bank copied);
void bank_table_destroy(bank_table *table);
int accounts_table(bank_table table, FILE *stream);
bank_table *read_table(bank_table *table, FILE *stream);

int main(){
    FILE* Data = fopen("data.txt", "r");
    setlocale(LC_ALL, "Rus");
    bank_table table = {};

    bank_table_construct(&table);

    read_table(&table, Data);

    fclose(Data);

    printf("Добавить новый элемент 	                 1\n\
               Распечатать базу товаров	             2\n\
               Поиск клиента по фамилии и имени      3\n\
               Фильтр по цене	                     4\n\
               Выход из программы	                 5\n");
    int command = 0;
    do {
        printf("Введите команду: ");
        scanf("%d", &command);
    } while(interface(command, &table));

    Data = fopen("data.txt", "w");
    accounts_table(table, Data);
    fclose(Data);
    bank_table_destroy(&table);
    return 0;
}
bank account_copy(bank copied)
{
    copied.name = strdup(copied.name);
    copied.surname = strdup(copied.surname);
    return copied;
}


int interface(int command, bank_table *table)
{
    bank_table selected_accounts;
    char* name_to_find;
    char* surname_to_find;

    switch(command)
    {
        case 1:
            add_new_elements(table, stdin);
            return 1;

        case 2:
            bank_table_print(table, stdout);

            return 1;

        case 3:

            printf("Введите фамилию и имя, разделённые пробелом, искомого клиента: ");

            while (scanf("%s %s, &name_to_find, &surname_to_find") != 2)
            {
                printf("Введите в формате (surname name) фамилию и имя искомого клиента");
            }
            selected_accounts = bank_table_search(table, name_to_find, surname_to_find);
            bank_table_print(&selected_accounts, stdout);
            bank_table_destroy(&selected_accounts);
            return 1;
        case 4:
            printf("Введите неотрицательное число: ");
            double filter_amount;


            while (1)
            {
                if (scanf("%lf", &filter_amount) != 1)
                {
                    printf("Введите число, пожалуйста: ");
                    continue;
                }
                if (filter_amount < 0)
                {
                    printf("Введите неотрицательное число, пожалуйста: ");
                    continue;
                }
                break;
            }
            bank_table selected_accounts;
            selected_accounts = bank_table_search_amount(table, filter_amount);
            bank_table_print(&selected_accounts, stdout);
            bank_table_destroy(&selected_accounts);


            return 1;
        case 5:
            printf("\nКонец работы\n");
            return 0;
        default:
            printf("Неправильная команда попытайтесь ввести число от 1 до 5\n");
            return 1;

    }

}
int add_new_elements(bank_table *table, FILE *stream)
{
    int S_LEN = 100;

    if (stream == stdin)
        printf("Введите фамилию, имя и сумму депозита(разделённые пробелом), введите \"stop\" когда закончите");

    char *name = (char*) calloc(1, S_LEN);
    char *surname = (char*) calloc(1, S_LEN);
    double amount = 0;
    for (;;)
    {
        fscanf(stream, "%s", &surname);
        if (!strcmp(surname, "stop")) {
            break;
        }
        fscanf(stream, "%s", &name);
        fscanf(stream, "%lf", &amount);
        bank_table_add(table, (bank) {surname, name, amount});
    }

    return 0;
}

int bank_table_construct(bank_table *table)
{
    const size_t MIN_CAPACITY = 8;

    bank *array = (bank*) calloc(MIN_CAPACITY, sizeof(*array));

    // ENOMEM return in errno, caller is supposed to check
    if (array == NULL)
        return -1;

    *table = (bank_table) {
        .accounts = array,
        .capacity = MIN_CAPACITY,
        .size = 0
    };

    return 0;
}
int bank_table_add(bank_table *table, bank new_account)
{
    const double GROW_FACTOR = 2;

    if (table->size == table->capacity) {
        size_t new_capacity = table->capacity * GROW_FACTOR;
        bank *new_accounts = (bank*) realloc(table->accounts, new_capacity * sizeof(*table->accounts));


        if (table->accounts == NULL)
            return -1;

        table->capacity = new_capacity;
        table->accounts = new_accounts;
    }

    table->accounts[table->size ++] = new_account;
    return 0;
}

#define FPRINTF_PADDED(stream, target_length, ...)                                    \
    do {                                                                              \
        int padding_length = target_length - fprintf(stream, __VA_ARGS__);            \
        assert(padding_length >= 0 && "String is too large, can't pad backwards :|"); \
                                                                                      \
        fprint_padding(stream, ' ', padding_length);                                  \
    } while(0)

void bank_table_print(bank_table *table, FILE* stream)
{
    const int MIN_COLUMN_WIDTH = 16;
    const int MIN_SPACING = 4;


    size_t max_name_size = 0;
    size_t i;
    for ( i = 0; i < table->size; ++ i)
        max_name_size = MAX(strlen(table->accounts[i].name), max_name_size);
    size_t max_surname_size = 0;

    for ( i = 0; i < table->size; ++ i)
        max_surname_size = MAX(strlen(table->accounts[i].surname), max_surname_size);
    size_t max_amount_size = MIN_COLUMN_WIDTH;




    size_t name_alignment = MAX(max_name_size, MIN_COLUMN_WIDTH) + MIN_SPACING;
    size_t surname_alignment = MAX(max_surname_size, MIN_COLUMN_WIDTH) + MIN_SPACING;
    size_t amount_alignment = MAX(max_amount_size, MIN_COLUMN_WIDTH) + MIN_SPACING;


    FPRINTF_PADDED(stream, surname_alignment, "Фамилия клиента");
    FPRINTF_PADDED(stream, name_alignment, "Имя клиента");
    FPRINTF_PADDED(stream, amount_alignment, "Сумма на счёте");

    fprintf(stream, "\n");



    const int total_length = surname_alignment + name_alignment + amount_alignment;
    fprint_padding(stream, '-', total_length);
    fprintf(stream, "\n");
    double total_amount = 0;
    for (i = 0; i < table->size; ++i) {
        bank *current_account = &table->accounts[i];
        double total_current_amount = current_account->amount;

        FPRINTF_PADDED(stream, surname_alignment, "%s", current_account->surname);
        FPRINTF_PADDED(stream, name_alignment, "%s", current_account->name);
        FPRINTF_PADDED(stream, amount_alignment, "%.2lf", current_account->amount);


        fprintf(stream, "\n");

        total_amount += total_current_amount;
    }
    fprint_padding(stream, '-', total_length);
    fprintf(stream, "\n");
    FPRINTF_PADDED(stream, 0, "Total amount");
    fprintf(stream, "%.2lf\n", total_amount);

}

static void fprint_padding(FILE* stream, char padding_symbol, size_t size) {
    size_t i;
    for ( i = 0; i < size; ++ i)
        fprintf(stream, "%c", padding_symbol);
}
bank_table bank_table_search(bank_table *table, char* name, char* surname)
{
    bank_table selected_table = {};
    bank_table_construct(&selected_table);
    for (int i = 0; i < table->size; ++i)
    {
        if (table->accounts[i].name == name && table->accounts[i].surname == surname)
            bank_table_add(&selected_table, account_copy(table->accounts[i]));
    }
    return selected_table;

}
bank_table bank_table_search_amount(bank_table *table, double amount)
{
    bank_table selected_table = {};
    bank_table_construct(&selected_table);
    for (int i = 0; i < table->size; ++i)
    {
        if (table->accounts[i].amount > amount)
            bank_table_add(&selected_table, account_copy(table->accounts[i]));
    }
    return selected_table;
}
void bank_table_destroy(bank_table *table)
{
    if (table->accounts != NULL)
    {
        for (int i = 0; i < table->size; ++i)
        {
            free(table->accounts[i].name);
            free(table->accounts[i].surname);
        }
        free(table->accounts);
        *table = (bank_table) {0, 0, 0};
    }
}
int accounts_table(bank_table table, FILE *stream)
{
    for (int i = 0; i < table.size; i++)
    {
        fprintf(stream, "%s %s %lf", table.accounts[i].name, table.accounts[i].surname, table.accounts[i].amount);
    }

    fprintf(stream, "stop\n");
    return 0;

}
bank_table *read_table(bank_table* table, FILE *stream)
{
    bank_table_construct(table);
    add_new_elements(table, stream);
    return table;
}
