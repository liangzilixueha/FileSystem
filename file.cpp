#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// FAT表
struct fatitem
{
	int item;	  //存放文件的下一个磁盘的指针
	char em_disk; //磁盘块是否空闲
};
//文件控制块
struct direct
{
	struct FCB
	{
		char name[20]; //名字
		char property; // 0是文件1是目录
		int size;	   //大小
		int firstdisk; //起始盘块号
		int next;	   //子目录起始盘块号
		int sign;	   // 1是根目录，0不是根目录

	} directitem[7]; //
};
//用户打开表，记录有关文件的使用的动态信息
struct opentable
{
	struct openttableitem
	{
		char name[9];  //名字
		int firstdisk; //起始盘块号
		int size;	   //文件大小
	} openitem[5];	   //最多五个
	int cur_size;	   //当前打开文件的数目
};
struct trash
{
	struct direct dir;	  //能回收7个文件或目录
	char nameDir[7][100]; //回收的路径名
	char blank[7];		  //该回收位置是否被占用
};
int totleDiskSize = 1024 * 1024;				//整体磁盘块的大小
int diskSize = 1024;							//每个盘块的大小
int diskNum = totleDiskSize / diskSize;			//盘块数量
int fatSize = diskNum * sizeof(struct fatitem); // FAT表大小
int rootSize = sizeof(struct direct);			//根目录大小
int rootNum = fatSize / diskSize + 1;			// fatSize / diskSize + 1;根目录的地址（根目录起始盘块号）
int maxDirNum = 5;								//最大子目录数/最大打开文件数

char path[100];
char searchpath[10][100]; //存放查找的文件路径，【10】是最多10个文件，【100】每个文件的查找路径
int searchpath_num = 0;	  //查到的文件个数
char *fdisk;
//虚拟磁盘的起始地址
struct direct *root;	//根目录
struct direct *cur_dir; //当前目录

struct fatitem *fat; // FAT表

struct opentable u_opentable;									//用户打开表
struct direct *Trash = (struct direct *)malloc(sizeof(direct)); //初始化一个垃圾桶

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
			//创建目录
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
			//创建文件
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
				printf("关闭文件成功\n");
				break;
			case -1:
				printf("关闭失败\n");
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
				printf("name:%s,属性:%c\n",
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
		else if (!strcmp(command, "recover"))
		{
			scanf("%s", command);
			int code = recover(command);
			switch (code)
			{
			case -4:
				printf("父目录已被删除\n");
				break;
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
			printf("输入的命令有误，操作失败。\n");
		}
	}
	getc(stdin);
}

