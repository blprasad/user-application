#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>
#include<errno.h>
#include<time.h>
#include<sys/time.h>
#include <sys/signal.h>
#include<sys/ioctl.h>
#include "friendlist.h"
#include "lcklib.h"

/**
 * The size of a Tox Public Key in bytes.Defined in tox.h in toxcore source files.
 */

#define TOX_PUBLIC_KEY_SIZE            32
#define LCD_CLR			    0x11
#define LCD_LINECLR		    0x12
#define LCD_TEXT		    0x13
#define LCD_BMP			    0x14
#define LCD_BOX			    0x15
#define	LCD_ALPHABETS		0x16
#define LCD_NUMERIC		    0x17
#define LISTUPDOWN      	0x18
#define MENUOPTION          0x19
#define IDENTRY             0x01
#define NAMEENTRY           0x22
#define KEYREAD             0x23
int lcdk_fd ;
extern FriendsList Friends;
struct lcd {
    unsigned char row ;
    unsigned char col ;
    unsigned char font ;
    unsigned char size ;
    unsigned char *buffer ;
}__attribute__((packed));


typedef struct line{
    char contactname[20];
    int linenumer;
}disprowfill;

// a struct to read and write
struct  person
{
    char name[TOXIC_MAX_NAME_LENGTH + 1];
    unsigned char namelength;
    char pub_key[TOX_PUBLIC_KEY_SIZE*2+1];
    //bool datataken;
};

struct lcd lcd_text ;
int totalcontacts;
FILE  *outfile;

unsigned int savenames=0;

int savestatus(int status){
    FILE * fs;
    fs = fopen("/home/prasad/traildat.txt","w");
    savenames=status;
    fprintf(fs,"%d",savenames);
    fclose(fs);
    return 0;
}

int returnstatus(){
    FILE * fs= NULL;
    fs = fopen("/home/prasad/traildat.txt","r");
    while(fscanf(fs,"%d\n",&savenames)!=EOF)
        printf("rows value = %d\n",savenames); // for debug
    fclose(fs);
    return savenames;

}

int lcdk_open(void)
{
    lcdk_fd = open ("/dev/keypad_cdrv",O_RDWR|O_NOCTTY|O_NONBLOCK);
    if (lcdk_fd < 0)
    {
        fprintf (stderr,"Unable to open LCD device\n");
        return -1;
    }
    return 1;
}

void lcdk_close(void)
{
    if (lcdk_fd >0) // lcd_fd is available
        close(lcdk_fd);
}

void lcdk_dispclr(void)
{

    ioctl(lcdk_fd,LCD_CLR,0);

}

int lcdk_getid(char* id){
int ret;
    ret=ioctl(lcdk_fd,IDENTRY,(unsigned char*)id);

    return ret;
    
}

int lcdk_getname(char *name){
    
    ioctl(lcdk_fd,IDENTRY,(unsigned char*)name);

    return 0;
    
}

// displays text to lcd
int lcdk_disptext(unsigned char line_no,unsigned char column,unsigned char *data, unsigned char font)
{
    int ret ;
    if (  line_no > 5 ||  column > 20 || font > 4  )
    {
        fprintf(stderr, "Please Check arguments\n");
        return -1;//ARG_ERR;
    }
    lcd_text.row = line_no ;
    lcd_text.col = column ;
    lcd_text.font = font ;
    lcd_text.buffer = data ;
    lcd_text.size = strlen(data) ;
    printf("from lcdk_disptext buffer= %s\n",lcd_text.buffer);
    ret = ioctl(lcdk_fd,LCD_TEXT,&lcd_text);
    if (ret < 0)
        return -1 ;
    return strlen(data);
}

// lists all contact names of tox frieds on lcd

