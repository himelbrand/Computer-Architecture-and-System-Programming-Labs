#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct virus virus;
 
struct virus {
    unsigned short length;
    char name[16];
    char signature[];
};

typedef struct link link;
 
struct link {
    virus *v;
    link *next;
};

void printHex(unsigned char * buff, size_t len);

void list_print(link *virus_list); 
link* list_append(link* virus_list, virus* data); 
void list_free(link *virus_list);
void detect_virus(char *buffer, link *virus_list, unsigned int size);

int bigEndian;
int firstOnly=0;
int main(int argc, char **argv){
    int i;
    FILE * virus_sig=NULL;
    FILE * suspected=NULL;
    for(i=1;i<argc;i++){
        if(strcmp(argv[i],"-f")==0)
            firstOnly=1;
        else if(virus_sig==NULL)
            virus_sig = fopen(argv[i],"rb");
        else if(suspected==NULL)
            suspected = fopen(argv[i],"rb+");
    }
        
    /*checks size in bytes of virus_sig file*/
    fseek(virus_sig, 0, SEEK_END);
    size_t virusFileSize = ftell(virus_sig);
    fseek(virus_sig, 0, SEEK_SET);
    
    /*buffers init*/
    unsigned char first[1];
    unsigned char number[2];
    unsigned char buffer[10000];
    
    /*read the format type*/
    fread(first,1,1,virus_sig);
    bigEndian = (int)first[0];
   
    

    unsigned short N;
    int offset=1;
    /*init the virus list and temp virus*/
    link *virus_list=NULL;
    virus_list = (link*)malloc(sizeof(link));
    virus_list->v=NULL;
    virus_list->next=NULL;
    virus *newVirus=NULL;


    /*reading viruses file and creating list*/
    while(offset<virusFileSize){
    	offset+=fread(number,1,2,virus_sig);
        if(bigEndian)
            N = (number[0] << 8) | number[1]; 
        else
            N = *(unsigned short *)number;
        newVirus=(virus *)malloc(N);
	newVirus->length=N-18;
	fread(newVirus->name,1,16,virus_sig);
	offset+=16;
	fread(newVirus->signature,1,N-18,virus_sig);
	virus_list = list_append(virus_list,newVirus);
	offset+=(N-18);
    }
    
    /*print the list*/
    list_print(virus_list);
    
    /*checks if there is a suspected file to check*/
    if(suspected != NULL){
        /*checks size in bytes of suspected file*/
        fseek(suspected, 0, SEEK_END);
        size_t suspectedFileSize = ftell(suspected);
        fseek(suspected, 0, SEEK_SET);
        if(suspectedFileSize>10000)
            suspectedFileSize=10000;
        /*reades suspected file to buffer*/
        fread(buffer,1,suspectedFileSize,suspected);
        detect_virus(buffer,virus_list,suspectedFileSize);
        fclose(suspected);
    }
    
    /*free list , tmp virus and close file*/
    list_free(virus_list);
    free(newVirus);
    fclose(virus_sig);
    
    return 0;
}

void list_print(link *virus_list){
	link *current = virus_list;
	while(current != NULL){
		printf("Virus name: %s\n", current->v->name);
		printf("Virus Size: %d\n", current->v->length);
		printf("signature: ");
		printHex(current->v->signature,(size_t)current->v->length);
		current=current->next;
	}

}
void printHex(unsigned char * buff, size_t len){
	int i;
	for(i=0;i<len;++i){
		printf("%02X ", (int)buff[i]);
	}
	printf("\n\n");
}

link* list_append(link* virus_list, virus* data){
	link *current=virus_list;
	if(current->v==NULL){
		virus_list->v=data;
		return virus_list;
	}
	while(current->next!=NULL){
		current=current->next;
	}
	current->next = (link*)malloc(sizeof(link));
        current->next->v=data;
	current->next->next=NULL;
	return virus_list;
}
void list_free(link *virus_list){
	link *prev=NULL;
	while(virus_list->next!=NULL){
		free(virus_list->v);
		prev=virus_list;
		virus_list=virus_list->next;
		free(prev);
	}
	free(virus_list);
}
void detect_virus(char *buffer, link *virus_list, unsigned int size){
	int offset=0;
	link *current=NULL;
	while(offset<size){
		current=virus_list;
		while(current!=NULL){
			if(memcmp(buffer+offset,current->v->signature,current->v->length)==0){
				printf("----VIRUS FOUND----\n");
				printf("Virus starting byte: %d\n", offset);
				printf("Virus name:: %s\n", current->v->name);
				printf("Virus signature size: %d\n", current->v->length);
                                if(firstOnly){/*got -f in command line args*/
                                    offset=size;
                                    break;
                                }
			}
                        current=current->next;
		}
		offset++;
	}
}