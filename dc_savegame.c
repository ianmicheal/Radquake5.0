// ---------------------------------------
// speud (27-06-2004) - Replacing fprintf 
// ---------------------------------------

void DC_fprintf(FILE *stream, const char *format, ...) {
	char tmp_buf[40000];
	sprintf(tmp_buf, format);
	fwrite(tmp_buf, strlen(tmp_buf), 1, stream);
}


// -----------------------------------------
// BlackAura (08-12-2002) - Replacing fscanf
// -----------------------------------------

int cur; // speud (27-06-2004) - Replacing fgetc in Host_Loadgame_f

static void DC_ScanString(FILE *file, char *string)
{
	char newchar;
	fread(&newchar, 1, 1, file);
	while(newchar != '\n')
	{
		*string++ = newchar;
		fread(&newchar, 1, 1, file);
		cur ++;
	}
	*string++ = '\0';
}

static int DC_ScanInt(FILE *file)
{
	char sbuf[32768];
	DC_ScanString(file, sbuf);
	return Q_atoi(sbuf);
}

static float DC_ScanFloat(FILE *file)
{
	char sbuf[32768];
	DC_ScanString(file, sbuf);
	return Q_atof(sbuf);
}
