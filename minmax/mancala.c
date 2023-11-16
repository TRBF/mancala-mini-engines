#include <curses.h>
#include <stdlib.h>
#include <stdio.h>

//// TODO: make the values realistic with backtracking or back-propagation or something like that
//// TODO: (maybe?) pruning to optimise and increase prediction ability


struct boardVar{
  int a[6][2], s[2];
  bool player, game_ended;
};

struct positionVar{
  int row, column;
};
int ctr = 0;
void printBoard(struct boardVar board){
  // system("clear");
  printf("Turn: %b\n", board.player);
  printf("  ");
  
  for(int i = 0; i<6; i++)
    printf("%2d ", board.a[i][1]);
  
  printf("\n");
  printf("%2d", board.s[1]);
  printf("  ");

  for(int i = 0; i<5; i++) 
    printf("   ");
  
  printf("%2d", board.s[0]); 
  printf("\n");
  printf("  ");

  for(int i = 0; i<6; i++)
    printf("%2d ", board.a[i][0]);
  
  printf("\n");
}

int max(int num1, int num2)
{
    return (num1 > num2 ) ? num1 : num2;
}

int min(int num1, int num2) 
{
    return (num1 > num2 ) ? num2 : num1;
}

void resetBoard(struct boardVar *board){
  for(int i = 0; i<6; i++)
    board->a[i][0] = board->a[i][1] = 4;
  board->s[0] = board->s[1] = 0;
  board->game_ended = 0;
  board->player = 0;
}

struct boardVar checkEnd(struct boardVar board){
  int empty[2] = {1, 1};
  for(int i = 0; i<6; i++){
    if(board.a[i][0]!=0){
      empty[0] = 0;
    }
    if(board.a[i][1]!=0){
      empty[1] = 0;
    }
  }
  if(empty[0] && empty[1]){
    board.game_ended = 1;
    return board;
  }
  if(!empty[0] && empty[1]){
    for(int i = 0; i<6; i++){
      board.s[0]+=board.a[i][0];
      board.a[i][0] = 0;
    }
    board.game_ended = 1;
  }
  else if(empty[0] && !empty[1]){
    for(int i = 0; i<6; i++){
      board.s[1]+=board.a[i][1];
      board.a[i][1] = 0;
    }
    board.game_ended = 1;
  }
  return board;
}

struct boardVar movePieces(struct boardVar board, struct positionVar position){
  ctr++;
  int mod = 1;
  if(position.row) 
    mod = -1;
  bool player = position.row;
  int pebbles = board.a[position.column][position.row];
  board.a[position.column][position.row] = 0;
  position.column+=mod;
  while(pebbles){
    for(int i = position.column; i<6 && i>=0 && pebbles>0; i+=mod){
      board.a[i][position.row]++;
      pebbles--;
      position.column = i;
    }

    if(pebbles){
      board.s[position.row]++;
      pebbles--;

      if(!pebbles){
        board.player = !board.player;
      }
      if(position.row){
        position.column = 0;
      } else {
        position.column = 5;
      }


      mod=-mod;
      position.row = !position.row;
    }
  }

  int *move_end = &board.a[position.column][position.row];
  int *move_opposite = &board.a[position.column][!position.row];
  if(*move_end == 1 && position.row==player){
    board.s[player] += *move_opposite;
    *move_opposite = 0;
  }

  board.player = !board.player;
  return board;
}

int evaluate(struct boardVar board, int turn, int maxTurn){
  // aka minmax
  int minEval = 48, maxEval = -48, eval = 0;
  board = checkEnd(board);
  if(turn >= maxTurn || board.game_ended){
    eval = board.s[0]-board.s[1]; 
    return eval;
  }
  for(int i = 0; i<6; i++){
    if(board.a[i][board.player]){
      struct positionVar position;
      position.row = board.player;
      position.column = i;
      struct boardVar newBoard = movePieces(board, position);
      int temp = evaluate(newBoard, turn+1, maxTurn);
      maxEval = max(maxEval, temp);
      minEval = min(minEval, temp);
    }
  }
  if(board.player)
    eval = maxEval;
  else
   eval = minEval;
  return eval;
}

struct positionVar engineMove(struct boardVar board, int evalLimit){
  struct positionVar move;
  int moves[6] = {0};
  move.row = board.player;
  for(int i = 0; i<6; i++){
    move.column = i;
    struct boardVar temp = movePieces(board, move);
    moves[i] = evaluate(temp, 0, evalLimit);
  }
  if(board.player){
    move.column = 48;
    for(int i = 0; i<6; i++){
      if(moves[i]<=moves[move.column]){
        move.column = i;
      }
    }
    if(!board.a[move.column][board.player]){
      for(int i = 0; i<6; i++){
        if(board.a[i][board.player]){
          move.column = i;
        }
      }
      for(int i = 0; i<6; i++){
        if(board.a[i][board.player]<board.a[move.column][board.player] && board.a[i][board.player]){
          move.column = i;
        }
      }
    }
  }
  else{
    move.column = -48;
    for(int i = 0; i<6; i++){
      if(moves[i]>=moves[move.column]){
        move.column = i;
      }
    }
    if(!board.a[move.column][board.player]){
      for(int i = 0; i<6; i++){
        if(board.a[i][board.player]){
          move.column = i;
        }
      }
      for(int i = 0; i<6; i++){
        if(board.a[i][board.player]>board.a[move.column][board.player] && board.a[i][board.player]){
          move.column = i;
        }
      }
    }
  }
  for(int i = 0; i<6; i++) printf("move%d: %d\n", i, moves[i]);
  printf("enginemove: %d %d\n", move.column, move.row);
  return move;
}


void playPvP(struct boardVar board){
  struct positionVar position;
  int column = -2;

  board = checkEnd(board);
  if(!board.game_ended){
    printBoard(board);
    
    do{
    printBoard(board);
    scanf("%d", &column);
    column--;
    }
    while(!board.a[column][board.player]);

    position.row = board.player;
    position.column = column;
    
    board = movePieces(board, position);

    playPvP(board);
  } else {
    printf("\nGame ended: %d - %d", board.s[0], board.s[1]);
  }
}

void playPvE(struct boardVar board, bool player, int evalLimit){
  struct positionVar position;
  int column = -2;
  ctr = 0;
  board = checkEnd(board);
  if(!board.game_ended){
    printBoard(board);
    
    if(board.player != player){
      printf("\nThinking...\n");
      board = movePieces(board, engineMove(board, evalLimit));
    }
    else{

      do{
      printBoard(board);
      scanf("%d", &column);
      column--;
      }
      while(!board.a[column][board.player] || column>=6 || column<0);

      position.row = board.player;
      position.column = column;
      
      board = movePieces(board, position);

    }
    playPvE(board, player, evalLimit);
  } else {
    printf("\nGame ended: %d - %d\n", board.s[0], board.s[1]);
    printf("(aka) you kind of ");
    if(board.s[player]>board.s[!player])
       printf("won");
    else
      printf("lost");
    printf("\n");
  }
}

int main(){ 
  struct boardVar newBoardVar; struct positionVar newPositionVar;
  
  newPositionVar.row = 0;
  newPositionVar.column = 1;

  resetBoard(&newBoardVar);

  playPvE(newBoardVar, 0, 9);
  
  return 0;
}

