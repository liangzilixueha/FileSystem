#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct fatitem
{
    int item;
    char em_disk;
};

struct direct
{
    struct FCB
    {
        char name[9];
        char property; // 0是文件 1目录
        int size;
        int firstdisk;
        int next;
        int sign; // 1?????

    } directitem[7];
    // 7??FCB,??????????????????????????????????
    // ???????????????
};

struct opentable
{
    struct openttableitem
    {
        char name[9];
        int firstdisk;
        int size;
    } openitem[5];
    //?????????
    int cur_size;
};

int totleDiskSize = 1024 * 1024;                //??????��
int diskSize = 1024;                            //????????��
int diskNum = totleDiskSize / diskSize;         //????????
int fatSize = diskNum * sizeof(struct fatitem); //???fat??��
int rootSize = sizeof(struct direct);           //???????��
int rootNum = fatSize / diskSize + 1;           // fatSize / diskSize + 1;?????????????
int maxDirNum = 5;                              //??????????5????��???

//?????????
char path[100];
//??????????
char *fdisk;
//????
struct direct *root;
struct direct *cur_dir; //?????
//???????fat??
struct fatitem *fat;
//???????
struct opentable u_opentable;

void Startup();
int createDir(char *name);
int createFile(char *name);
void showDir();
int cd(char *name);
int removeDir(char *name);

int open(char *name);
int read(char *name);
int write(char *name);

int main()
{
    char command[10];
    strcpy(path, "Root>");
    Startup();
    while (1)
    {
        printf("%s", path);
        scanf("%s", command);
        if (!strcmp(command, "exit"))
            break;
        else if (!strcmp(command, "mkdir"))
        {
            //??????
            scanf("%s", command);
            int code = createDir(command);
            if (code == 0)
                printf("?????????\n");
            else if (code == -1)
                printf("???????\n");
            else if (code == -2)
                printf("???5???????????\n");
            else if (code == -3)
                printf("????????\n");
        }
        else if (!strcmp(command, "create"))
        {
            //???????
            scanf("%s", command);
            int code = createFile(command);
            if (code == 0)
                printf("??????????\n");
            else if (code == -1)
                printf("???????\n");
            else if (code == -2)
                printf("???5?????????\n");
            else if (code == -3)
                printf("??????????\n");
            else if (code == -4)
                printf("????????\n");
        }
        else if (!strcmp(command, "dir"))
        {
            showDir();
        }
        else if (!strcmp(command, "cd"))
        {
            scanf("%s", command);
            int code = cd(command);
            switch (code)
            {
            case 0:
                printf("??????????????????\n");
                break;
            case -1:
                printf("????????\n");
                break;
            case -2:
                printf("??????????\n");
                break;
            default:
                break;
            }
        }
        else if (!strcmp(command, "rmdir"))
        {
            scanf("%s", command);
            int code = removeDir(command);
            switch (code)
            {
            case 0:
                printf("??????\n");
                break;
            case -1:
                printf("????????\n");
                break;
            case -2:
                printf("???????????\n");
                break;
            case -3:
                printf("???????��???\n");
                break;

            default:
                break;
            }
        }
        else if (!strcmp(command, "open"))
        {
            scanf("%s", command);
            int code = open(command);
        }
        else if (!strcmp(command, "write"))
        {
            scanf("%s", command);
            int code = write(command);
        }
        else if (!strcmp(command, "read"))
        {
            scanf("%s", command);
            int code = read(command);
        }
    }
    getc(stdin);
}