void Startup()
{
	FILE *fp;
	if ((fp = fopen("date", "rb")) == NULL)
	{
		printf("文件不存在，开始创建文件。\n");

		fdisk = (char *)malloc(totleDiskSize * sizeof(char)); //申请空间
		fat = (struct fatitem *)(fdisk + diskSize);			  //计算FAT表的地址
		//初始化FAT表
		fat[0].item = -1;
		fat[0].em_disk = '1';
		//初始化根目录
		fat[rootNum].item = -1;
		fat[rootNum].em_disk = '1';

		for (int i = rootNum + 1; i < diskNum; i++)
		{
			fat[i].item = -1;
			fat[i].em_disk = '0';
		}
		//根目录的地址//
		root = (struct direct *)(fdisk + diskSize + fatSize);
		//初始化目录
		//指向当前的目录
		root->directitem[0].sign = 1;
		root->directitem[0].firstdisk = rootNum;
		strcpy(root->directitem[0].name, "Root");
		root->directitem[0].next = root->directitem[0].firstdisk;
		root->directitem[0].property = '1';

		//指向上一级的目录
		root->directitem[1].sign = 1;
		root->directitem[1].firstdisk = rootNum;
		strcpy(root->directitem[1].name, "..");
		root->directitem[1].next = root->directitem[1].firstdisk;
		root->directitem[1].property = '1';
		root->directitem[1].size = rootSize;

		//初始化回收站
		Trash = (struct direct *)(fdisk + diskSize * (diskNum - 2));
		for (int i = 0; i < 7; i++)
		{
			Trash->directitem[i].sign = 0;
			Trash->directitem[i].firstdisk = rootNum;
			strcpy(Trash->directitem[i].name, "");
			Trash->directitem[i].next = Trash->directitem->firstdisk;
			Trash->directitem[i].property = '1';
			Trash->directitem[i].size = rootSize;
			strcpy(Trash->directitem[i].name, "");
		}

		if ((fp = fopen("date", "wb")) == NULL)
		{
			printf("不能打开文件。\n");
			return;
		}
		//将子目录初始化为空
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
			printf("不能打开文件。\n");
			return;
		}
		if (fwrite(fdisk, totleDiskSize, 1, fp) != 1) //把虚拟磁盘空间保存到磁盘文件中
		{
			printf("shuxie\n");
		}

		fclose(fp);
		printf("初始化成功\n");
	}
	else
	{
		printf("文件已存在，开始读取。\n");
		fdisk = (char *)malloc(totleDiskSize * sizeof(char));
		if ((fp = fopen("date", "rb")) == NULL)
		{
			printf("不能打开文件。\n");
			return;
		}
		if (!fread(fdisk, totleDiskSize, 1, fp))
		{
			printf("读取失败。\n");
			exit(0);
		}
		fat = (struct fatitem *)(fdisk + diskSize);
		root = (struct direct *)(fdisk + diskSize + fatSize);
		Trash = (struct direct *)(fdisk + diskSize * (diskNum - 2));
		fclose(fp);

		for (int i = 0; i < 5; i++)
		{
			strcpy(u_opentable.openitem[i].name, "");
			u_opentable.openitem[i].firstdisk = -1;
			u_opentable.openitem[i].size = 0;
		}

		printf("读取成功。\n");
	}
	cur_dir = root;
}
int createDir(char *name) //创建子目录
{
	int i, j;
	struct direct *temp;

	for (i = 2; i < 7; i++)
	{
		if (cur_dir->directitem[i].firstdisk == -1) //如果找到空闲目录项，就退出
			break;
	}
	if (i >= 7) //如果目录已经满了
	{
		printf("目录已满。\n");
		return -2;
	}
	//判断有没有重名的目录
	for (j = 2; j < 7; j++)
	{
		if (!strcmp(cur_dir->directitem[j].name, name))
		{
			printf("目录重名。\n");
			return -1;
		}
	}
	//查找空闲磁盘块
	for (j = rootNum + 1; j < diskNum; j++)
	{
		if (fat[j].em_disk == '0')
			break;
	}
	if (j >= diskNum) //如果没有空闲磁盘块
	{
		printf("空间已满。\n");
		return -3;
	}
	//将找到的这块空闲磁盘块设置成已分配的状态
	fat[j].em_disk = '1';
	//填写目录项
	strcpy(cur_dir->directitem[i].name, name);
	cur_dir->directitem[i].firstdisk = j;
	cur_dir->directitem[i].size = rootSize;
	cur_dir->directitem[i].next = j;
	cur_dir->directitem[i].property = '1';

	//初始化目录//

	temp = (struct direct *)(fdisk + cur_dir->directitem[i].firstdisk * diskSize); //当前所创建的目录在虚拟磁盘上的地址（内存物理地址）
	//指向当前目录的目录项
	temp->directitem[0].sign = 0;
	temp->directitem[0].firstdisk = cur_dir->directitem[i].firstdisk;
	strcpy(temp->directitem[0].name, name);
	temp->directitem[0].next = temp->directitem[0].firstdisk;
	temp->directitem[0].property = '1';
	temp->directitem[0].size = rootSize;
	//指向上一级目录的目录项
	temp->directitem[1].sign = cur_dir->directitem[0].sign;
	temp->directitem[1].firstdisk = cur_dir->directitem[0].firstdisk;
	strcpy(temp->directitem[1].name, "..");
	temp->directitem[1].next = temp->directitem[1].firstdisk;
	temp->directitem[1].property = '1';
	temp->directitem[1].size = rootSize;
	//子目录都初始化为空
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
int createFile(char *name) //创建文件
{
	int i, j;
	//文件是否重名
	for (j = 2; j < 7; j++)
	{
		if (!strcmp(cur_dir->directitem[j].name, name))
		{
			printf("文件重名。\n");
			return -1;
		}
	}
	//查找第一个空闲子目录
	for (i = 2; i < 7; i++)
	{
		if (cur_dir->directitem[i].firstdisk == -1)
			break;
	}
	if (i >= 7)
	{
		printf("子目录已满。\n");
		return -2;
	}
	//如果用户打开表中打开的文件数大于五（最多只能打开五个）
	if (u_opentable.cur_size >= 5)
		return -3;
	//查找空闲盘块
	for (j = rootNum + 1; j < diskNum; j++)
	{
		if (fat[j].em_disk == '0')
			break;
	}
	if (j >= diskNum)
	{
		printf("空间已满。\n");
		return -4;
	}
	//将找到的这块空闲盘块置为已分配
	fat[j].em_disk = '1';
	//填写文件信息
	strcpy(cur_dir->directitem[i].name, name);
	cur_dir->directitem[i].firstdisk = j;
	cur_dir->directitem[i].size = 0;
	cur_dir->directitem[i].next = j;
	cur_dir->directitem[i].property = '0';

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
		printf("该目录不存在。\n");
		return -1;
	}
	//如果这个名字的属性是目录
	if (temp->directitem[j].property == '1')
	{
		temp = (struct direct *)(fdisk + item * diskSize); //当前目录在磁盘中的位置
		// cd ..
		if (!strcmp("..", name))
		{

			if (!strcmp(path, "Root>")) //路径等于根目录
			{
				printf("已经不能再返回了。\n");
				return 0;
			}
			else
			{
				int count = 0;
				for (int i = 0; i < strlen(path); i++)
				{
					if (path[i] == '>')
						count++; //记录箭头个数
				}
				char point[100];
				for (int i = 0; i < 100; i++)
				{
					point[i] = '\0'; //把数组初始化为空
				}
				for (int i = 0; i < strlen(path); i++) //找到倒数第二个箭头，把最后一个箭头后的东西都删掉
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
		else //进入目录
		{
			strcat(path, name);
			strcat(path, ">");
		}
		cur_dir = temp; //记录当前目录在磁盘中的位置
		return 0;
	}
	else
	{
		return -2; //进不了文件
	}
}
int removeDir(char *name) //删除目录
{
	int i;
	for (i = 2; i < maxDirNum + 2; i++)
	{
		if (!strcmp(cur_dir->directitem[i].name, name))
			break;
	}
	if (i >= maxDirNum + 2) //没有找到输入的目录名
	{
		printf("目录名不存在！\n");
		return -1;
	}
	if (cur_dir->directitem[i].property == '0') //找到的名字的属性是文件
	{
		printf("目录名不存在！\n");
		return -2;
	}

	int j;
	struct direct *temp =
		(struct direct *)(fdisk + cur_dir->directitem[i].next * diskSize);
	for (j = 2; j < maxDirNum + 2; j++)
	{
		if (temp->directitem[j].next != -1) //如果该目录下面还有子目录
		{
			printf("存在子目录，删除目录失败！\n");
			return -3;
		}
	}
	int item = cur_dir->directitem[i].firstdisk; //当前目录的起始盘块号

	int t;
	for (t = 0; t < 7; t++)
	{
		//如果名字是空的话，跳出，为我们所用
		if (!Trash->directitem[t].name[0])
			break;
	}
	if (t < 7)
	{
		Trash->directitem[t].sign = cur_dir->directitem[i].sign;
		Trash->directitem[t].firstdisk = cur_dir->directitem[i].firstdisk;
		strcpy(Trash->directitem[t].name, cur_dir->directitem[i].name);
		Trash->directitem[t].next = cur_dir->directitem[i].next;
		Trash->directitem[t].property = cur_dir->directitem[i].property;
		Trash->directitem[t].size = cur_dir->directitem[i].size;
		strcpy(Trash->directitem[t].name, path);
		strcat(Trash->directitem[t].name, cur_dir->directitem[i].name);
	}

	fat[item].em_disk = '0';
	//删除目录项
	cur_dir->directitem[i].sign = 0;
	cur_dir->directitem[i].firstdisk = -1;
	strcpy(cur_dir->directitem[i].name, "");
	cur_dir->directitem[i].next = -1;
	cur_dir->directitem[i].property = '0';
	cur_dir->directitem[i].size = 0;

	return 0;
}

int open(char *name) //打开文件
{
	printf("open start\n");
	//如果文件已经打开了五个
	if (u_opentable.cur_size >= maxDirNum)
	{
		printf("文件打开数达到限额。\n");
		return 1;
	}
	int i;
	//文件是否存在
	for (i = 2; i < maxDirNum + 2; i++)
	{
		if (!strcmp(cur_dir->directitem[i].name, name))
			break;
	}
	if (i >= maxDirNum + 2)
	{
		printf("文件不存在。\n");
		return -1;
	}
	//如果找到的名字的属性是目录
	if (cur_dir->directitem[i].property != '0')
	{
		printf("文件打开失败。\n");
		return -2;
	}

	int j = 0;
	for (j = 0; j < maxDirNum; j++) //查找文件是否已经被打开
	{
		if (!strcmp(u_opentable.openitem[j].name, name))
		{
			printf("文件已经打开了。\n");
			return -3;
		}
	}
	//查找一个空闲的用户打开表项
	for (j = 0; j < maxDirNum; j++)
	{
		if (u_opentable.openitem[j].firstdisk == -1)
			break;
	}
	//填写表项的相关信息
	u_opentable.openitem[j].firstdisk = cur_dir->directitem[i].firstdisk;
	strcpy(u_opentable.openitem[j].name, name);
	u_opentable.openitem[j].size = cur_dir->directitem[i].size;
	++u_opentable.cur_size;
	return j; //返回用户打开表表项的序号
}
int close(char *name) //关闭文件
{
	int i = 0;
	for (i = 0; i < maxDirNum; i++) //查找文件是否已经被打开
	{
		if (!strcmp(u_opentable.openitem[i].name, name))
			break;
	}
	//文件没有被打开
	if (i >= maxDirNum)
		return -1;
	//清空该文件的用户打开表的表项的内容
	strcpy(u_opentable.openitem[i].name, "");
	u_opentable.openitem[i].firstdisk = -1;
	u_opentable.openitem[i].size = 0;
	--u_opentable.cur_size;
	return 0;
}
int write(char *name) //写文件
{
	int i;
	for (i = 2; i < maxDirNum + 2; i++) //查找该文件是否存在
	{
		if (!strcmp(cur_dir->directitem[i].name, name))
			break;
	}
	if (i >= maxDirNum + 2) //该文件不存在
	{
		printf("文件不存在。\n");
		return -1;
	}
	if (cur_dir->directitem[i].property != '0') //查找到的名字的属性是目录
	{
		printf("文件不存在。\n");
		return -2;
	}

	char neirong[100];
PLAESEWRITE:
	printf("请输入你要写入的内容：\n");
	scanf("%s", neirong);
	//****************************************//
	int flag = open(name); //用户打开表表项的序号
	if (!strcmp(neirong, "exit"))
	{
		printf("退回主界面。\n");
		return 0;
	}

	int item = u_opentable.openitem[flag].firstdisk; //打开用户打开表对应表项的第一个盘块号

	char *first = fdisk +
				  item * diskSize +
				  u_opentable.openitem[flag].size % diskSize;			 //计算出该文件的最末位置
	if (!strcmp(neirong, "del") && u_opentable.openitem[flag].size == 0) //如果文件里没有内容，但是又要删东西
	{
		printf("文件没有内容，删除失败！\n");
		goto PLAESEWRITE;
	}
	while (!strcmp(neirong, "del"))
	{
		printf("删除一个字符。\n");
		first = first - sizeof(char);	   //要写入的指针前移
		--u_opentable.openitem[flag].size; //用户打开表里文件的大小减一
		--cur_dir->directitem[i].size;	   //文件大小减一
		if (u_opentable.openitem[flag].size == 0)
		{
			printf("文件内容已删完。\n");
			goto PLAESEWRITE; //返回上面请输入内容
		}
		scanf("%s", neirong);
	}

	strcpy(first, neirong);																 //把内容加到指针后面
	u_opentable.openitem[flag].size = u_opentable.openitem[flag].size + strlen(neirong); //用户打开表里文件大小扩充
	cur_dir->directitem[i].size = cur_dir->directitem[i].size + strlen(neirong);		 //文件大小扩充

	close(name); //关闭文件
	return 0;
}
int read(char *name) //读文件
{
	int i;
	for (i = 2; i < maxDirNum + 2; i++)
	{
		if (!strcmp(cur_dir->directitem[i].name, name)) //查找文件是否存在在当前目录
			break;
	}
	if (i >= maxDirNum + 2)
	{
		printf("文件不存在。\n");
		return -1;
	}
	if (cur_dir->directitem[i].property != '0') //找到的名字的属性是目录
	{
		printf("文件不存在。\n");
		return -2;
	}
	if (cur_dir->directitem[i].size == 0) //文件大小为0
	{
		printf("null\n");
		return 0;
	}
	char *first = fdisk + cur_dir->directitem[i].firstdisk * diskSize; //计算文件的起始位置
	//读取文件内容
	for (int j = 0; j < cur_dir->directitem[i].size; j++)
	{
		printf("%c", first[j]);
	}
	printf("\n");
	return 0;
}
int del(char *name) //删除文件
{
	int i = 0;
	int temp, item;
	//查找文件是否存在在当前目录
	for (i = 2; i < maxDirNum + 2; i++)
	{
		if (!strcmp(cur_dir->directitem[i].name, name))
			break;
	}
	temp = i; //用来保存目录项的序号
	if (i >= maxDirNum + 2)
		return -1; //文件不存在
	if (cur_dir->directitem[i].property != '0')
		return -2; //这个名字的属性是目录
	for (int j = 0; j < maxDirNum; j++)
	{
		if (!strcmp(u_opentable.openitem[j].name, name))
			return -3; //文件是打开的，就不能删除，退出
	}
	item = cur_dir->directitem[temp].firstdisk; //要删除的文件的起始盘块号

	while (item != -1) //释放空间，将FAT表对应项给修改
	{
		int k = fat[item].item;
		fat[item].item = -1;
		fat[item].em_disk = '0';
		item = k;
	}
	//查找空白的垃圾桶
	int t;
	for (t = 0; t < 7; t++)
	{
		if (!Trash->directitem[t].name[0])
			break;
	}
	if (t < 7)
	{
		Trash->directitem[t].sign = cur_dir->directitem[temp].sign;
		Trash->directitem[t].firstdisk = cur_dir->directitem[temp].firstdisk;
		strcpy(Trash->directitem[t].name, cur_dir->directitem[temp].name);
		Trash->directitem[t].next = cur_dir->directitem[temp].next;
		Trash->directitem[t].property = '0';
		Trash->directitem[t].size = cur_dir->directitem[temp].size;
		strcpy(Trash->directitem[t].name, path);
		strcat(Trash->directitem[t].name, cur_dir->directitem[temp].name);
	}
	printf("删除了:%s\n", Trash->directitem[t].name);
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
	search(p, name);		 //查找
	if (searchpath_num == 0) //如果没有找到文件
	{
		printf("没有找到文件。\n");
		return -1;
	}
	printf("找到了%d个文件\n", searchpath_num);
	for (int i = 0; i < searchpath_num; i++)
	{
		printf("%s\n", searchpath[i]);
	}
	return 0;
}
void search(struct direct *p, char *name) //查找
{										  //查找同名目录
	for (int i = 2; i < 7; i++)			  //遍历当前目录的子目录
	{

		if (!strcmp(p->directitem[i].name, name) && p->directitem[i].property == '1') //如果找到了名字但是对应属性是目录
		{
			struct direct *temp = p;
			strcpy(searchpath[searchpath_num], temp->directitem[i].name);
			strcat(searchpath[searchpath_num], "<");
			while (strcmp(temp->directitem[0].name, "Root")) //是根目录了，不能再回溯了
			{
				strcat(searchpath[searchpath_num], "<");
				strcat(searchpath[searchpath_num], temp->directitem[0].name);
				temp = (struct direct *)(fdisk + temp->directitem[1].firstdisk * diskSize);
			}
			strcat(searchpath[searchpath_num], "<");
			strcat(searchpath[searchpath_num], "Root");
			++searchpath_num;
			search((struct direct *)(fdisk + p->directitem[i].firstdisk * diskSize), name); //进入找到的子目录
		}
	}
	for (int i = 2; i < 7; i++) //查找目录里的文件
	{
		// if (p->directitem[i].name[0] == '.')
		// {
		//     continue;
		// }

		//
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
		//如果找到的目录不是这个名字，进入这个目录再进行递归查找
		if (p->directitem[i].property == '1' && strcmp(p->directitem[i].name, name))
		{
			search((struct direct *)(fdisk + p->directitem[i].firstdisk * diskSize), name);
		}
	}
}
int recover(char *name)
{
	int i, j, t, item, m, n;
	// 1 是我们的恢复的名字，必须是全称，2是临时变量
	char str1[100], str2[100];
	struct direct *temp = root;
	for (int mm = 0; mm < 100; mm++)
	{
		//全部置空
		str1[mm] = str2[mm] = '\0';
	}
	strcpy(str1, name);
	//判断名字对不对
	for (j = 0; j < 7; j++)
	{
		if (!strcmp(Trash->directitem[j].name, str1))
			break;
	}
	if (j >= maxDirNum + 2)
		return (-1); //回收站里没有该文件
	int count = 0;
	for (int mm = 5; str1[mm]; mm++)
	{
		if (str1[mm] != '>')
			str2[count] = str1[mm];
		else
		{
			//进入这个目录
			printf("开始进入%s\n", str2);
			//查找这个东西
			int k = 0;
			for (k = 2; k < maxDirNum + 2; k++)
			{
				if (!strcmp(str2, temp->directitem[k].name))
				{
					temp = (struct direct *)(fdisk + temp->directitem[k].firstdisk * diskSize);
					printf("成功进入%s\n", str2);
					break;
				}
			}
			if (k >= maxDirNum + 2)
				return -4;
			count = 0;
		}
	}
	for (t = 2; t < maxDirNum + 2; t++)
	{
		if (!strcmp(temp->directitem[t].name, str2))
			return -3;
	}
	for (i = 2; i < maxDirNum + 2; i++) //找到空闲的磁盘块
	{
		if (temp->directitem[i].firstdisk == -1)
			break;
	}
	if (i >= maxDirNum)
		return -2; //该目录下文件已满
	//把回收站的东西赋给这个空闲磁盘块
	temp->directitem[i].sign = Trash->directitem[j].sign;
	temp->directitem[i].firstdisk = Trash->directitem[j].firstdisk;
	strcpy(temp->directitem[i].name, str2);
	temp->directitem[i].next = Trash->directitem[j].next;
	temp->directitem[i].property = Trash->directitem[j].property;
	temp->directitem[i].size = Trash->directitem[j].size;
	item = temp->directitem[i].firstdisk;
	fat[item].em_disk = '1';
	//释放回收站
	Trash->directitem[j].sign = 0;
	Trash->directitem[j].firstdisk = -1;
	Trash->directitem[j].next = -1;
	strcpy(Trash->directitem[j].name, "");
	Trash->directitem[j].property = '0';
	Trash->directitem[j].size = 0;
	return 0;
}
void showTrash()
{
	int i;
	for (i = 0; i < 7; i++)
	{
		//如果垃圾桶的名字存在
		if (Trash->directitem[i].name[0] == 'R')
			printf("%d:\t%s\n", i, Trash->directitem[i].name);
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
	if (!fwrite(fdisk, totleDiskSize, 1, fp)) /*把虚拟磁盘空间保存到磁盘文件中*/
	{
		printf("Error:\nFile write error!\n");
	}
	fclose(fp);

	free(fdisk);
	// free(path);
	return;
}
