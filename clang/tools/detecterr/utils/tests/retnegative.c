int foo(int x){
  int *y = malloc(sizeof(int));
  *y = 0;
  // should be included
  if (*y == 0){
    return -1;
  }      
  return 0;
}
