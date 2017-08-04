#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int unit_size = 1,debug_flag = 0;
char file_name[100],* data_pointer = NULL;
FILE * fp = NULL;

char * getpass (const char *prompt);

void toggle(){
	char * tryPass =  getpass("Enter debug password:\n");
	int i=0;
	for(i=0;i<strlen(tryPass) && i < 5;i++)
		*(tryPass+i)=*(tryPass+i)^((10*(i+1))+i);
	if(strcmp(tryPass,"OyVBE")==0){
		if(debug_flag == 0){
		debug_flag = 1;
		printf("%s\n","Debug flag now on");
		}else{
			debug_flag = 0;
			printf("%s\n","Debug flag now off");
		}
	}else{
		printf("%s\n","Wrong password sorry...");
		if(debug_flag)
			printf("Debug mode is still on\n");
		else
			printf("Debug mode is still off\n");
	}

	
}
void setfn(){
	printf("%s\n","Please enter <file name>");
	scanf("%100s",file_name);
	if(debug_flag)
		printf("Debug: file name set to '%s'\n",file_name);
}

void setus(){
	printf("%s\n","Please enter <unit size>");
	int old_unit_size = unit_size;
	scanf("%d",&unit_size);
	if(unit_size == 1 || unit_size == 2 || unit_size == 4){
		if(debug_flag)
			printf("Debug: set size to %d\n",unit_size);  
	}else{
		printf("unit size '%d' not valid\n",unit_size);
		unit_size = old_unit_size;
	}
	if(debug_flag)
		printf("Debug: file name set to '%s'\n",file_name);
}
void fdisplay(){
	if(!file_name[0]){
		printf("No file name set\n");
	}else{
		fp = fopen (file_name, "r");
		if(fp){
			int location,length;
			printf("Please enter <location> <length>\n");
			scanf("%x %d",&location,&length);
			if(debug_flag)
                            printf("location : %x length : %d\n",location,length);
			char * buffer = (char *) malloc (unit_size * length);
			fseek(fp,location,SEEK_SET);
			int ans = fread(buffer,unit_size, length, fp);
			if( ans < 0){
				printf("Error while reading from %s\n",file_name);
				fclose(fp);
				fp = NULL;
				free(buffer);
				buffer = NULL;
				return;
			}
			else
				printf("Loaded %d bytes into %p\n",length,data_pointer);
			fclose(fp);
			fp = NULL;
			printf("read unit in hex :\n");
			int i;
                        int j =0;
			for(i=0;i<ans*unit_size;i++){
				if(i%2 == 0 && i>0)
					printf(" ");
                                if( i%unit_size == 0)
                                    j++;
                                if(i==ans*unit_size-1 && i%unit_size==0)
                                    printf("%02x",(unsigned char)*(buffer + i));
                                else
                                    printf("%02x",(unsigned char)*(buffer + unit_size*j - ( (i% unit_size) + 1)));
			}
			printf("\nread unit in dec :\n");
                        j =0;
			for(i=0;i<ans*unit_size;i+=2){
				if(i>0)
					printf(" ");
                                if( i%unit_size == 0)
                                    j++;
                                printf("%u",((int)(unsigned char)*(buffer + unit_size*j - ( (i% unit_size) + 1)))<<8 | 
                                (int)(unsigned char)*(buffer + unit_size*j - ( (i% unit_size) + 1) -1));
			}
			free(buffer);
			buffer = NULL;
		}else{
			printf("Error while opening file\n");
		}
	}
}

void ldmem(){
	if(!file_name[0]){
		printf("No file name set\n");
	}else{
		fp = fopen (file_name, "r");
		if(fp){
			int location,length;
			printf("Please enter <location> <length>\n");
			scanf("%x %d",&location,&length);
			if(debug_flag)
				printf("location : %x length : %d\n",location,length);
			if(data_pointer != NULL){
				free(data_pointer);
				data_pointer = NULL;
			}
			data_pointer = (char *) malloc (length);
			if(debug_flag){
				printf("file name: %s\n",file_name);
				printf("data pointer: %p\n",data_pointer);
				printf("length: %d\n",length);
				printf("location: %d\n",location);        
			}
			fseek(fp,location,SEEK_SET);
			int ans = fread(data_pointer,1, length, fp);
			if( ans < 0)
				printf("Error while reading from %s\n",file_name);
			else
				printf("Loaded %d bytes into %p\n",length,data_pointer);
			fclose(fp);
			fp = NULL;
		}else{
			printf("Error while opening file\n");
		}
	}
}

