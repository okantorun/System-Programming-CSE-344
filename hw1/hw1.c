#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int my_strcmp(char *str1, char *str2){

	int control = 1;
	int i = 0;
	while(*(str1 + i) != '\0' || *(str2 + i) != '\0'){
		if(*(str1 + i) != *(str2 + i)){
			control = 0;
			break;
		}
		i++;
	}

	if(control == 0)
		return 0;
	else
		return 1;
}

int my_strlen(char *str1){
	int length = 0;
	while(*str1 != '\0'){
		length++;
		str1++;
	}
	return length;

}
void Replace_Two_String(char *path,int fd,char *buffer ,char *str1, char *str2,int flag,int caseSens,struct flock lock){
	
	int str2Size = my_strlen(str2);
	int tempSize=0,bufferSize=0;
	char lastChar = '\n';

	char *temp = malloc(1 * sizeof(char));
	char *tempBuffer = malloc(1 * sizeof(char));
	buffer = malloc(1 * sizeof(char));


	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
	while(read(fd,tempBuffer,1) != '\0'){
		if(*tempBuffer != '\n' && *tempBuffer != ' '){
			tempSize++;
			temp = (char*)realloc(temp,tempSize*sizeof(char));
			temp[tempSize-1] = *tempBuffer;
		}
		else{
			if(my_strcmp(str1,temp) && flag == 3){
				bufferSize+=str2Size+1;
				buffer = (char*)realloc(buffer,bufferSize*sizeof(char));
				for(int i=0;i<str2Size;i++){
					buffer[bufferSize-i-2] =  str2[str2Size-i-1];
				}
			}
			else if(my_strcmp(str1,temp) && *tempBuffer == '\n' && flag == 2){
				bufferSize+=str2Size+1;
				buffer = (char*)realloc(buffer,bufferSize*sizeof(char));
				for(int i=0;i<str2Size;i++){
					buffer[bufferSize-i-2] =  str2[str2Size-i-1];
				}

			}
			else if(my_strcmp(str1,temp) && lastChar == '\n' && flag == 1){
				bufferSize+=str2Size+1;
				buffer = (char*)realloc(buffer,bufferSize*sizeof(char));
				for(int i=0;i<str2Size;i++){
					buffer[bufferSize-i-2] =  str2[str2Size-i-1];
				}

			}
			else if(!strncasecmp(str1, temp, tempSize) && caseSens && flag!=1 && flag!=2){
				bufferSize+=str2Size+1;
				buffer = (char*)realloc(buffer,bufferSize*sizeof(char));
				for(int i=0;i<str2Size;i++){
					buffer[bufferSize-i-2] =  str2[str2Size-i-1];
				}
			}
			else{
				bufferSize+=tempSize+1;
				buffer = (char*)realloc(buffer,bufferSize*sizeof(char));
				for(int i=0;i<tempSize;i++){
					buffer[bufferSize-i-2] =  temp[tempSize-i-1];
				}
			}
			lastChar = *tempBuffer;
			buffer[bufferSize-1] = *tempBuffer;
			free(temp);
			tempSize=0;
			temp = malloc(1 * sizeof(char));

		}
    }
     lock.l_type = F_UNLCK;
 	fcntl (fd, F_SETLKW, &lock);
 	close(fd);

	fd = open(path, O_RDWR | O_EXCL | O_TRUNC, mode);
	write(fd,buffer,bufferSize);

	 lock.l_type = F_UNLCK;
 	fcntl (fd, F_SETLKW, &lock);
	close(fd);
}

void Crate_Options(char **str1Options,char *options,char *str1,int start){
	for(int i = 0; i < my_strlen(options) ; i++){
		if(start>0){
			for(int j=0;j<start;j++){
				str1Options[i][j] = str1[j];
			}
			str1Options[i][start] = options[i];
			for(int j=0; str1[start+j] != '\0'; j++){
				str1Options[i][start+j+1] = str1[start+j];
			}
		}
		else{
			str1Options[i][start] = options[i];
			for(int j=0; str1[j]!='\0';j++){
				str1Options[i][j+1] = str1[j];
			}
		}
	}

}
int Script_Control(char *script){
	int scrLength=my_strlen(script);
	if(script[0] != '/' || (script[scrLength-1] != '/' && script[scrLength-1] != 'i'))
		return 0;
	else
		return 1;
}


