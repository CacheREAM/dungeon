#include <math.h>
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>

#define MIN_ROOMS 3
#define MAX_ROOMS 15
#define MAX_ROOM_SIZE 10
#define MIN_ROOM_SIZE 3
#define MAX_MAP_SIZE 100
#define MIN_MAP_SIZE 50
#define TILE_SIZE 31

typedef struct {
  int x, y;
  int width, height;
} Room;

typedef struct {
  int x, y;
  int impassable;
} Tile;

typedef struct {
  int width, height;
  Tile **tiles;
} Map;

typedef struct {
  int x, y;
  int health;
} Character;

Map *create_map(int width, int height) {
  Map *map = malloc(sizeof(Map));
  map->width = width;
  map->height = height;
  map->tiles = malloc(sizeof(Tile *) * height);
  for (int y = 0; y < height; y++) {
    map->tiles[y] = malloc(sizeof(Tile) * width);
    for (int x = 0; x < width; x++) {
      map->tiles[y][x].x = x;
      map->tiles[y][x].y = y;
      map->tiles[y][x].impassable = 1;
    }
  }
  return map;
}

void free_map(Map *map) {
  for (int y = 0; y < map->height; y++) {
    free(map->tiles[y]);
  }
  free(map->tiles);
  free(map);
}

int intersect(Room *room1, Room *room2) {
  if (room1->x + room1->width <= room2->x)
    return 0;
  if (room1->x >= room2->x + room2->width)
    return 0;
  if (room1->y + room1->height <= room2->y)
    return 0;
  if (room1->y >= room2->y + room2->height)
    return 0;
  return 1;
}

void carve_room(Map *map, Room *room) {
  for (int y = room->y; y < room->y + room->height; y++) {
    for (int x = room->x; x < room->x + room->width; x++) {
      map->tiles[y][x].impassable = 0;
    }
  }
}

int is_visible(Map *map, Character *character, int x, int y) {
  int dx = x - character->x;
  int dy = y - character->y;
  int distance = sqrt(dx * dx + dy * dy);

  for (int i = 1; i < distance; i++) {
    int world_x = character->x + dx * i / distance;
    int world_y = character->y + dy * i / distance;
    if (map->tiles[world_y][world_x].impassable) {
      return 0;
    }
  }
  return 1;
}

void print_map(Map *map, Character *character, WINDOW *window) {
  int x_start = character->x - TILE_SIZE / 2;
  int y_start = character->y - TILE_SIZE / 2;
  //  int x_end = x_start + TILE_SIZE; // currently unused...
  //  int y_end = y_start + TILE_SIZE; // currently unused...

  for (int y = 0; y < TILE_SIZE; y++) {
    for (int x = 0; x < TILE_SIZE; x++) {
      int world_x = x_start + x;
      int world_y = y_start + y;
      if (world_x >= 0 && world_x < map->width && world_y >= 0 &&
          world_y < map->height) {
        if (is_visible(map, character, world_x, world_y)) {
          if (map->tiles[world_y][world_x].impassable) {
            mvwprintw(window, y, x, "#");
          } else if (world_x == character->x && world_y == character->y) {
            mvwprintw(window, y, x, "@");
          } else {
            mvwprintw(window, y, x, ".");
          }
        } else {
          mvwprintw(window, y, x, " ");
        }
      } else {
        mvwprintw(window, y, x, " ");
      }
    }
  }
  wrefresh(window);
}

