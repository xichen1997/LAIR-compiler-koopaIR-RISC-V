int buf[2][100];


void merge_sort(int l, int r)
{
    if (l + 1 >= r)
        return;
}

int main()
{
    int n = getarray(buf[0]);
    merge_sort(0, n);
    putarray(n, buf[0]);
    return 0;
}