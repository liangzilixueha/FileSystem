#define _CRT_SECURE_NO_WARNINGS
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
        char property; //0是文件1是目录
        int size;
        int firstdisk;
        int next;
        int sign; // 1是根目录

    } directitem[7];
    
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
struct trash
{
    struct direct dir;//能回收7个文件或目录
    char nameDir[7][100];//回收的路径名
    char blank[7];//该回收位置是否被占用
};
int totleDiskSize = 1024 * 1024;                //??????��
int diskSize = 1024;                            //????????��
int diskNum = totleDiskSize / diskSize;         //????????
int fatSize = diskNum * sizeof(struct fatitem); //???fat??��
int rootSize = sizeof(struct direct);           //???????��
int rootNum = fatSize / diskSize + 1;           // fatSize / diskSize + 1;?????????????
int maxDirNum = 5;                              //����ļ���Ŀ

//��ʾĿ¼
char path[100];
//Ѱ��Ŀ¼ ���ʮ��
char searchpath[10][100];
int searchpath_num = 0;
//??????????
char *fdisk;
//????
struct direct *root;
struct direct *cur_dir; //?????
//???????fat??
struct fatitem *fat;
//???????
struct opentable u_opentable;
struct direct *Trash=(struct direct*)malloc(sizeof(direct));//����վ 
char nameDir[7][100];
int blank[7];
//struct trash *Trash;

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
int search(char *name);
void search(direct *, char *);
void showTrash();
int recover(char *name);
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
                printf("目录创建成功\n");
            else if (code == -1)
                printf("目录已存在\n");
            else if (code == -2)
                printf("最多只能有5个文件\n");
            else if (code == -3)
                printf("磁盘空间满了\n");
        }
        else if (!strcmp(command, "create"))
        {
            //???????
            scanf("%s", command);
            int code = createFile(command);
            if (code == 0)
                printf("文件创建成功\n");
            else if (code == -1)
                printf("文件已经存在\n");
            else if (code == -2)
                printf("最多只能有5个文件\n");
            else if (code == -3)
                printf("打开的文件最多为5个\n");
            else if (code == -4)
                printf("磁盘空间已经满了\n");
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
                printf("进入目录成功\n");
                break;
            case -1:
                printf("没有该目录\n");
                break;
            case -2:
                printf("只能进入目录\n");
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
                printf("目录删除成功\n");
                break;
            case -1:
                printf("没有该目录\n");
                break;
            case -2:
                printf("删除的不是目录\n");
                break;
            case -3:
                printf("目录路径里有文件\n");
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
                printf("�رճɹ�\n");
                break;
            case -1:
                printf("�Ҳ������ļ�\n");
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
                printf("文件删除成功\n");
                break;
            case -1:
                printf("没有该文件\n");
                break;
            case -2:
                printf("删除的不是文件\n");
                break;
            case -3:
                printf("文件已经打开\n");
                break;
            default:
                break;
            }
        }
        else if (!strcmp(command, "showcur"))
        {
            for (int i = 0; i < maxDirNum + 2; i++)
            {
                printf("name:%s,���ԣ�%c\n",
                       cur_dir->directitem[i].name,
                       cur_dir->directitem[i].property);
            }
        }
        else if (!strcmp(command, "exit"))
        {
            exit();
            printf("退出文件系统\n");
            break;
        }
        else if (!strcmp(command, "search"))
        {
            scanf("%s", command);
            int code = search(command);
        }
        else if(!strcmp(command,"recover"))
        {
        	scanf("%s",command);
        	int code=recover(command);
            switch (code)
            {
            case -3:
                printf("有文件重名\n");
                break;
            case -2:
                printf("该目录下文件已经满了\n");
                break;
            case -1:
                printf("回收站里没有该文件\n");
                break;
            case 0:
                printf("恢复成功\n");
                break;
            default:
                break;
            }
		}
        else if (!strcmp(command, "showTrash"))
        {
            showTrash();
        }
        else
        {
            printf("�������\n");
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
        strcpy(root->directitem[0].name, "Root");
        root->directitem[0].next = root->directitem[0].firstdisk;
        root->directitem[0].property = '1'; //??????

        //??????
        root->directitem[1].sign = 1;
        root->directitem[1].firstdisk = rootNum;
        strcpy(root->directitem[1].name, "..");
        root->directitem[1].next = root->directitem[1].firstdisk;
        root->directitem[1].property = '1'; //??????
        root->directitem[1].size = rootSize;
        
        //初始化回收站
        Trash = (struct direct*)(fdisk + diskSize*(diskNum-1) + fatSize + 1);
        for(int i=0;i<7;i++)
        {
            Trash->directitem[i].sign = 0;
            Trash->directitem[i].firstdisk = rootNum;
            strcpy(Trash->directitem[i].name, "");
            Trash->directitem[i].next = Trash->directitem->firstdisk;
            Trash->directitem[i].property = '1';
            Trash->directitem[i].size = rootSize;
            strcpy(nameDir[i],"");
            blank[i]=0;
        }
              
        
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
    strcpy(temp->directitem[0].name, name);
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
    һ�ѵ���������
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
    // cd "\"
    if (!strcmp("\\", name))
    {
        cur_dir = root;
        strcpy(path, "Root>");
        return 0;
    }

    struct direct *temp = cur_dir;
    int j;
    for (j = 1; j < 7; j++)
    {
        if (!strcmp(temp->directitem[j].name, name))
            break;
    }
    int item = temp->directitem[j].firstdisk;

    if (j >= 7)
    {
        printf("û������ļ�\n");
        return -1;
    }
    //���������ļ�
    if (temp->directitem[j].property == '1')
    {
        temp = (struct direct *)(fdisk + item * diskSize);
        // cd ..
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
                for (int i = 0; i < 100; i++)
                {
                    point[i] = '\0';
                }
                for (int i = 0; i < strlen(path); i++)
                {
                    if (count == 1)
                        break;
                    if (path[i] == '>')
                        --count;
                    point[i] = path[i];
                }
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
    
    //����վ����
    int t;
    for(t=0;t<7;t++)
    {
        if(blank[t]==0)
        {
            blank[t]=1;
            break;
        }
    }
    if(t<7)
    {Trash->directitem[t].sign = cur_dir->directitem[i].sign;
    Trash->directitem[t].firstdisk = cur_dir->directitem[i].firstdisk;
    strcpy(Trash->directitem[t].name, cur_dir->directitem[i].name);
    Trash->directitem[t].next = cur_dir->directitem[i].next;
    Trash->directitem[t].property = cur_dir->directitem[i].property;
    Trash->directitem[t].size = cur_dir->directitem[i].size;
    strcpy(nameDir[t],path);
    strcat(nameDir[t], cur_dir->directitem[i].name);
    }
	
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
    //ֻ�ܴ�5���ļ�
    if (u_opentable.cur_size >= maxDirNum)
    {
        printf("�Ѿ�����5���ˣ�\n");
        return 1;
    }
    int i;
    //�ļ�������
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
        printf("û���ҵ�����ļ�,����open\n");
        return -1;
    }
    //�ǲ���Ŀ¼��
    if (cur_dir->directitem[i].property != '0')
    {
        printf("ֻ��open�ļ�\n");
        return -2;
    }
    //�ļ��Ƿ��
    int j = 0;
    for (j = 0; j < maxDirNum; j++)
    {
        if (!strcmp(u_opentable.openitem[j].name, name))
        {
            printf("�ļ��Ѿ���\n");
            return -3;
        }
    }
    //���Ҵ򿪱�
    for (j = 0; j < maxDirNum; j++)
    {
        if (u_opentable.openitem[j].firstdisk == -1)
            break;
    }
    //д��
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
    //û������ļ��򿪴���
    if (i >= maxDirNum)
        return -1;
    //��ʼ�ͷ�
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
        printf("û���ҵ�����ļ�,����write\n");
        return -1;
    }
    if (cur_dir->directitem[i].property != '0')
    {
        printf("ֻ��write�ļ�\n");
        return -2;
    }
    //****************************************//
    char neirong[100];
