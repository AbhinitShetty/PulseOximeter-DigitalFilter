#include<stdio.h>
#include<math.h>
#include<stdlib.h>

#define  IIR_NMAX 10            // Maximum Order of Filter 
typedef struct {
	int n;
	double  num[IIR_NMAX];
	double  den[IIR_NMAX];
	double  x[IIR_NMAX];        //For Delay Block count we'll use x[k]
} IIR_object;

static IIR_object IIR_Hpf, IIR_Lpf, IIR_Notch;

//INITIALIZING IIR FILTER Coefficients & Delay Blocks
static void InitIIR() {          
	int  k;

	//------ HIGH PASS FILTER Specifications (fc = 0.5Hz) ------

	
	IIR_Hpf.n = 4;              //Order of Filter

	IIR_Hpf.num[0] =  0.971224810242597;    //b0
	IIR_Hpf.num[1] =  -3.884899240970387;   //b1
    IIR_Hpf.num[2] =  5.827348861455580;    //b2
    IIR_Hpf.num[3] =  -3.884899240970387;   //b3
    IIR_Hpf.num[4] =  0.971224810242597;    //b3

    IIR_Hpf.den[0] =  1.0;   			   //a0
    IIR_Hpf.den[1] = -3.941607386855387;   //a1
    IIR_Hpf.den[2] =  5.826520971017184;   //a2
    IIR_Hpf.den[3] = -3.828190973978209;   //a3
    IIR_Hpf.den[4] =  0.943277632030769;   //a3

	for (k = 0; k < IIR_NMAX; k++) {  //Delay Blocks added depending upon Coefficients standings
		IIR_Hpf.x[k] = 0.0;
	}

	// LOW PASS FILTER - 9Hz
	IIR_Lpf.n = 4;              //Order of Filter 

	IIR_Lpf.num[0] =   0.003227021246802;   //b0
	IIR_Lpf.num[1] =   0.012908084987208;   //b1
	IIR_Lpf.num[2] =   0.019362127480811;   //b2
	IIR_Lpf.num[3] =   0.012908084987208;   //b3
	IIR_Lpf.num[4] =   0.003227021246802;   //b4
	

	IIR_Lpf.den[0] =   1; 		 //a0
	IIR_Lpf.den[1] =  -2.548097716243580; 	 //a1
	IIR_Lpf.den[2] =   2.609926241233029; 	 //a2
	IIR_Lpf.den[3] =  -1.237146989281122; 	 //a3
	IIR_Lpf.den[4] =   0.226950804240504; 	 //a4
	
	for (k = 0; k < IIR_NMAX; k++) {  //Delay Blocks added depending upon Coefficients standings
		IIR_Lpf.x[k] = 0.0;
	}

}


// FILTER CALCULATIONS  
/* Which filter calculations are carried out in this function "IIRdfII" depends on 
   what's passed on to the function in *filtobj [Could be "&IIR_Hpf" or "&IIR_Lpf" as well] */
static double IIRcalc(IIR_object *filtobj, double u)  // "filtobj" is used as a pointer 
{
	int k, n;
	double  y;

	n = filtobj->n;
	// Output Equation
	y = filtobj -> num[0] *u + filtobj -> x[0] ;     // "filtobj ->" is to dereference a pointer
	for (k=1; k<n; k++){
		filtobj -> x[k-1] = filtobj->num[k] *u + filtobj->x[k] - filtobj->den[k] * y;  // Delay "State" Equations 
	}
	filtobj -> x[n-1] = filtobj->num[n] *u  - filtobj->den[n]*y ; 
	return y;
}


int main (){
	FILE *fi, *fo;  //Input File -> Read data & Output File -> Transfer Filtered Signal data
	int  k;
	double  u, y_hp, y_lp;
	
	printf("--- IIR Filter for ECG signals ---\n");
	if ((fi = fopen("Sample_2K_4K.txt", "r")) == NULL) {
		printf(" *** cannot open InputSamples.txt for reading.\n");
		exit(EXIT_FAILURE);
	}
	if ((fo = fopen("FixedPointTesting.txt", "w")) == NULL) {
		printf(" *** cannot open FilterOutput.txt for writing.\n");
		fclose(fi);
		exit(EXIT_FAILURE);
	}

    InitIIR();
	k = 0;
	printf("         k           u            y_lp               y_hp\n");
	printf("------------------------------------------------------\n");
	while (!feof(fi)) {
		if ((fscanf(fi, "%lf", &u)) == 1) {
			k++;
			y_lp = IIRcalc(&IIR_Lpf, u);
			y_hp= IIRcalc(&IIR_Hpf, y_lp);
			printf("%10d  %10.5g  %14.6g  %14.6g\n", k, u, y_lp, y_hp);
			fprintf(fo, "%17.7g\n", y_hp);
		}
	}
	printf("------------------------------------------------------\n");
	printf(" %d lines processed.\n", k);
	fclose(fo);
	fclose(fi);
	printf("That`s all, folks.\n");
	return EXIT_SUCCESS;
}