void Startup()
{
    FILE *fp;
    if ((fp = fopen("date", "rb")) == NULL)
    {
        printf("???????????\n");
        //???????????????��
        fdisk = (char *)malloc(totleDiskSize * sizeof(char));
        fat = (struct fatitem *)(fdisk + diskSize);

        fat[0].item = -1;
        fat[0].em_disk = '1';

        fat[rootNum].item = -1;
        fat[rootNum].em_disk = '1';

        //????????????????
        for (int i = rootNum + 1; i < diskNum; i++)
        {
            fat[i].item = -1;
            fat[i].em_disk = '0';
        }
        //****************//
        root = (struct direct *)(fdisk + diskSize + fatSize);
        //???????

        //??????
        root->directitem[0].sign = 1;
        root->directitem[0].firstdisk = rootNum;
        strcpy(root->directitem[0].name, ".");
        root->directitem[0].next = root->directitem[0].firstdisk;
        root->directitem[0].property = '1'; //??????

        //??????
        root->directitem[1].sign = 1;
        root->directitem[1].firstdisk = rootNum;
        strcpy(root->directitem[1].name, "..");
        root->directitem[1].next = root->directitem[1].firstdisk;
        root->directitem[1].property = '1'; //??????
        root->directitem[1].size = rootSize;

        if ((fp = fopen("date", "wb")) == NULL)
        {
            printf("???????????\n");
            return;
        }
        //???????????????
        for (int i = 2; i < 7; i++)
        {
            root->directitem[i].sign = 0;
            root->directitem[i].firstdisk = -1;
            root->directitem[i].next = -1;
            root->directitem[i].property = '0';
            strcpy(root->directitem[i].name, "null");
            root->directitem[i].size = 0;
        }
        if ((fp = fopen("date", "wb")) == NULL)
        {
            printf("???????????\n");
            return;
        }
        if (fwrite(fdisk, totleDiskSize, 1, fp) != 1)
        {
            printf("��?????\n");
        }

        fclose(fp);
        printf("???????\n");
    }
    else
    {
        printf("????,?????????\n");
        fdisk = (char *)malloc(totleDiskSize * sizeof(char));
        if ((fp = fopen("date", "rb")) == NULL)
        {
            printf("??????????\n");
            return;
        }
        if (!fread(fdisk, totleDiskSize, 1, fp))
        {
            printf("???????\n");
            exit(0);
        }
        fat = (struct fatitem *)(fdisk + diskSize);
        root = (struct direct *)(fdisk + diskSize + fatSize);
        fclose(fp);

        for (int i = 0; i < 5; i++)
        {
            strcpy(u_opentable.openitem[i].name, "");
            u_opentable.openitem[i].firstdisk = -1;
            u_opentable.openitem[i].size = 0;
        }

        printf("??????\n");
    }
    cur_dir = root;
}
int createDir(char *name)
{
    int i, j;
    printf("?????????\n");
    struct direct *temp;
    /*
    ?��??????????????????
    */
    //?????????
    for (i = 2; i < 7; i++)
    {
        if (cur_dir->directitem[i].firstdisk == -1)
            break;
    }
    if (i >= 7)
    {
        printf("??????\n");
        return -2;
    }
    //??????????
    for (j = 2; j < 7; j++)
    {
        if (!strcmp(cur_dir->directitem[j].name, name))
        {
            printf("???????\n");
            return -1;
        }
    }
    //??????????
    for (j = rootNum + 1; j < diskNum; j++)
    {
        if (fat[j].em_disk == '0')
            break;
    }
    if (j >= diskNum)
    {
        printf("????????\n");
        return -3;
    }
    //??????
    fat[j].em_disk = '1';
    //?????��
    strcpy(cur_dir->directitem[i].name, name);
    cur_dir->directitem[i].firstdisk = j;
    cur_dir->directitem[i].size = rootSize;
    cur_dir->directitem[i].next = j;
    cur_dir->directitem[i].property = '1';

    //******************//

    temp = (struct direct *)(fdisk + cur_dir->directitem[i].firstdisk * diskSize);

    temp->directitem[0].sign = 0;
    temp->directitem[0].firstdisk = cur_dir->directitem[i].firstdisk;
    strcpy(temp->directitem[0].name, ".");
    temp->directitem[0].next = temp->directitem[0].firstdisk;
    temp->directitem[0].property = '1';
    temp->directitem[0].size = rootSize;

    temp->directitem[1].sign = cur_dir->directitem[0].sign;
    temp->directitem[1].firstdisk = cur_dir->directitem[0].firstdisk;
    strcpy(temp->directitem[1].name, "..");
    temp->directitem[1].next = temp->directitem[1].firstdisk;
    temp->directitem[1].property = '1';
    temp->directitem[1].size = rootSize;

    for (int i = 2; i < 7; i++)
    {
        temp->directitem[i].sign = 0;
        temp->directitem[i].firstdisk = -1;
        strcpy(temp->directitem[i].name, "");
        temp->directitem[i].next = -1;
        temp->directitem[i].property = '0';
        temp->directitem[i].size = 0;
    }
    return 0;
}
int createFile(char *name)
{
    printf("??????????\n");
    /*
    ?��??????????????????
    */
    int i, j;
    //??????
    for (j = 2; j < 7; j++)
    {
        if (!strcmp(cur_dir->directitem[j].name, name))
        {
            printf("????????\n");
            return -1;
        }
    }
    //?????
    for (i = 2; i < 7; i++)
    {
        if (cur_dir->directitem[i].firstdisk == -1)
            break;
    }
    if (i >= 7)
    {
        printf("???????\n");
        return -2;
    }
    //?????????
    if (u_opentable.cur_size >= 5)
        return -3;
    //??????��????
    for (j = rootNum + 1; j < diskNum; j++)
    {
        if (fat[j].em_disk == '0')
            break;
    }
    if (j >= diskNum)
    {
        printf("????????\n");
        return -4;
    }
    //???????
    fat[j].em_disk = '1';
    //??��
    strcpy(cur_dir->directitem[i].name, name);
    cur_dir->directitem[i].firstdisk = j;
    cur_dir->directitem[i].size = 0;
    cur_dir->directitem[i].next = j;
    cur_dir->directitem[i].property = '0';

    //******************//
    //????????��??
    // fd=open(name);
    return 0;
}
void showDir()
{
    for (int i = 2; i < 7; i++)
    {
        if (cur_dir->directitem[i].firstdisk != -1)
        {
            printf("%s\t", cur_dir->directitem[i].name);
            if (cur_dir->directitem[i].property == '1')
            {
                printf("<??>\n");
            }
            else
            {
                printf("<???>\n");
            }
        }
    }
}
int cd(char *name)
{
    //????????cd \???????Root
    if (!strcmp("\\", name))
    {
        cur_dir = root;
        strcpy(path, "Root>");
        return 0;
    }

    struct direct *temp = cur_dir;
    int j;
    for (j = 0; j < 7; j++)
    {
        if (!strcmp(temp->directitem[j].name, name))
            break;
    }
    int item = temp->directitem[j].firstdisk;

    if (j >= 7)
    {
        printf("????????\n");
        return -1;
    }
    //????????
    if (temp->directitem[j].property == '1')
    {
        temp = (struct direct *)(fdisk + item * diskSize);
        // cd .. ??��????????
        if (!strcmp("..", name))
        {

            if (!strcmp(path, "Root>"))
            {
                printf("??????????\n");
                return 0;
            }
            else
            {
                int count = 0;
                for (int i = 0; i < strlen(path); i++)
                {
                    if (path[i] == '>')
                        count++;
                }
                char point[100];
                //?????��?ponint????'\0'
                for (int i = 0; i < 100; i++)
                {
                    point[i] = '\0';
                }
                //??path?��????????????point??
                for (int i = 0; i < strlen(path); i++)
                {
                    if (count == 1)
                        break;
                    if (path[i] == '>')
                        --count;
                    point[i] = path[i];
                }
                //??point?��????????????path??
                strcpy(path, point);
            }
        }
        else
        {
            strcat(path, name);
            strcat(path, ">");
        }
        cur_dir = temp;
        return 0;
    }
    else
    {
        return -2;
    }
}
int removeDir(char *name)
{
    int i;
    for (i = 2; i < maxDirNum + 2; i++)
    {
        if (!strcmp(cur_dir->directitem[i].name, name))
            break;
    }
    if (i >= maxDirNum + 2)
    {
        printf("????????\n");
        return -1;
    }
    if (cur_dir->directitem[i].property == '0')
    {
        printf("??????????\n");
        return -2;
    }
    //?????,?????????????
    int j;
    struct direct *temp =
        (struct direct *)(fdisk + cur_dir->directitem[i].next * diskSize);
    for (j = 2; j < maxDirNum + 2; j++)
    {
        if (temp->directitem[j].next != -1)
        {
            printf("?????????\n");
            return -3;
        }
    }
    int item = cur_dir->directitem[i].firstdisk;

    fat[item].em_disk = '0';

    cur_dir->directitem[i].sign = 0;
    cur_dir->directitem[i].firstdisk = -1;
    strcpy(cur_dir->directitem[i].name, "");
    cur_dir->directitem[i].next = -1;
    cur_dir->directitem[i].property = '0';
    cur_dir->directitem[i].size = 0;

    return 0;
}

int open(char *name)
{
    int i;
    //????open?????
    for (int i = 2; i < maxDirNum + 2; i++)
    {
        if (!strcmp(cur_dir->directitem[i].name, name))
            break;
    }
    if (i >= maxDirNum + 2)
    {
        printf("????\n");
        return -1;
    }
    //?????????????????
    if (cur_dir->directitem[i].property == '1')
    {
        printf("????????????cd\n");
        return -2;
    }
    //
}
int write(char *name)
{
}
int read(char *name)
{
}