void Repetitions_Character(char *path,int fd,char *buffer , int indexRpt, char *str1, char *str2,int flag,int caseSens,struct flock lock){

	int str2Size = my_strlen(str2);
	int tempSize=0,bufferSize=0,control = 1;
	int i = 0;
	char lastChar = '\n';
	char *temp = malloc(1 * sizeof(char));
	char *tempBuffer = malloc(1 * sizeof(char));
	buffer = malloc(1 * sizeof(char));
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;

	while(read(fd,tempBuffer,1) != '\0'){
		if(*tempBuffer != '\n' && *tempBuffer != ' '){
			tempSize++;
			temp = (char*)realloc(temp,tempSize*sizeof(char));
			temp[tempSize-1] = *tempBuffer;
		}
		else{
			for(i=0; i<indexRpt;i++){
				if(str1[i] != temp[i])
					{control = 0;break;}
			}
			if(control && str1[indexRpt] == temp[indexRpt]){
				int iter = 0;
				i=0;
				for(iter=indexRpt;iter<my_strlen(temp);iter++){
					
					if(temp[iter] != str1[indexRpt]){
						break;
					}
				}
			
				while(str1[indexRpt + i + 1] != '\0' || temp[iter + i] != '\0'){
					if(str1[indexRpt + i + 1] != temp[iter + i]){
						control = 0;
						break;
					}
					i++;
				}
			}
			else if(control){
				while(str1[indexRpt + i + 1] != '\0' || temp[indexRpt + i] != '\0'){
					if(str1[indexRpt + i + 1] != temp[indexRpt + i]){
						control = 0;
						break;
					}
					i++;
				}
			}
			if(control && flag!=1 && flag!=2){
				
				bufferSize+=str2Size+1;
				buffer = (char*)realloc(buffer,bufferSize*sizeof(char));
				for(int i=0;i<str2Size;i++){
					buffer[bufferSize-i-2] =  str2[str2Size-i-1];
				}
					
			}
			else if(control && flag==1 && lastChar == '\n'){
				bufferSize+=str2Size+1;
				buffer = (char*)realloc(buffer,bufferSize*sizeof(char));
				for(int i=0;i<str2Size;i++){
					buffer[bufferSize-i-2] =  str2[str2Size-i-1];
				}

			}
			else if(control && flag==2 && *tempBuffer == '\n'){
				bufferSize+=str2Size+1;
				buffer = (char*)realloc(buffer,bufferSize*sizeof(char));
				for(int i=0;i<str2Size;i++){
					buffer[bufferSize-i-2] =  str2[str2Size-i-1];
				}
			}
			else if(!strncasecmp(str1, temp, tempSize) && caseSens){
				bufferSize+=str2Size+1;
				buffer = (char*)realloc(buffer,bufferSize*sizeof(char));
				for(int i=0;i<str2Size;i++){
					buffer[bufferSize-i-2] =  str2[str2Size-i-1];
				}
			}
			else{
				bufferSize+=tempSize+1;
				buffer = (char*)realloc(buffer,bufferSize*sizeof(char));
				for(int i=0;i<tempSize;i++){
					buffer[bufferSize-i-2] =  temp[tempSize-i-1];
				}
			}
			lastChar = *tempBuffer;
			buffer[bufferSize-1] = *tempBuffer;
			free(temp);
			tempSize=0;
			temp = malloc(1 * sizeof(char));
			control=1;

		}
	}
	 lock.l_type = F_UNLCK;
 	fcntl (fd, F_SETLKW, &lock);
	close(fd);
	fd = open(path, O_RDWR | O_EXCL | O_TRUNC, mode);
	write(fd,buffer,bufferSize);
	 lock.l_type = F_UNLCK;
 	fcntl (fd, F_SETLKW, &lock);
	close(fd);
	free(temp);
	free(tempBuffer);
	free(buffer);
	
}
void Repetitions_Character2(char *path,int fd,char *buffer , int indexRpt, char **str1, char *str2, int optionsSize,int flag,int caseSens,struct flock lock){

	int str2Size = my_strlen(str2);
	int tempSize=0,bufferSize=0,control = 1;
	int i = 0,j = 0;

	char lastChar = '\n';
	char *temp = malloc(1 * sizeof(char));
	char *tempBuffer = malloc(1 * sizeof(char));
	buffer = malloc(1 * sizeof(char));
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;

	while(read(fd,tempBuffer,1) != '\0'){
		if(*tempBuffer != '\n' && *tempBuffer != ' '){
			tempSize++;
			temp = (char*)realloc(temp,tempSize*sizeof(char));
			temp[tempSize-1] = *tempBuffer;
		}
		else{
			for(j=0;j<optionsSize;j++){
				for(i=0; i<indexRpt;i++){
					if(str1[j][i] != temp[i])
						{control = 0;break;}
				}
				if(control && str1[j][indexRpt] == temp[indexRpt]){
					int iter = 0;
					i=0;
					for(iter=indexRpt;iter<my_strlen(temp);iter++){
						if(temp[iter] != str1[j][indexRpt]){
							break;
						}
					}
				
					while(str1[j][indexRpt + i + 1] != '\0' || temp[iter + i] != '\0'){
						if(str1[j][indexRpt + i + 1] != temp[iter + i]){
							control = 0;
							break;
						}
						i++;
					}
				}
				else if(control){
					while(str1[j][indexRpt + i + 1] != '\0' || temp[indexRpt + i] != '\0'){
						if(str1[j][indexRpt + i + 1] != temp[indexRpt + i]){
							control = 0;
							break;
						}
						i++;
					}
				}
				if(control && flag!=1 && flag!=2){
					bufferSize+=str2Size+1;
					buffer = (char*)realloc(buffer,bufferSize*sizeof(char));
					for(int i=0;i<str2Size;i++){
						buffer[bufferSize-i-2] =  str2[str2Size-i-1];
					}
					break;
						
				}
				else if(control && flag==1 && lastChar == '\n'){
					bufferSize+=str2Size+1;
					buffer = (char*)realloc(buffer,bufferSize*sizeof(char));
					for(int i=0;i<str2Size;i++){
						buffer[bufferSize-i-2] =  str2[str2Size-i-1];
					}
					break;

				}
				else if(control && flag==2 && *tempBuffer == '\n'){
					bufferSize+=str2Size+1;
					buffer = (char*)realloc(buffer,bufferSize*sizeof(char));
					for(int i=0;i<str2Size;i++){
						buffer[bufferSize-i-2] =  str2[str2Size-i-1];
					}
					break;

				}
				else if(!strncasecmp(str1[j], temp, tempSize) && caseSens){
					bufferSize+=str2Size+1;
					buffer = (char*)realloc(buffer,bufferSize*sizeof(char));
					for(int i=0;i<str2Size;i++){
						buffer[bufferSize-i-2] =  str2[str2Size-i-1];
					}
					break;
				}
				else if(j==optionsSize-1 || control){
					bufferSize+=tempSize+1;
					buffer = (char*)realloc(buffer,bufferSize*sizeof(char));
					for(int i=0;i<tempSize;i++){
						buffer[bufferSize-i-2] =  temp[tempSize-i-1];
					}
					break;
				}
				if(control)
					break;
				control=1;
			}
			lastChar = *tempBuffer;
			buffer[bufferSize-1] = *tempBuffer;
			free(temp);
			tempSize=0;
			temp = malloc(1 * sizeof(char));
			control=1;

		}
	}

	close(fd);
	fd = open(path, O_RDWR | O_EXCL | O_TRUNC, mode);
	write(fd,buffer,bufferSize);
	close(fd);
	free(temp);
	free(tempBuffer);
	free(buffer);
}