//returns -2 if none selected and pressed <- button ie its for cancel for now need to change in future implementation,
//returns 1 to 4 if some option in list is selected need to improve the code by storing proper value in slection variable
int list_contacts()
{
    int scrollup=0;
    int totalpages=0;
    int lastpageitems=0;
    int i,j;
    int opt,selected;
    int currentpage=0;
    //disprowfill df;
    totalpages=Friends.num_friends/4;
    lastpageitems=Friends.num_friends%4;
    printf("total pages =%d\n",totalpages);
    //printf("last page items=%d\n",lastpageitems);

    for(i=0;i<totalpages;i++)
    {
        printf("in for %d\n",i);

        lcdk_dispclr();
        if(!scrollup)
            currentpage=i;//i*4;

        for(j=1;j<=4;j++)
        {
            printf("current pag = %d\t i=%d\t j=%d\t",currentpage,i,j);
     //       printf("friend name from listcontacts= %s\n",&Friends.list[currentpage+j].name);
            lcdk_disptext(j,2,&Friends.list[currentpage+j].name,0);
            //sleep(1);

        }
        opt=lcdk_listupdown(4);
        if(opt==-2){
            //none selected
            return -2;
        }
        else if(opt<=4&&opt>0){
            selected=i+j+opt;
            scrollup=0;
            //return selected;
        }
        else if (opt==-1&&currentpage>=1) {
            currentpage=currentpage+opt;
            printf("\t npg=%d\n",currentpage);
            scrollup=scrollup+1;
            //totalpages=scrollup;//here scollup should have an upper limit because it may exceeds original total pages
            if(i>1)
                i=i-1;
        }
       // else if (opt==5) currentpage++; //going to next page when continously pressed down arrow more than 3 times leads to next page entries.

    //sleep(3);
    }
    if(lastpageitems){
        for(j=1;j<=lastpageitems;j++)
        {
            lcdk_disptext(j,2,&Friends.list[i+j].name,0);

        }
        opt=lcdk_listupdown(lastpageitems);
        if(opt==-2){
            //none selected
            return -2;
        }
        else if(opt<=lastpageitems){
            selected=i+j+opt;
        }
        else if (opt==-1&&currentpage>=1) {
            currentpage=currentpage+opt;printf("\n lpg=%d\n",currentpage);
            scrollup=scrollup+1;
            totalpages=scrollup;
            i=i-1;
        }
        if(opt>0 && opt<5)
            return selected;
    }
   // return 0;
}

//scroll up and down in displayed names
int lcdk_listupdown(int nopts)  //select from list of contacts by moving up and down
{
    int retval,select;
    select=nopts;
    retval=0;
    /*for (i = 0; i <=Friends.num_friends; i++) {
        //printf("IN FOR loop list of friends %d\n",i);
        for (ii = 0; ii < TOX_PUBLIC_KEY_SIZE; ii++)
            snprintf(pubkey_buf + ii * 2, 3, "%02X", Friends.list[i].pub_key[ii] & 0xff);
        }*/

    retval=ioctl(lcdk_fd,LISTUPDOWN,&select);
    if(retval){
    printf("in listupdown select=%d\n",select);
    return select;
    }
    else if(retval == 5)
        return 5;
    else
        return -2;
}

int open_contacts(){
    // open file for writing
    outfile =  fopen  ("person.dat", "w");
    if  (outfile == NULL)
    {
        fprintf (stderr, "\nError opend file\n");
        exit  (1);
    }

}

//these calls are nothing to do with toxids and toxfriend names that are originally implemented by tox protocol
//save_contact() and restore_contacts() are only used to maintain a backup copy locally as contacts names are updated every time toxic goes
//online so names are not stored any where locally by toxic client so its our job to save a copy of contacts with some default names so that
//to assign some default name to tox friend,so that some initial name could be shown at contact list until its updated from other end of tox client.

//the following call saves the default name given to each friend at this end,the name may change any time on other end of a fried because he may change
//his nick name any time he wishes.this function saves the friends default name to a file one at each call.
int  save_contact(int nos,char* name,char* toxid)
{
    int k;
    char pubkey_buf[TOX_PUBLIC_KEY_SIZE * 2 + 1];
    struct  person input;   //if i use pointer,malloc here the program is behaving in a different way while print toxid it's only half is printed
    //input.namelength = namelength;
    strcpy(input.name , name);
    input.name[sizeof(name)]='\0';
    //the following line of code is not doing what is needed
    /* memcpy(input.pub_key , toxid, TOX_PUBLIC_KEY_SIZE);
     printf("input.pub.key == %s\n",input.pub_key);*/
    //strcpy(input.pub_key , toxid);
    // input.pub_key[TOX_PUBLIC_KEY_SIZE-1]='\0';
    // write struct to file


    for (k = 0; k < TOX_PUBLIC_KEY_SIZE; k++)
        snprintf(pubkey_buf + k * 2, 3, "%02X", toxid[k] & 0xff);


    pubkey_buf[TOX_PUBLIC_KEY_SIZE * 2] = '\0';
    printf("\n  pubkey buf %s\n",pubkey_buf);

    memcpy(input.pub_key , pubkey_buf, TOX_PUBLIC_KEY_SIZE*2);
    printf("input.pub.key == %s\n",input.pub_key);
    //input.datataken = 0;
    //fwrite(&input,sizeof(struct person),1,outfile);

    if (fwrite(&input,sizeof(struct person),1,outfile)!= 0){
        printf ("contents to file written successfully !\n");
        totalcontacts = totalcontacts+1;
    }
    else
        printf ("error writing file !\n");
    return 0;
}

