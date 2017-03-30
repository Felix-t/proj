#include "Fonctions_Utiles.h" 



/********************************************************/
/*Fonction: Zero_str                                    */
/*                                                      */
/********************************************************/
void Zero_str(char *str)
{

	memset(str,0,sizeof(str));

}



/********************************************************/
/*Fonction: Find_substr_in_str                          */
/*                                                      */
/********************************************************/
int Find_substr_in_str(char *str, char *substr)
{
	int ok,len_str,len_substr;
	char *ret;

	len_str=strlen(str);
	len_substr=strlen(substr);

	if (len_str<len_substr){ok=0;goto fin_Find_substr_in_str;}

	ret=strstr(str,substr);
	if (ret!=NULL)
		/*if (strncmp(ret,substr,len_substr)==0)*/
	{
		ok=1;
	}else{
		ok=0;
	}

fin_Find_substr_in_str:;
		       return ok;
}







/********************************************************/
/*Fonction: Find_position_substr_in_str                 */
/*                                                      */
/********************************************************/
int Find_position_substr_in_str(char *str, char *substr)
{
	int i,j,pos;
	int len_str,len_substr;
	char *temp_str;

	len_str=strlen(str);
	len_substr=strlen(substr);

	pos=-1;
	if ((len_substr==0)||(len_str<len_substr)){goto fin_Find_position_substr_in_str;}

	temp_str=malloc((len_str+1)*sizeof(char));
	//pos=len_str+1;


	for(i=0;i<(len_str-len_substr+1);i++)
	{
		Zero_str(temp_str);
		for(j=0;j<len_substr;j++)
		{
			temp_str[j]=str[i+j];
		}
		temp_str[len_substr]='\0';

		if(Compare_2str(temp_str, substr)==1){pos=i;break;}

	}

	free(temp_str);
	temp_str=NULL;

fin_Find_position_substr_in_str:;

				return pos;
}








/********************************************************/
/*Fonction: Compare_2str                          */
/*                                                      */
/********************************************************/
int Compare_2str(const char *str1, char *str2)
{
	int i,ok,ok_c,len_str1,len_str2;
	char c_str1,c_str2;

	len_str1=strlen(str1);
	len_str2=strlen(str2);

	if (len_str1!=len_str2){ok=0;goto fin_compare_2str;}

	ok=1;
	for(i=0;i<len_str1;i++)
	{
		c_str1=str1[i];c_str2=str2[i];
		if(c_str1==c_str2){ok_c=1;}else{ok_c=0;}

		ok=ok*ok_c;
	}


fin_compare_2str:;
		 return ok;
}






/********************************************************/
/*Fonction: Execute_file_sh                             */
/*                                                      */
/********************************************************/
int Execute_file_sh(char * str_cmd)
{
	char *res_df,buf[256];
	FILE * pp;
	int ok=1;

	if ((pp = popen(str_cmd, "r")) == NULL)
	{
		ok=0;
		printf("%s %s\n","Echec de la commande ",str_cmd);
	}else{
		while(fgets(buf, sizeof buf, pp)!=NULL)
		{
			printf("\t->%s",buf);
		}
	}
	pclose(pp);
	return ok;
}



/********************************************************/
/*Fonction: Execute_file_sh_sans_retour_info            */
/*                                                      */
/********************************************************/
int Execute_file_sh_sans_retour_info (char * str_cmd)
{
	char *res_df,buf[256];
	FILE * pp;
	int ok=1;

	if ((pp = popen(str_cmd, "r")) == NULL)
	{
		ok=0;
		printf("%s %s\n","Echec de la commande ",str_cmd);
	}
	pclose(pp);
	return ok;
}




/********************************************************/
/*Fonction: Lecture saisie                              */
/*                                                      */
/********************************************************/
int fgets_stdin(char *str_line,int size_str_line)
{
	int ok;
	ok=1;

	fgets(str_line,size_str_line,stdin);

	char *p=strchr(str_line,'\n');
	if (p!=NULL)
	{
		*p=0;
	}else{

		int c;
		while((c=fgetc(stdin)) !='\n' && c != EOF){}
	}



	return ok;
}




