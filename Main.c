#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 10
#define BUFFER_SIZE 1024

struct node {
    int id;
    struct node *next;
};

struct ator {
    int id;
    char *name;
    struct node *filmes;
};

struct filme {
    int id;
    char *title;
    struct node *neighbors;
};

struct ArrayDinamico {
    struct ator *data;
    int size;
    int tamanho;
};

struct listaAdjacencia {
    struct filme *data;
    int size;
    int tamanho;
};

void inicializarArray(struct ArrayDinamico *arr) {
    arr->data = malloc(CAPACIDADEINICIAL * sizeof(struct ator));
    arr->size = 0;
    arr->tamanho = CAPACIDADEINICIAL;
}

void adicionarArrayDinamico(struct ArrayDinamico *arr, struct ator actor) {
    if (arr->size >= arr->tamanho) {
        arr->tamanho *= 2;
        arr->data = realloc(arr->data, arr->tamanho * sizeof(struct ator));
    }
    arr->data[arr->size++] = actor;
}

void inicializarListaAdjacencia(struct listaAdjacencia *list) {
    list->data = malloc(CAPACIDADEINICIAL * sizeof(struct filme));
    list->size = 0;
    list->tamanho = CAPACIDADEINICIAL;
}

void adicionarListaAdjacencia(struct listaAdjacencia *list, struct filme movie) {
    if (list->size >= list->tamanho) {
        list->tamanho *= 2;
        list->data = realloc(list->data, list->tamanho * sizeof(struct filme));
    }
    list->data[list->size++] = movie;
}

struct node* criarNode(int id) {
    struct node *new_node = malloc(sizeof(struct node));
    new_node->id = id;
    new_node->next = NULL;
    return new_node;
}

void add_edge(struct filme *movie, int neighbor_id) {
    struct node *new_node = criarNode(neighbor_id);
    new_node->next = movie->neighbors;
    movie->neighbors = new_node;
}

void add_movie(struct ator *actor, int movie_id) {
    struct node *new_node = criarNode(movie_id);
    new_node->next = actor->filmes;
    actor->filmes = new_node;
}

int extract_id(char *str) {
    return atoi(str + 2); // Remove os dois primeiros caracteres (tt ou nm)
}

void read_artists(const char *filename, struct ArrayDinamico *actors) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    fgets(buffer, BUFFER_SIZE, file); // Skip header

    int counter = 0;
    while (fgets(buffer, BUFFER_SIZE, file)) {
        if (counter++ >= 200) break; // Limita a leitura para testes

        struct ator actor;
        char *token = strtok(buffer, "\t");
        actor.id = extract_id(token);
        
        token = strtok(NULL, "\t");
        actor.name = strdup(token);
        
        actor.filmes = NULL;
        token = strtok(NULL, "\t"); // Skip birthYear
        token = strtok(NULL, "\t"); // Skip deathYear
        token = strtok(NULL, "\t"); // Skip primaryProfession
        
        token = strtok(NULL, "\t");
        char *movie_id_str = strtok(token, ",");
        while (movie_id_str) {
            int movie_id = extract_id(movie_id_str);
            add_movie(&actor, movie_id);
            movie_id_str = strtok(NULL, ",");
        }
        adicionarArrayDinamico(actors, actor);
    }
    fclose(file);
}

void read_movies(const char *filename, struct listaAdjacencia *movies) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    fgets(buffer, BUFFER_SIZE, file); // Skip header

    while (fgets(buffer, BUFFER_SIZE, file)) {
        char *token = strtok(buffer, "\t");
        int id = extract_id(token);

        token = strtok(NULL, "\t");
        if (strcmp(token, "movie") != 0) continue; // Skip non-movie entries

        struct filme movie;
        movie.id = id;
        
        token = strtok(NULL, "\t");
        movie.title = strdup(token);
        
        movie.neighbors = NULL;
        adicionarListaAdjacencia(movies, movie);
    }
    fclose(file);
}

struct filme* find_movie(struct listaAdjacencia *movies, int id) {
    int i;
    for (i = 0; i < movies->size; i++) {
        if (movies->data[i].id == id) {
            return &movies->data[i];
        }
    }
    return NULL;
}

void connect_movies(struct listaAdjacencia *movies, struct ArrayDinamico *actors) {
    int i;
    for (i = 0; i < actors->size; i++) {
        struct ator *actor = &actors->data[i];
        struct node *movie1 = actor->filmes;
        while (movie1) {
            struct node *movie2 = movie1->next;
            while (movie2) {
                struct filme *m1 = find_movie(movies, movie1->id);
                struct filme *m2 = find_movie(movies, movie2->id);
                if (m1 && m2) {
                    add_edge(m1, movie2->id);
                    add_edge(m2, movie1->id);
                }
                movie2 = movie2->next;
            }
            movie1 = movie1->next;
        }
    }
}

void print_dot(struct listaAdjacencia *movies, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "graph {\n  concentrate=true\n");
    int i;
    for ( i = 0; i < movies->size; i++) {
        struct filme *movie = &movies->data[i];
        struct node *neighbor = movie->neighbors;
        while (neighbor) {
            struct filme *adj_movie = find_movie(movies, neighbor->id);
            if (adj_movie && movie->id < adj_movie->id) { // Avoid duplicating edges
                fprintf(file, "  \"%s\" -- \"%s\";\n", movie->title, adj_movie->title);
            }
            neighbor = neighbor->next;
        }
    }
    fprintf(file, "}\n");
    fclose(file);
}

int main() {
    struct ArrayDinamico actors;
    inicializarArray(&actors);
    read_artists("C:\\Users\\luizg\\Desktop\\TrabalhoEstrutura\\name.basics.tsv", &actors);

    struct listaAdjacencia movies;
    inicializarListaAdjacencia(&movies);
    read_movies("C:\\Users\\luizg\\Desktop\\TrabalhoEstrutura\\title.basics.tsv", &movies);

    connect_movies(&movies, &actors);

    print_dot(&movies, "input.dot");

    // Free allocated memory (omitted for brevity)
    return 0;
}
