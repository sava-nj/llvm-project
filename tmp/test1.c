void foo() {
    int X = 21;
    int Y = 22;
    {
      int Z = 23;
      Z = X;
    }
    X = Y;
}
