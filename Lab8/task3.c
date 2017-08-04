#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "elf.h"
#include <errno.h>
int unit_size = 1,debug_flag = 0,Currentfd=-1,file_size=0,program_offset =0,section_offset =0,section_size=0;
char file_name[100],* data_pointer = NULL;

char * map_start = NULL;
char * shstr_table  = NULL;
char * str_table  = NULL;
Elf32_Shdr * section_header = NULL;
FILE * fp = NULL;
struct stat st;

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
void examine_ELF(){

	
	
	printf("%s\n","Please enter <file name>");
	scanf("%100s",file_name);
	
	if(Currentfd != -1){
		munmap(map_start,file_size);
		close(Currentfd);
		Currentfd = -1;
	}

	Currentfd = open(file_name,O_RDWR);
	if(Currentfd<0){
		printf("Error while opening '%s'\n",file_name);
		Currentfd=-1;
		return;
	}else{
		fstat(Currentfd, &st);
		file_size = st.st_size;
		map_start = mmap(0,file_size,PROT_READ,MAP_PRIVATE,Currentfd,0);
		if((int)map_start == -1){
			printf("Error while mapping '%s' to memory %d   %d\n",file_name,errno,file_size);
			close(Currentfd);
			Currentfd=-1;
			return;
		}else{
			if(strncmp((char *)((Elf32_Ehdr *)map_start)->e_ident,"\177ELF",4) == 0){

				printf("Magic: ELF\n");
				if(((Elf32_Ehdr *)map_start)->e_ident[5] == ELFDATA2LSB)	
					printf("Data: 2's complement, little endian\n");
				else
					printf("Data: 2's complement, big endian\n");
				printf("Entry point address: 0x%x\n",((Elf32_Ehdr *)map_start)->e_entry);
				section_offset = ((Elf32_Ehdr *)map_start)->e_shoff;
				printf("Section header table file offset: %d\n",section_offset);
				printf("Number of section header entries: %d\n",((Elf32_Ehdr *)map_start)->e_shnum);
				section_size = ((Elf32_Ehdr *)map_start)->e_shnum;
				printf("Size of section headers: %d\n",section_size);
				printf("Program header table file offset: %d\n",((Elf32_Ehdr *)map_start)->e_phoff);
				printf("Number of program header entries: %d\n",((Elf32_Ehdr *)map_start)->e_phnum);
				printf("Size of program headers: %d\n",((Elf32_Ehdr *)map_start)->e_phentsize);
				shstr_table = (char *)(map_start+((Elf32_Shdr *)(map_start+section_offset+(sizeof(Elf32_Shdr)*((Elf32_Ehdr *)map_start)->e_shstrndx)))->sh_offset);
				section_header = (Elf32_Shdr *)(map_start + section_offset);
    


			}else{
			
				printf("Error '%s' is not an ELF file\n",file_name);
				close(Currentfd);
				Currentfd=-1;
				return;
			}

		}
	}
	if(debug_flag)
		printf("Debug: file name set to '%s'\n",file_name);
}
char * getSectionName(unsigned int i){
	if(i>0)
		return (shstr_table + i);
	else
		return "NULL";
}
void print_section_names(){
	if(Currentfd == -1){
		printf("Error: no open file\n");
	}else{
		Elf32_Shdr * temp = NULL;
		unsigned int i,off=section_offset,next=0;
		char * name = NULL;
		for(i=0;i<section_size;i++){
			temp = (Elf32_Shdr *)(map_start+off+next);
			
			next+=sizeof(Elf32_Shdr);
			name = getSectionName(temp->sh_name);
			printf("[%d]	%s    0x%x 	 %x   %u   %u\n",
				i,name,temp->sh_addr,temp->sh_offset,temp->sh_size,temp->sh_type);
		}
	}
}

