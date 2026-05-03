CC      = gcc
CFLAGS  = $(shell sdl2-config --cflags) -Wall -Wextra -std=c11 -Iinclude -lm
LIBS    = $(shell sdl2-config --libs) -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm

TARGET  = avatar_game_final

SRC = main.c \
      src/utils.c \
      src/menu_principal.c \
      src/menu_options.c \
      src/menu_sauvegarde.c \
      src/menu_joueur.c \
      src/menu_score.c \
      src/menu_enigme.c \
      src/puzzle.c \
      src/jeu_background.c \
      src/jeu_personage.c \
      src/jeu_ennemi.c \
      src/jeu_entite.c \
      src/jeu_minimap.c

OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LIBS)

main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
