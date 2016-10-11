#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <errno.h>
#include <assert.h>
#include <sys/time.h>
#include <math.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <ctype.h>
#include <string.h>

using namespace std;

#define DEV_NAME    "/dev/ttyUSB0"        // デバイスファイル名　ここは環境に合わせて変える必要がある
#define BAUD_RATE    B921600              // RS232C通信ボーレート
#define BUFF_SIZE    4096                 // 適当

    int fd; //global
    int len;
    unsigned char buffer[BUFF_SIZE];    // データ受信バッファ
    char *argv[1000];

    double latitude,longitude;
    long *addrido,*addrkeido;
    unsigned char *dummyX,*dummyY,*dummyZ,*dummyido,*dummykeido,*dummyRrate; 


    void set_TKK(){
  
    	// デバイスファイル（シリアルポート）オープン
    	fd = open(DEV_NAME,O_RDWR);
    	if(fd<0){
    		// デバイスの open() に失敗したら
    		perror(argv[1]);
    		exit(1);
  	}
    }

    // シリアルポートの初期化
    void serial_init(int fd)
    {
	struct termios tio;
  	memset(&tio,0,sizeof(tio));
  	tio.c_cflag = CS8 | CLOCAL | CREAD;
  	tio.c_cc[VTIME] = 100;
  	// ボーレートの設定
  	cfsetispeed(&tio,BAUD_RATE);
  	cfsetospeed(&tio,BAUD_RATE);
  	// デバイスに設定を行う
  	tcsetattr(fd,TCSANOW,&tio);
    }

    void gps(){
	int i,t,kouho=0,INS=0,j;
	int rcounter=0;

	double ido2[1000],keido2[1000];
        long ido1,keido1;

	double ido3,keido3,ido4,keido4;
	float pi=3.1415926535897;

	addrido=&ido1;
        addrkeido=&keido1;

	dummyido= (unsigned char *)addrido;
        dummykeido= (unsigned char *)addrkeido;


	// ここで受信待ち
        len=read(fd,buffer,BUFF_SIZE);
        if(len==0){
        // read()が0を返したら、end of file
        // 通常は正常終了するのだが今回は無限ループ
 //       continue;
        }
        if(len<0){
          printf("%s: ERROR\n",argv[0]);
          // read()が負を返したら何らかのI/Oエラー
          perror("");
          exit(2);
        }
        // read()が正を返したら受信データ数

    // 受信したデータを 16進数形式で表示    
    for(i=0; i<len; i++){
      unsigned char data=buffer[i];
  //    printf("%02X ",data);
    
      //受信データは意味の塊ごとに先頭に16進数で　16 16 06 02　と言うデータがつくことになっているので
      //以下でそのデータを受信するごとに改行して表示するようになっている．
      switch(rcounter){
      case 0:
        if (data==0x16)rcounter++;
        else rcounter=0;
        break;
      case 1:
        if (data==0x16)rcounter++;
        else rcounter=0;
        break;
      case 2:
        if (data==0x06)rcounter++;
        else rcounter=0;
        break;
      case 3:
        if (data==0x02){

//	  printf("\n");

	  if(buffer[i+7]==0x40){
		kouho=i+1;
	  }
	  else if(buffer[i+7]==0x20){
		INS=i+1;
	  }

        }
        rcounter=0;
        break;
      }
     }

	if(buffer[kouho+6]==0x40 && buffer[INS+7+38]==0x04){ 
		for(i=0;i<=7;i++){  		//緯度,経度を抽出

		*dummyido=buffer[kouho+i+7+41];
		*dummykeido=buffer[kouho+i+7+49];

		*dummyido++;
		*dummykeido++;
	
		}


	//	if(ut>3.0 && ido1!=0 && keido1!=0 && buffer[INS+7+38]==0x04){		//3秒経過

			ido3= (double)ido1;		  //long→double
			keido3= (double)keido1;

			ido3=ido3*(180/pi);		  //rad→度
			keido3=keido3*(180/pi);	

			ido3=ido3/(10*10*10*10*10*10*10*10*10);
			ido3=ido3/10;

			keido3=keido3/(10*10*10*10*10*10*10*10*10);
			keido3=keido3/10;	

			ido2[j]=ido3;
			keido2[j]=keido3;

			char ido[255];		//緯度データをtxt化
			sprintf(ido,"/home/itolab/work/toma/TKK/test/gps/latitude_goal.txt");
			char dido[255];
			ofstream GPSido(ido);

			for(i=0;i<=j;i++){			
				sprintf(dido,"%lf ",ido2[i]);
				GPSido<<dido<<endl;
			}
			GPSido.close();

			char keido[255];		//経度データをtxt化
			sprintf(keido,"/home/itolab/work/toma/TKK/test/gps/longitude_goal.txt");
			char dkeido[255];
			ofstream GPSkeido(keido);

			for(i=0;i<=j;i++){			
				sprintf(dkeido,"%lf ",keido2[i]);
				GPSkeido<<dkeido<<endl;
			}
			GPSkeido.close();

			j=j+1;


	//	}

	}

    }



