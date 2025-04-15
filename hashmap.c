#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "hashmap.h"


typedef struct HashMap HashMap;
int enlarge_called=0;

struct HashMap {
    Pair ** buckets;
    long size; //cantidad de datos/pairs en la tabla
    long capacity; //capacidad de la tabla
    long current; //indice del ultimo dato accedido
};

// Crea un nuevo par (clave, valor) y lo retorna
Pair * createPair(char * key, void * value) {
    Pair * new = (Pair *)malloc(sizeof(Pair)); // Reserva memoria para el par
    new->key = key;      // Asigna la clave
    new->value = value;  // Asigna el valor
    return new;
}

// Función hash para obtener una posición en el arreglo a partir de la clave
long hash(char * key, long capacity) {
    unsigned long hash = 0;
    char * ptr;
    for (ptr = key; *ptr != '\0'; ptr++) {
        hash += hash * 32 + tolower(*ptr); // Acumulación basada en los caracteres de la clave (ignorando mayúsculas)
    }
    return hash % capacity; // Se retorna el valor dentro del rango de la capacidad
}

// Compara dos claves, retorna 1 si son iguales, 0 si no lo son
int is_equal(void* key1, void* key2) {
    if (key1 == NULL || key2 == NULL) return 0;
    if (strcmp((char*)key1, (char*)key2) == 0) return 1; // Compara cadenas
    return 0;
}

// Inserta un nuevo par (clave, valor) en el mapa
void insertMap(HashMap * map, char * key, void * value) {
    if (map == NULL || key == NULL) return;

    // Si la clave ya existe, no se inserta nada
    if (searchMap(map, key) != NULL) return;

    // Se calcula la posición inicial
    long pos = hash(key, map->capacity);

    // Se aplica sondeo lineal hasta encontrar una posición vacía
    while (map->buckets[pos] != NULL && map->buckets[pos]->key != NULL) {
        pos = (pos + 1) % map->capacity;
    }

    // Si el espacio está completamente vacío, se crea un nuevo par
    if (map->buckets[pos] == NULL) {
        map->buckets[pos] = createPair(strdup(key), value); // Se duplica la clave para evitar conflictos
    } else {
        // Si ya hay un espacio sin clave (clave borrada), se reutiliza
        map->buckets[pos]->key = strdup(key);
        map->buckets[pos]->value = value;
    }

    map->size++;        // Aumenta el tamaño del mapa
    map->current = pos; // Se actualiza la posición actual
}

// Duplica la capacidad del mapa y reubica todos los elementos
void enlarge(HashMap * map) {
    enlarge_called = 1; // No eliminar: utilizado para pruebas automáticas

    Pair **old_buckets = map->buckets; // Guarda los antiguos buckets
    long old_capacity = map->capacity;

    map->capacity *= 2; // Duplicar capacidad
    map->buckets = (Pair **)calloc(map->capacity, sizeof(Pair *)); // Nuevos buckets vacíos
    map->size = 0; // Reinicia el tamaño

    // Se reinserta cada par en el nuevo mapa
    for (long i = 0; i < old_capacity; i++) {
        if (old_buckets[i] != NULL && old_buckets[i]->key != NULL) {
            insertMap(map, old_buckets[i]->key, old_buckets[i]->value);
            free(old_buckets[i]->key); // Libera la clave anterior duplicada
            free(old_buckets[i]);      // Libera el par
        }
    }

    free(old_buckets); // Libera la antigua tabla
}

// Crea un nuevo mapa hash con una capacidad dada
HashMap * createMap(long capacity) {
    HashMap *map = (HashMap *)malloc(sizeof(HashMap)); // Reserva memoria
    map->buckets = (Pair **)calloc(capacity, sizeof(Pair *)); // Crea los buckets
    map->capacity = capacity;
    map->size = 0;
    map->current = -1; // No hay posición actual aún
    return map;
}

// Elimina una clave del mapa (solo marca como NULL la clave)
void eraseMap(HashMap * map, char * key) {
    if (map == NULL || key == NULL) return;

    Pair *pair = searchMap(map, key); // Busca la clave
    if (pair != NULL) {
        free(pair->key);   // Libera la memoria de la clave
        pair->key = NULL;  // Marca la clave como eliminada
        map->size--;       // Reduce el tamaño
    }
}

// Busca una clave y retorna el par correspondiente
Pair * searchMap(HashMap * map, char * key) {
    if (map == NULL || key == NULL) return NULL;

    long pos = hash(key, map->capacity);
    long start = pos;

    // Búsqueda con sondeo lineal
    while (map->buckets[pos] != NULL) {
        if (map->buckets[pos]->key != NULL && is_equal(map->buckets[pos]->key, key)) {
            map->current = pos; // Actualiza posición actual
            return map->buckets[pos];
        }
        pos = (pos + 1) % map->capacity;
        if (pos == start) break; // Evita bucle infinito
    }

    return NULL; // No se encontró la clave
}

// Retorna el primer par del mapa
Pair * firstMap(HashMap * map) {
    if (map == NULL) return NULL;

    // Itera desde el inicio hasta encontrar una posición ocupada
    for (long i = 0; i < map->capacity; i++) {
        if (map->buckets[i] != NULL && map->buckets[i]->key != NULL) {
            map->current = i;
            return map->buckets[i];
        }
    }

    return NULL;
}

// Retorna el siguiente par del mapa a partir de la posición actual
Pair * nextMap(HashMap * map) {
    if (map == NULL || map->current == -1) return NULL;

    for (long i = map->current + 1; i < map->capacity; i++) {
        if (map->buckets[i] != NULL && map->buckets[i]->key != NULL) {
            map->current = i;
            return map->buckets[i];
        }
    }

    map->current = -1; // No hay más elementos
    return NULL;
}
