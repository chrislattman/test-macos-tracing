static void func1(int arg1, int arg2)
{
    int k = arg2;
    while (arg1 != 100) {
        ++k;
    }
}

int main(void)
{
    func1(1, 4);

    return 0;
}
