#include "hashMap.h"
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define MIN3(a, b, c) ((a) < (b) ? ((a) < (c) ? (a) : (c)) : ((b) < (c) ? (b) : (c)))

/**
 * Allocates a string for the next word in the file and returns it. This string
 * is null terminated. Returns NULL after reaching the end of the file.
 * @param file
 * @return Allocated string or NULL.
 */
char* nextWord(FILE* file)
{
    int maxLength = 16;
    int length = 0;
    char* word = malloc(sizeof(char) * maxLength);
    for (int i = 0; i < maxLength; i++) {
        word[i] = 0;
    }
    while (1)
    {
        char c = fgetc(file);
        if ((c >= '0' && c <= '9') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            c == '\'')
        {
            if (length + 1 >= maxLength)
            {
                maxLength *= 2;
                word = realloc(word, maxLength);
                for (int i = 0; i < maxLength; i++) {
                    word[i] = 0;
                }
            }
            word[length] = c;
            for (int i = 0; i < strlen(word); i++) {
                word[i] = tolower(word[i]);
            }
            length++;
        }
        else if (length > 0 || c == EOF)
        {
            break;
        }
    }
    if (length == 0)
    {
        free(word);
        return NULL;
    }
    word[length] = '\0';
    return word;
}

/**
 * Loads the contents of the file into the hash map.
 * @param file
 * @param map
 */
void loadDictionary(FILE* file, HashMap* map)
{
    char *word = nextWord(file);
    while (word != NULL) {
        hashMapPut(map, word, 0);
        free(word);
        word = nextWord(file);
    }
   
}

/**
 * Computes the Levenshtein distance between two strings
 * @param
 * @param
 *Source: https:en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance#C
 */
int levenshtein(char *s1, char *s2) {
    unsigned int x, y, s1len, s2len;
    s1len = strlen(s1);
    s2len = strlen(s2);
    unsigned int matrix[s2len+1][s1len+1];
    matrix[0][0] = 0;
    for (x = 1; x <= s2len; x++)
        matrix[x][0] = matrix[x-1][0] + 1;
    for (y = 1; y <= s1len; y++)
        matrix[0][y] = matrix[0][y-1] + 1;
    for (x = 1; x <= s2len; x++)
        for (y = 1; y <= s1len; y++)
            matrix[x][y] = MIN3(matrix[x-1][y] + 1, matrix[x][y-1] + 1, matrix[x-1][y-1] + (s1[y-1] == s2[x-1] ? 0 : 1));
    
    return(matrix[s2len][s1len]);
}

/**
 * Loads the Levenshtein distance for each word in the disctionary
 * @param
 * @param
  */
void loadLevenshteinDist(HashMap *map, char *input) {
    HashLink *curr = NULL;
    for (int i = 0; i < hashMapCapacity(map); i++) {
        curr = map->table[i];
        while (curr != NULL) {
            int levenDist = levenshtein(input, curr->key);
            curr->value = levenDist;
            curr = curr->next;
        }
    }
}

/**
 * Prints the concordance of the given file and performance information. Uses
 * the file input1.txt by default or a file name specified as a command line
 * argument.
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, const char** argv)
{
    // FIXME: implement
    HashMap* map = hashMapNew(1000);
    
    FILE* file = fopen("dictionary.txt", "r");
    clock_t timer = clock();
    loadDictionary(file, map);
    timer = clock() - timer;
    printf("Dictionary loaded in %f seconds\n", (float)timer / (float)CLOCKS_PER_SEC);
    fclose(file);
    
    char inputBuffer[256];
    int quit = 0;
    while (!quit)
    {
        printf("Enter a word or \"quit\" to quit: ");
        scanf("%s", inputBuffer);
        int isValid = 1;
        for (int i = 0; i < strlen(inputBuffer); i++) {
            char currentChar = inputBuffer[i];
            if (((int)currentChar < 65) ||
                ((int)currentChar >= 91 && (int)currentChar < 97) ||
                ((int)currentChar > 122)) {
                isValid = 0;
            }
        }
        
        if (!isValid) {
            printf("Invalid word");
        } else {
            if (strcmp(inputBuffer, "quit") != 0) {
                char inputLowercase[256];
                strcpy(inputLowercase, inputBuffer);
                for (int i = 0; i < strlen(inputLowercase); i++) {
                    inputLowercase[i] = tolower(inputLowercase[i]);
                }
                loadLevenshteinDist(map, inputLowercase);
                
                int contains = hashMapContainsKey(map, inputLowercase);
                if (contains) {
                    printf("The inputted word %s is spelled correctly\n", inputBuffer);
                } else {
                    HashLink *candidates = malloc(sizeof(HashLink)*5);
                    assert(candidates != 0);
                    for (int j = 0; j < 5; j++) {
                        candidates[j].key = 0;
                        candidates[j].value = 0;
                    }
                    
                    HashLink *curr = NULL;
                    for (int i = 0; i < hashMapCapacity(map); i++) {
                        curr = map->table[i];

                        while (curr != NULL) {
                            for (int k = 0; k < 5; k++) {
                                if (candidates[k].key == 0) {
                                    candidates[k].key = curr->key;
                                    candidates[k].value = curr->value;
                                    break;
                                } else {
                                    if (curr->value < candidates[k].value) {
                                        candidates[k].key = curr->key;
                                        candidates[k].value = curr->value;
                                        break;
                                    }
                                }
                            }
                            curr = curr->next;
                        }
                    }
                    printf("The inputted word %s is spelled incorrectly\n", inputBuffer);
                    printf("Did you mean %s, %s, %s, %s, %s?\n", candidates[0].key, candidates[1].key, candidates[2].key, candidates[3].key, candidates[4].key);
                    for(int n = 0; n < 5; n++) {
                        free(candidates);
                        candidates = NULL;
                    }
                }
            }
        }
        
        if (strcmp(inputBuffer, "quit") == 0)
        {
            quit = 1;
        }
    }
    
    hashMapDelete(map);
    return 0;
}
