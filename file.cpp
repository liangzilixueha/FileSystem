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
        char property; // 0文件 1目录
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

int totleDiskSize = 1024 * 1024;                //??????锟斤拷
int diskSize = 1024;                            //????????锟斤拷
int diskNum = totleDiskSize / diskSize;         //????????
int fatSize = diskNum * sizeof(struct fatitem); //???fat??锟斤拷
int rootSize = sizeof(struct direct);           //???????锟斤拷
int rootNum = fatSize / diskSize + 1;           // fatSize / diskSize + 1;?????????????
int maxDirNum = 5;                              //最大文件数目

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
int fd = -1;

void Startup();
int createDir(char *name);
int createFile(char *name);
void showDir();
int cd(char *name);
int removeDir(char *name);

int open(char *name);
int close(char *name);
int read(char *name);
int write(char *name);
int del(char *name);

void exit();

int main()
{
    char command[10];
    strcpy(path, "Root>");
    Startup();
    while (1)
    {
        printf("%s", path);
        scanf("%s", command);
        if (!strcmp(command, "mkdir"))
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
                printf("???????锟斤拷???\n");
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
        else if (!strcmp(command, "close"))
        {
            scanf("%s", command);
            int code = close(command);
            switch (code)
            {
            case 0:
                printf("关闭成功\n");
                break;
            case -1:
                printf("找不到该文件\n");
                break;
            default:
                break;
            }
        }
        else if (!strcmp(command, "del"))
        {
            scanf("%s", command);
            int code = del(command);
            switch (code)
            {
            case 0:
                printf("删除成功\n");
                break;
            case -1:
                printf("没找到啊\n");
                break;
            case -2:
                printf("不能删除目录啊\n");
                break;
            case -3:
                printf("文件打开中，请先关闭文件\n");
                break;
            default:
                break;
            }
        }
        else if (!strcmp(command, "showcur"))
        {
            for (int i = 2; i < maxDirNum + 2; i++)
            {
                printf("name:%s,属性：%c\n",
                       cur_dir->directitem[i].name,
                       cur_dir->directitem[i].property);
            }
        }
        else if (!strcmp(command, "exit"))
        {
            exit();
            printf("退出程序\n");
            break;
        }
        else
        {
            printf("输入错误\n");
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
        //???????????????锟斤拷
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
            printf("锟斤拷?????\n");
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
    ?锟斤拷??????????????????
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
    //?????锟斤拷
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
    一堆的限制条件
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
    //??????锟斤拷????
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
    //??锟斤拷
    strcpy(cur_dir->directitem[i].name, name);
    cur_dir->directitem[i].firstdisk = j;
    cur_dir->directitem[i].size = 0;
    cur_dir->directitem[i].next = j;
    cur_dir->directitem[i].property = '0';

    //******************//
    //????????锟斤拷??
    // fd = open(name);
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
                printf("<目录>\n");
            }
            else
            {
                printf("<文件>\n");
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
        // cd .. ??锟斤拷????????
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
                //?????锟斤拷?ponint????'\0'
                for (int i = 0; i < 100; i++)
                {
                    point[i] = '\0';
                }
                //??path?锟斤拷????????????point??
                for (int i = 0; i < strlen(path); i++)
                {
                    if (count == 1)
                        break;
                    if (path[i] == '>')
                        --count;
                    point[i] = path[i];
                }
                //??point?锟斤拷????????????path??
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
    printf("open start\n");
    //只能打开5个文件
    if (u_opentable.cur_size >= maxDirNum)
    {
        printf("已经打开了5个了！\n");
        return 1;
    }
    int i;
    //文件存在吗？
    for (i = 2; i < maxDirNum + 2; i++)
    {
        // printf("%s,%s,i:%d,%d\n",
        //        cur_dir->directitem[i].name,
        //        name,
        //        i,
        //        strcmp(cur_dir->directitem[i].name, name));
        if (!strcmp(cur_dir->directitem[i].name, name))
            break;
    }
    if (i >= maxDirNum + 2)
    {
        // printf("%d,%d", i, maxDirNum + 2);
        printf("没有找到这个文件,来自open\n");
        return -1;
    }
    //是不是目录啊
    if (cur_dir->directitem[i].property != '0')
    {
        printf("只能open文件\n");
        return -2;
    }
    //文件是否打开
    int j = 0;
    for (j = 0; j < maxDirNum; j++)
    {
        if (!strcmp(u_opentable.openitem[j].name, name))
        {
            printf("文件已经打开\n");
            return -3;
        }
    }
    //查找打开表
    for (j = 0; j < maxDirNum; j++)
    {
        if (u_opentable.openitem[j].firstdisk == -1)
            break;
    }
    //写表
    u_opentable.openitem[j].firstdisk = cur_dir->directitem[i].firstdisk;
    strcpy(u_opentable.openitem[j].name, name);
    u_opentable.openitem[j].size = cur_dir->directitem[i].size;
    ++u_opentable.cur_size;
    return j;
}
int close(char *name)
{
    int i = 0;
    for (i = 0; i < maxDirNum; i++)
    {
        if (!strcmp(u_opentable.openitem[i].name, name))
            break;
    }
    //没有这个文件打开错了
    if (i >= maxDirNum)
        return -1;
    //开始释放
    strcpy(u_opentable.openitem[i].name, "");
    u_opentable.openitem[i].firstdisk = -1;
    u_opentable.openitem[i].size = 0;
    --u_opentable.cur_size;
    return 0;
}
int write(char *name)
{
    int i;
    for (i = 2; i < maxDirNum + 2; i++)
    {
        if (!strcmp(cur_dir->directitem[i].name, name))
            break;
    }
    if (i >= maxDirNum + 2)
    {
        printf("没有找到这个文件,来自write\n");
        return -1;
    }
    if (cur_dir->directitem[i].property != '0')
    {
        printf("只能write文件\n");
        return -2;
    }
    //****************************************//
    char neirong[100];
    printf("请输入要写入的内容：\n");
    scanf("%s", neirong);
    //****************************************//
    int flag = open(name);
    //打开表的盘块号
    int item = u_opentable.openitem[flag].firstdisk;

    // while (fat[item].item != -1)
    // {
    //     item = fat[item].item; /*-查找该文件的下一盘块--*/
    // }

    char *first = fdisk +
                  item * diskSize +
                  u_opentable.openitem[flag].size % diskSize;
    strcpy(first, neirong);
    u_opentable.openitem[flag].size = u_opentable.openitem[flag].size + strlen(neirong);
    cur_dir->directitem[i].size = cur_dir->directitem[i].size + strlen(neirong);
    /*
    没有写如果磁盘块不够的情况
    */
    close(name);
    return 0;
}
int read(char *name)
{
    int i;
    for (i = 2; i < maxDirNum + 2; i++)
    {
        if (!strcmp(cur_dir->directitem[i].name, name))
            break;
    }
    if (i >= maxDirNum + 2)
    {
        printf("没有找到这个文件,来自read\n");
        return -1;
    }
    if (cur_dir->directitem[i].property != '0')
    {
        printf("只能read文件\n");
        return -2;
    }
    if (cur_dir->directitem[i].size == 0)
    {
        printf("null\n");
        return 0;
    }
    char *first = fdisk + cur_dir->directitem[i].firstdisk * diskSize;

    for (int j = 0; j < cur_dir->directitem[i].size; j++)
    {
        printf("%c\n", first[j]);
    }
    return 0;
}
int del(char *name)
{
    int i = 0;
    int temp, item;
    //找一找删除的**文件**是否存在
    for (i = 2; i < maxDirNum + 2; i++)
    {
        if (!strcmp(cur_dir->directitem[i].name, name))
            break;
    }
    temp = i;
    if (i >= maxDirNum + 2)
        return -1; //没找到
    if (cur_dir->directitem[i].property != '0')
        return -2; //不能删除目录
    for (int j = 0; j < maxDirNum; j++)
    {
        if (!strcmp(u_opentable.openitem[j].name, name))
            return -3; //已经被打开了
    }
    item = cur_dir->directitem[temp].firstdisk;
    while (item != -1)
    {
        int k = fat[item].item;
        fat[item].item = -1;
        fat[item].em_disk = '0';
        item = k;
    }
    //开始释放
    cur_dir->directitem[temp].sign = 0;
    cur_dir->directitem[temp].firstdisk = -1;
    // strcpy(u_opentable.openitem[temp].name, "");
    strcpy(cur_dir->directitem[temp].name, "");
    cur_dir->directitem[temp].next = -1;
    cur_dir->directitem[temp].property = '0';
    cur_dir->directitem[temp].size = 0;
    return 0;
}
void exit()
{
    FILE *fp;
    int i;

    if ((fp = fopen("date", "wb")) == NULL)
    {
        printf("Error:\nCannot open file\n");
        return;
    }
    if (!fwrite(fdisk, totleDiskSize, 1, fp)) /*把虚拟磁盘空间(内存)内容读入磁盘文件disk.dat */
    {
        printf("Error:\nFile write error!\n");
    }
    fclose(fp);

    free(fdisk);
    free(path);
    scanf("%d", &i);
    return;
}