int main() {
  srand(time(NULL));
  initscr();
  noecho();
  nodelay(stdscr, TRUE);
  keypad(stdscr, TRUE);

  int map_width = rand() % (MAX_MAP_SIZE - MIN_MAP_SIZE) + MIN_MAP_SIZE;
  int map_height = rand() % (MAX_MAP_SIZE - MIN_MAP_SIZE) + MIN_MAP_SIZE;
  Map *map = create_map(map_width, map_height);

  Room rooms[MAX_ROOMS];
  int num_rooms = rand() % (MAX_ROOMS - MIN_ROOMS) + MIN_ROOMS;

  for (int i = 0; i < num_rooms; i++) {
    Room room;
    room.x = rand() % (map->width - MAX_ROOM_SIZE - 1) + 1;
    room.y = rand() % (map->height - MAX_ROOM_SIZE - 1) + 1;
    room.width = rand() % (MAX_ROOM_SIZE - MIN_ROOM_SIZE) + MIN_ROOM_SIZE;
    room.height = rand() % (MAX_ROOM_SIZE - MIN_ROOM_SIZE) + MIN_ROOM_SIZE;

    for (int j = 0; j < i; j++) {
      if (intersect(&room, &rooms[j])) {
        i--;
        break;
      }
    }
    if (i < num_rooms) {
      rooms[i] = room;
      carve_room(map, &room);
    }
  }

  Character character;
  character.x = rooms[0].x + rooms[0].width / 2;
  character.y = rooms[0].y + rooms[0].height / 2;
  character.health = 100;

  while (1) {
    print_map(map, &character, stdscr);
    int c = getch();
    if (c == 'q')
      break;
    if (c == 'r') {
      free_map(map);
      map_width = rand() % (MAX_MAP_SIZE - MIN_MAP_SIZE) + MIN_MAP_SIZE;
      map_height = rand() % (MAX_MAP_SIZE - MIN_MAP_SIZE) + MIN_MAP_SIZE;
      map = create_map(map_width, map_height);
      num_rooms = rand() % (MAX_ROOMS - MIN_ROOMS) + MIN_ROOMS;
      for (int i = 0; i < num_rooms; i++) {
        Room room;
        room.x = rand() % (map->width - MAX_ROOM_SIZE - 1) + 1;
        room.y = rand() % (map->height - MAX_ROOM_SIZE - 1) + 1;
        room.width = rand() % (MAX_ROOM_SIZE - MIN_ROOM_SIZE) + MIN_ROOM_SIZE;
        room.height = rand() % (MAX_ROOM_SIZE - MIN_ROOM_SIZE) + MIN_ROOM_SIZE;
        for (int j = 0; j < i; j++) {
          if (intersect(&room, &rooms[j])) {
            i--;
            break;
          }
        }
        if (i < num_rooms) {
          rooms[i] = room;
          carve_room(map, &room);
        }
      }
      character.x = rooms[0].x + rooms[0].width / 2;
      character.y = rooms[0].y + rooms[0].height / 2;
    }
    if (c == 'h' && character.x > 0 &&
        !map->tiles[character.y][character.x - 1].impassable) {
      character.x--;
    }
    if (c == 'j' && character.y < map->height - 1 &&
        !map->tiles[character.y + 1][character.x].impassable) {
      character.y++;
    }
    if (c == 'k' && character.y > 0 &&
        !map->tiles[character.y - 1][character.x].impassable) {
      character.y--;
    }
    if (c == 'l' && character.x < map->width - 1 &&
        !map->tiles[character.y][character.x + 1].impassable) {
      character.x++;
    }
    if (c == 'y' && character.y > 0 && character.x > 0 &&
        !map->tiles[character.y - 1][character.x - 1].impassable) {
      character.y--;
      character.x--;
    }
    if (c == 'n' && character.y < map->height - 1 &&
        character.x < map->width - 1 &&
        !map->tiles[character.y + 1][character.x + 1].impassable) {
      character.y++;
      character.x++;
    }
    if (c == 'b' && character.y < map->height - 1 && character.x > 0 &&
        !map->tiles[character.y + 1][character.x - 1].impassable) {
      character.y++;
      character.x--;
    }
    if (c == 'u' && character.y > 0 && character.x < map->width - 1 &&
        !map->tiles[character.y - 1][character.x + 1].impassable) {
      character.y--;
      character.x++;
    }
  }

  free_map(map);
  endwin();
  return 0;
}