/********************************************************/
/*Fonction: Verification_saisie                              */
/*                                                      */
/********************************************************/
int Verification_saisie(char *modif_date_time, char *choix_possible_modif_date_time_WLX2[], int nb_choix)
{
	//char temp_str[100];
	int i,ok=0;


	for(i=0;i<nb_choix;i++)
	{
		//Zero_str(temp_str);
		//strcat(temp_str,choix_possible_modif_date_time_WLX2[i]);strcat(temp_str,"\n");
		//if(Compare_2str(modif_date_time,temp_str)){ok=i+1;break;}
		if(Compare_2str(modif_date_time,choix_possible_modif_date_time_WLX2[i])){ok=i+1;break;}
	}
	return ok;
}





/********************************************************/
/*Fonction: Calcul_free_space                           */
/********************************************************/
float Calcul_free_space(char *chemin, int ok_print)
{
	int i;
	char command_df[500]="df -h ";
	char *res_df,res_df_value[10],res_df_unit;
	float multiple=1.0,df_value=0.0,temp_df_value=0.0;

	FILE * pp;
	char buf[256];

	res_df_value[0]='\0';
	df_value=0.0;

	strcat(command_df,chemin);
	//printf("%s\n",command_df);

	if ((pp = popen(command_df, "r")) != NULL)
	{
		fgets(buf, sizeof buf, pp);
		//fputs(buf, stdout);
		fgets(buf, sizeof buf, pp);
		pclose(pp);
		//printf("%s\n",buf);
		res_df=strtok(buf," ");

		res_df=strtok(NULL," ");
		res_df=strtok(NULL," ");
		res_df=strtok(NULL," ");
		//printf("%s\n",res_df);
		for (i=0;i<(strlen(res_df)-1);i++){res_df_value[i]=res_df[i];}
		res_df_value[i]='\0';
		res_df_unit=res_df[i];
		//printf("\n%s %c\n",res_df_value,res_df_unit);
		/*while(res_df!=NULL)
		  {
		  printf("%s\n",res_df);
		  res_df=strtok(NULL," ");
		  }*/

		switch(res_df_unit)
		{
			case 'K':
				multiple=1.0;
				break;
			case 'M':
				multiple=1000.0;
				break;
			case 'G':
				multiple=1000000.0;
				break;
		}

		if (ok_print){printf("-> %s %s%c\n","Espace disponible pour l'enregistrement des données: ",res_df_value,res_df_unit);}

		temp_df_value=(atof(res_df_value));
		df_value=temp_df_value*multiple;
	}
	//printf("%f %f\n",temp_df_value,df_value);
	return df_value;
}




/********************************************************/
/*Fonction: Verif_if_dir_exist                           */
/********************************************************/
int Verif_if_dir_exist(char *chemin)
{
	int ok,i_len;
	char test_file[1024]="",test[1024]="test";

	FILE *fp;

	struct stat info;

	if (stat(chemin,&info)!=0)
	{
		ok=0;
		printf("%s %s %s\n","Erreur: Le chemin ",chemin," est invalide");
	}else if (info.st_mode & S_IFDIR)
	{
		strcat(test_file,chemin);
		i_len=strlen(test_file)-1;
		if(test_file[i_len]!='/')
		{
			strcat(test_file,"/");
		}
		strcat(test_file,test);


		if ((fp=fopen(test_file,"w"))==NULL)
		{
			ok=0;
			printf("%s %s\n","Error: Impossible de créer un fichier dans",chemin);
		}else{
			fclose(fp);
			ok=1;
			remove(test_file); 
		}

	}else{
		ok=0;
		printf("%s %s %s\n","Error: ",chemin," n'est pas répertoire");
	}

	return ok;

}




/********************************************************/
/*Fonction: find_value_in_int_array                     */
/*type: int                                             */
/*return:   -1 si non trouvé                            */
/*         ou la position de la valeur dans le tableau */
/********************************************************/
int find_value_in_int_array(int value, const int array[])
{
	int i,pos_find;
	int len_array;

	pos_find=-1;

	len_array=array[0];
	//printf("len_array %d\n",len_array);
	i=1;
	while(i<len_array)
	{
		//printf("array %d %d\n",i,array[i]);
		if(array[i]==value){pos_find=1;break;} 
		i++;
	}


	return pos_find;

}