PLAESEWRITE:
    printf("������Ҫд������ݣ�\n");
    scanf("%s", neirong);
    //****************************************//
    int flag = open(name);
    if (!strcmp(neirong, "exit"))
    {
        printf("�˳�д��\n");
        return 0;
    }
    //�򿪱����̿��
    int item = u_opentable.openitem[flag].firstdisk;

    // while (fat[item].item != -1)
    // {
    //     item = fat[item].item; /*-���Ҹ��ļ�����һ�̿�--*/
    // }
    char *first = fdisk +
                  item * diskSize +
                  u_opentable.openitem[flag].size % diskSize;
    if (!strcmp(neirong, "del") && u_opentable.openitem[flag].size == 0)
    {
        printf("û�ж�������ɾ����\n");
        goto PLAESEWRITE;
    }
    while (!strcmp(neirong, "del"))
    {
        printf("ɾ����һ���ַ�\n");
        first = first - sizeof(char);
        --u_opentable.openitem[flag].size;
        --cur_dir->directitem[i].size;
        if (u_opentable.openitem[flag].size == 0)
        {
            printf("ɾ�����ˣ����ܼ���ɾ����\n");
            goto PLAESEWRITE;
        }
        scanf("%s", neirong);
    }

    strcpy(first, neirong);
    u_opentable.openitem[flag].size = u_opentable.openitem[flag].size + strlen(neirong);
    cur_dir->directitem[i].size = cur_dir->directitem[i].size + strlen(neirong);
    /*
    û��д������̿鲻�������
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
        printf("û���ҵ�����ļ�,����read\n");
        return -1;
    }
    if (cur_dir->directitem[i].property != '0')
    {
        printf("ֻ��read�ļ�\n");
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
        printf("%c", first[j]);
    }
    printf("\n");
    return 0;
}
int del(char *name)
{
    int i = 0;
    int temp, item;
    //��һ��ɾ����**�ļ�**�Ƿ����
    for (i = 2; i < maxDirNum + 2; i++)
    {
        if (!strcmp(cur_dir->directitem[i].name, name))
            break;
    }
    temp = i;
    if (i >= maxDirNum + 2)
        return -1; //û�ҵ�
    if (cur_dir->directitem[i].property != '0')
        return -2; //����ɾ��Ŀ¼
    for (int j = 0; j < maxDirNum; j++)
    {
        if (!strcmp(u_opentable.openitem[j].name, name))
            return -3; //�Ѿ�������
    }
    item = cur_dir->directitem[temp].firstdisk;
    
    while (item != -1)
    {
        int k = fat[item].item;
        fat[item].item = -1;
        fat[item].em_disk = '0';
        item = k;
    }
    //��ʼ�ͷ�
    int t;
    for(t=0;t<7;t++)
    {
        if(blank[t]==0)
        {
            blank[t]=1;
            printf("1\n");
            break;
        }
    }
    if(t<7)
    {
        Trash->directitem[t].sign = cur_dir->directitem[temp].sign;
        Trash->directitem[t].firstdisk = cur_dir->directitem[temp].firstdisk;
        strcpy(Trash->directitem[t].name, cur_dir->directitem[temp].name);
        Trash->directitem[t].next = cur_dir->directitem[temp].next;
        Trash->directitem[t].property = '0';
        Trash->directitem[t].size = cur_dir->directitem[temp].size;
        strcpy(nameDir[t],path);
        strcat(nameDir[t], cur_dir->directitem[temp].name);
    }
    printf("%s\n", nameDir[t]);
    cur_dir->directitem[temp].sign = 0;
    cur_dir->directitem[temp].firstdisk = -1;
    strcpy(cur_dir->directitem[temp].name, "");
    cur_dir->directitem[temp].next = -1;
    cur_dir->directitem[temp].property = '0';
    cur_dir->directitem[temp].size = 0;
    return 0;
}
int search(char *name)
{
    searchpath_num = 0;
    struct direct *p = root;
    search(p, name);
    if (searchpath_num == 0)
    {
        printf("û���ҵ�����ļ�\n");
        return -1;
    }
    printf("�ҵ���%d���ļ�\n", searchpath_num);
    for (int i = 0; i < searchpath_num; i++)
    {
        printf("%s\n", searchpath[i]);
    }
    return 0;
}
void search(struct direct *p, char *name)
{
    for (int i = 2; i < 7; i++)
    {
        //��Ŀ¼����������ƥ�䣬���ǵü�������ȥ
        if (!strcmp(p->directitem[i].name, name) && p->directitem[i].property == '1')
        {
            struct direct *temp = p;
            strcpy(searchpath[searchpath_num], temp->directitem[i].name);
            strcat(searchpath[searchpath_num], "<");
            while (strcmp(temp->directitem[0].name, "Root"))
            {
                strcat(searchpath[searchpath_num], "<");
                strcat(searchpath[searchpath_num], temp->directitem[0].name);
                temp = (struct direct *)(fdisk + temp->directitem[1].firstdisk * diskSize);
            }
            strcat(searchpath[searchpath_num], "<");
            strcat(searchpath[searchpath_num], "Root");
            ++searchpath_num;
            search((struct direct *)(fdisk + p->directitem[i].firstdisk * diskSize), name);
        }
    }
    for (int i = 2; i < 7; i++)
    {
        //�յĴ��̣�����
        // if (p->directitem[i].name[0] == '.')
        // {
        //     continue;
        // }

        //�ҵ��ˣ��������ļ�
        if (!strcmp(p->directitem[i].name, name) && p->directitem[i].property == '0')
        {
            struct direct *temp = p;
            strcpy(searchpath[searchpath_num], temp->directitem[i].name);
            strcat(searchpath[searchpath_num], "<");
            while (strcmp(temp->directitem[0].name, "Root"))
            {
                strcat(searchpath[searchpath_num], "<");
                strcat(searchpath[searchpath_num], temp->directitem[0].name);
                temp = (struct direct *)(fdisk + temp->directitem[1].firstdisk * diskSize);
            }
            strcat(searchpath[searchpath_num], "<");
            strcat(searchpath[searchpath_num], "Root");
            ++searchpath_num;
        }
        //��Ŀ¼,�������ֲ�ƥ��
        if (p->directitem[i].property == '1' && strcmp(p->directitem[i].name, name))
        {
            search((struct direct *)(fdisk + p->directitem[i].firstdisk * diskSize), name);
        }
    }
}
int recover(char *name)
{
    int i, j,t,item,m,n;
    char str1[100],str2[100];
    char* temp;
    strcpy(str1,name);
    for(m=1;str1[m]!='\0';m++)
    {
        if(str1[m-1]=='>')
        {
            for(n=0;str1[n+m]!='>';n++)
            {
                str2[n]=str1[m];
                if(str1[m]=='\0')
                break;
            }
            cd(str1);
        }   
    }
    for (j = 0; j < 7; j++)
    {
        if (!strcmp(nameDir[j],str1))
            break;
    }
    if (j >= maxDirNum + 2)
        return (-1);//回收站里没有该文件
    for (t = 2; t < maxDirNum+2; t++)
    {
        if (!strcmp(cur_dir->directitem[t].name, Trash->directitem[j].name))
            break;
    }
    if (t < maxDirNum + 2)
        return -3;//有重名文件
    for(i=2;i<maxDirNum+2;i++)
    {
        if(cur_dir->directitem[i].firstdisk==-1)
        break;
    }
    if(i>=maxDirNum)
    return -2;//该目录下文件已满
    
    cur_dir->directitem[i].sign = Trash->directitem[j].sign;
    cur_dir->directitem[i].firstdisk = Trash->directitem[j].firstdisk;
    strcpy(cur_dir->directitem[i].name, Trash->directitem[j].name);
    cur_dir->directitem[i].next = Trash->directitem[j].next;
    cur_dir->directitem[i].property = Trash->directitem[j].property;
    cur_dir->directitem[i].size = Trash->directitem[j].size;
    item=cur_dir->directitem[i].firstdisk;
    fat[item].em_disk='1';

    Trash->directitem[j].sign=0;
    Trash->directitem[j].firstdisk=-1;
    Trash->directitem[j].next=-1;
    strcpy(Trash->directitem[j].name,"");
    strcpy(nameDir[j],"");
    Trash->directitem[j].property='0';
    Trash->directitem[j].size=0;
    blank[j]=0;
    return 0;
	
}
void showTrash()
{
    int i;
    for (i = 0; i < 7; i++)
    {
        if (blank[i]==1)
            printf("%d\t%s\t\n", i, nameDir[i]);
        
    }
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
    if (!fwrite(fdisk, totleDiskSize, 1, fp)) /*��������̿ռ�(�ڴ�)���ݶ�������ļ�disk.dat */
    {
        printf("Error:\nFile write error!\n");
    }
    fclose(fp);

    free(fdisk);
    //free(path);
    return;
}