int close_contacts(){
    // close file
    fclose(outfile);
    return  0;
}

//this function retrieves the friends names from the file ,to which save_contact() function saves it retrieves all names at once,so its called
//at begging of the tox client initial start so as to show all contact names.otherwise the names are blank.
int  restore_contacts(void)
{
    FILE  *infile;
    struct  person input;
    int allnames=0;int namesup=0;
    int i,k,comp;
    char* datafill;
    char pubkey_buf[TOX_PUBLIC_KEY_SIZE * 2 + 1];
    printf("in restore contacs\n");
    // Open person.dat for reading
    infile =  fopen  ("person.dat", "r");
    if  (infile == NULL)
    {
        printf( "\nError opening file\n");
        exit(1);
    }

    allnames = Friends.num_friends;
    printf("allnames =%d\n",allnames);

    if(!totalcontacts)
        totalcontacts = allnames;

    datafill = (char*)malloc(allnames*sizeof(char));
    memset(datafill,0,allnames*sizeof(char));

    //Friends.num_friends
    //Friends.list[i+j].name
    // read file contents till end of file

    //while(allnames>0){
    while ( fread (&input,sizeof(struct person),1,infile))
    {
        //printf("fread while totalcontacts=%d \n",totalcontacts);
        //printf("person.name ********=%s\n",input.name);
        input.pub_key[TOX_PUBLIC_KEY_SIZE * 2] = '\0';
        //printf("input.pub.key == %s\n",input.pub_key);

        for(i=0;i<totalcontacts;i++){ printf("i= %d\n",i);
            //printf("Friend.list.pub.key == %s\n",Friends.list[i].pub_key);

            for (k = 0; k < TOX_PUBLIC_KEY_SIZE; k++)
                snprintf(pubkey_buf + k * 2, 3, "%02X", Friends.list[i].pub_key[k] & 0xff);


            pubkey_buf[TOX_PUBLIC_KEY_SIZE * 2] = '\0';
            //printf("\n  pubkey buf %s\n",pubkey_buf);


            //printf("input.pub.key == %s\n",input.pub_key);

            comp=strcmp(pubkey_buf,input.pub_key);
            //printf("comp = %d\n",comp);

            if((strcmp(pubkey_buf,input.pub_key)==0)&&(datafill[i]==0))
            {
                datafill[i]=1;
                printf("i= %d\n",i);
                printf("yes one match found allnames= %d\n",allnames);
                strcpy(Friends.list[i].name , input.name);
                printf("person.name =%s\n",input.name);
                //input.namelength = namelength;
                allnames = allnames-1;
                namesup = namesup+1;
                //if(namesup>5) break;
              //  sleep(1);
            }
        }
        printf("namesup= %d\t allnames =%d\n",namesup,allnames);
        if(namesup==totalcontacts){ allnames=0;  break;}
    }
    printf("in while\n");printf("namesup= %d\t allnames =%d\n",namesup,allnames);
    //}
    printf("after while\n");
    // close file
    fclose(infile); free(datafill);
    return  0;
}

//reads keypad pressed key stores it in keyret
int keypadread(char* keyret){

    ioctl(lcdk_fd,KEYREAD,(void*)keyret);
    return 0;
}


int display_menu(char* keyret){

    lcdk_disptext(1,0,"Tox client ready",0);
    lcdk_disptext(2,0,"Press any menu key",0);
   // sleep(2);
    ioctl(lcdk_fd,MENUOPTION,(void*)keyret);
    return 0;
}