void sifile(){
	if(!file_name[0]){
		printf("No file name set\n");
	}else{
		fp = fopen (file_name, "r+");
		if(fp){
			char * source_data = NULL;
			int source_address,length,target_location;
			printf("Please enter <source_address> <target_location> <length>\n");
			scanf("%x %x %d",&source_address,&target_location,&length);
			if(debug_flag)
				printf("source_address: %x target_location: %x length: %d\n",source_address,target_location,length);
			source_data = (source_address == 0) ? data_pointer : (void *)(long)source_address;
			fseek(fp, 0, SEEK_END); 
			int file_size = ftell(fp); 
			fseek(fp,0,SEEK_SET);
			if(target_location > file_size){
				printf("Error: target_location exceeds file size\n");
			}else{
				fseek(fp,target_location,SEEK_SET);
				printf("%d %d\n",target_location,length);
				int ans = fwrite(source_data, 1, length, fp);   
				if(ans<0)
					printf("Error while writing to %s\n",file_name);
				else 
					printf("Wrote %d bytes into %s at 0x%x from %p\n",length,file_name,target_location,source_data);
			}
			fclose(fp);
			fp = NULL;
		}else{
			printf("Error while opening file for writing\n");
		}
	}
}
void fm(){
	if(!file_name[0]){
		printf("No file name set\n");
	}else{
		fp = fopen (file_name, "r+");
		if(fp){
			unsigned int location,val;
			printf("Please enter <location> <val>\n");
			scanf("%x %x",&location,&val);
			if(debug_flag)
				printf("location: %x val: %x unit_size:%d\n",location,val,unit_size);
			char source_data[unit_size];
			int i;
			for (i = 0; i < unit_size; ++i){
				unsigned int temp_val = val >> (8*((unit_size - 1) - i));
				source_data[i]=(char)(temp_val);
			}
			fseek(fp, 0, SEEK_END); 
			int file_size = ftell(fp); 
			fseek(fp,0,SEEK_SET);
			if(location > file_size){
				printf("Error: location exceeds file size\n");
			}else{
				fseek(fp,location,SEEK_SET);
				printf("%x\n",location);
				int ans = fwrite(source_data, 1, unit_size, fp);
				if(ans<0)
					printf("Error while writing to %s\n",file_name);
				else 
					printf("Wrote %d bytes into %s at 0x%x\n",unit_size,file_name,location);
			}
			fclose(fp);
			fp = NULL;
		}else{
			printf("Error while opening file for writing\n");
		}
	}
}
void quit(){
	if(debug_flag)
    	printf("quitting\n");
    exit(0);
}
struct fun_desc {
	char *name;
	void (*fun)();
};

struct fun_desc Farray[] = {
    {"Toggle Debug Mode",toggle},
    {"Set File Name",setfn},
    {"Set Unit Size",setus},
    {"File Display",fdisplay},
    {"Load Into Memory",ldmem},
    {"Save Into File",sifile},
    {"File Modify",fm},
    {"Quit",quit},
    {NULL,NULL}
};

int main(int argc, char **argv){
    int i;
    int option;
    while(1){
        if(debug_flag){
          printf("%s %d\n","unit_size:",unit_size);
          printf("%s %s\n","file name:",file_name);
          printf("%s %p\n","data pointer:",data_pointer);
        }
        i=0;
        printf("%s\n","Choose action:");
        while(Farray[i].name != NULL){
         printf("%d-%s\n",i,Farray[i].name);
          i++;
        }
         printf("%s","Option: ");
         scanf("%d",&option);
         if(option >= 0 && option < i){
            Farray[option].fun();
        }
        else{
            printf("%s\n","Not within bounds");
            exit (0);
        }
    printf("\n\n");

    }



}