int main(int argc, char *argv[])
{

	char *path = argv[2];
	int control=0;
	int i=1;
	int str1Size=0;
	int str2Size=0;
	int start;
	char *options = malloc(1 * sizeof(char));
	int optionsSize=0;
	int flag1=0,flag2=0,flag3=0,flag4=0;
	char **strOptions;
	int indexRpt=0;
	int str1SizeReel=0;
	int caseSens=0;
	char *buffer;
	char *str1;
	char *str2;

	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;

	int fd = open(path, O_RDWR | O_EXCL, mode);

	if(argc!=3){
		perror("Number of Arguments");
		return 1;
	}
	if(fd == -1){
		perror("open");
		return 1;
	}
	if(!Script_Control(argv[1])){
		perror("String script expression is incorrect");
		return 1;
	}
    
     
 	struct flock lock;
	 printf ("opening %s\n", path);
	 /* Open a file descriptor to the file. */
	 printf ("locking\n");
	 /* Initialize the flock structure. */
	 memset (&lock, 0, sizeof(lock));
	 lock.l_type = F_WRLCK;
	 /* Place a write lock on the file. */
	 fcntl (fd, F_SETLKW, &lock);
 	fd = open(path, O_RDWR | O_EXCL, mode);
 	control=0;
 	buffer = malloc(1 * sizeof(char));
 	str1 = malloc(1 * sizeof(char));
 	str2 = malloc(1 * sizeof(char));
 	options = malloc(1 * sizeof(char));
     while(argv[1][i] != '/'){
     	caseSens=0;
     	if(argv[1][i] == '['){
     		flag4=1;
     		start=i-1-control;
     		i++;
     		while(argv[1][i] != ']'){
     			options = (char*)realloc(options,(optionsSize+1)*sizeof(char));
				options[optionsSize]=argv[1][i];
				optionsSize++;
     			i++;
     		}
     		i++;
     		str1SizeReel++;
     	}
     	if(argv[1][i] == '*' || argv[1][i] == '^' || argv[1][i] == '$'){
     		if(argv[1][i] == '^')
     			flag1=1;
     		if(argv[1][i] == '$')
     			flag2=1;
     		if(argv[1][i] == '*'){
     			flag3=1;
     			indexRpt=str1SizeReel-1;
     		}
     		control++;
     		i++;
     	}
     	else{
     		str1= (char*)realloc(str1,(str1Size+1)*sizeof(char));
	     	str1[str1Size] = argv[1][i];
	     	str1Size++;
	     	str1SizeReel++;
	     	i++;

     	}
     }
     str1[str1Size] = '\0';
     
     i++;
	  while(argv[1][i] != '/'){
	  	str2= (char*)realloc(str2,(str2Size+1)*sizeof(char));
	 	str2[str2Size] = argv[1][i];
	 	str2Size++;
	 	i++;
	  }
	  str2[str2Size] = '\0';
	  i++;
	  if(argv[1][i] == 'i'){
	  	caseSens=1;
	  	i++;
	  }
     if(flag4){
     	strOptions = (char**)malloc((my_strlen(str1)+1) * sizeof(char*));
     	for(int j=0;j<my_strlen(options);j++){
     		strOptions[j] = malloc((my_strlen(str1)+1) * sizeof(char));
     	}
     	Crate_Options(strOptions,options,str1,start);
     	 if(flag1)
	     	Repetitions_Character2(path,fd,buffer, indexRpt ,strOptions,str2,optionsSize,1,caseSens,lock);
	     else if(flag2)
	     	Repetitions_Character2(path,fd,buffer, indexRpt ,strOptions,str2,optionsSize,2,caseSens,lock);
	     else
	     	Repetitions_Character2(path,fd,buffer, indexRpt ,strOptions,str2,optionsSize,3,caseSens,lock);

     }
	  else if(flag3){
	  	if(flag1)
	  		Repetitions_Character(path,fd,buffer, indexRpt ,str1,str2,1,caseSens,lock);
	  	else if(flag2)
	  		Repetitions_Character(path,fd,buffer, indexRpt ,str1,str2,2,caseSens,lock);
	  	else{
	  		Repetitions_Character(path,fd,buffer, indexRpt ,str1,str2,3,caseSens,lock);
	  	}

	  }
	  else{
	  	if(flag1)
	  		Replace_Two_String(path,fd,buffer ,str1,str2,1,caseSens,lock);
	  	else if(flag2)
	  		Replace_Two_String(path,fd,buffer ,str1, str2,2,caseSens,lock);
	  	else
	  		Replace_Two_String(path,fd,buffer ,str1, str2,3,caseSens,lock);
	  }
	  
  	free(options);
  	for(int j=0;j<my_strlen(options);j++){
 		free(strOptions[j]);
 	}	
 	free(buffer);	  	
 	free(path);
 	free(str1);
 	free(str2);
	


	return 0;

}