void print_symbols(){
	if(Currentfd == -1){
		printf("Error: no open file\n");
	}else{
			Elf32_Shdr * temp = NULL;
			unsigned int j,i,off=section_offset,next=0;
			char * symbol_section_name = NULL;
			Elf32_Sym * sym_table = NULL;
			for(i=0;i<section_size;i++){
			temp = (Elf32_Shdr *)(map_start+off+next);
			next+=sizeof(Elf32_Shdr);
			if(temp->sh_type == SHT_SYMTAB || temp->sh_type == SHT_DYNSYM){
				sym_table = (Elf32_Sym *)(map_start + temp->sh_offset);
				for(j=0;j<temp->sh_size/temp->sh_entsize;j++){

					if(sym_table[j].st_shndx == 0 || sym_table[j].st_shndx == 65521)
						symbol_section_name = "";
					else
						symbol_section_name = shstr_table + section_header[sym_table[j].st_shndx].sh_name;
					printf("[%d]  %08x 	 %-6d  		%-10s 		 %-10s\n",
							j,sym_table[j].st_value,sym_table[j].st_shndx,(char *)(map_start + section_header[temp->sh_link].sh_offset + sym_table[j].st_name),symbol_section_name);
				}
			}
		}
}
}

void link_check(){
	Elf32_Shdr * temp = NULL;
	Elf32_Sym * sym_table = NULL;
	int i,j,next=0;
	for(i=0;i<section_size;i++){
	temp = (Elf32_Shdr *)(map_start+section_offset+next);
	next+=sizeof(Elf32_Shdr);
	if(temp->sh_type == SHT_SYMTAB || temp->sh_type == SHT_DYNSYM){
		sym_table = (Elf32_Sym *)(map_start + temp->sh_offset);
		for(j=0;j<temp->sh_size/temp->sh_entsize;j++){
			if(strcmp((char *)(map_start + section_header[temp->sh_link].sh_offset + sym_table[j].st_name),"_start") == 0){
				printf("_start check: PASSED\n");
				return;
			}
		}
	}
}
printf("_start check: FAILED");
}

void reloc_raw(){
	Elf32_Shdr * section_table = (Elf32_Shdr *)(map_start + section_offset);
	Elf32_Rel * rel_table = NULL;
	int i,j;
	int rel_table_size =0;
	for(i=0;i<section_size;i++){
		if(section_table[i].sh_type == SHT_REL){
			rel_table = (Elf32_Rel *)(map_start + section_table[i].sh_offset);
			rel_table_size = section_table[i].sh_size / section_table[i].sh_entsize;
			printf("offset	 	  info\n");
			for(j=0;j<rel_table_size;j++){
				printf("%-10x	%-10x\n",rel_table[j].r_offset,rel_table[j].r_info);
			}
			printf("\n");
	
		}
	}

	
}

void reloc_sem(){
    
        Elf32_Shdr * section_table = (Elf32_Shdr *)(map_start + section_offset);
        
	int i,j;
	int rel_table_size =0;

        char * dynstr_table  = NULL;
        Elf32_Sym * dynsym_table = NULL;
    
    
	Elf32_Rel * rel_table = NULL;
        
	for(i=0;i<section_size;i++){
		if(section_table[i].sh_type == SHT_REL){
                        
                        dynstr_table = (char *)(map_start + section_table[section_table[section_table[i].sh_link].sh_link].sh_offset);
                        dynsym_table = (Elf32_Sym *)(map_start + section_table[section_table[i].sh_link].sh_offset);
                        
                        printf("%x     \n",section_table[section_table[i].sh_link].sh_offset);
			rel_table = (Elf32_Rel *)(map_start + section_table[i].sh_offset);
			rel_table_size = section_table[i].sh_size / section_table[i].sh_entsize;
			printf("offset         info           Type            sym. name\n");
			for(j=0;j<rel_table_size;j++){
                            
                            
                            printf("%-10x	%-10x  %-10d   %-10s\n",rel_table[j].r_offset, rel_table[j].r_info,ELF32_R_TYPE(rel_table[j].r_info),
                                dynstr_table +  dynsym_table[(unsigned int)ELF32_R_SYM(rel_table[j].r_info)].st_name
                        );
			}
			printf("\n");
	
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
    {"Examine ELF File",examine_ELF},
    {"Print Section Names",print_section_names},
    {"Print Symbols",print_symbols},
    {"Link check",link_check},
    {"Relocation Tables - Raw",reloc_raw},
    {"Relocation Tables - Semantic",reloc_sem},
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