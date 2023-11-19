#include<stdio.h>
#include<math.h>
#include<stdlib.h>

//-------------------------------------------- DOUBLE --> FIXED pt Conversions ----------------------------------------//

inline signed int double_to_fixed(double input, int num) {	  // Converts double to int 16.4 format
	return (input * pow(2,num));
}

//-------------------------------------------- FIXED --> DOUBLE Conversions ----------------------------------------//

inline double fixed_to_double(signed long int y_out, int num){
	return (y_out * pow(2, num));
}

//---------------------------------------------------------------------------------------------------------------------------
#define  IIR_NMAX 10            		// Maximum Order of Filter
typedef struct {				        // Structure required to define a filter
	int n;						        // n -> Order of filter
	signed __int16  num[IIR_NMAX];      // Numerator Coefficients
	signed __int16  den[IIR_NMAX];	    // Denominator Coefficients
	signed __int32  x[IIR_NMAX];        // Delay Block count x[k]
} IIR_object;

static IIR_object IIR_Hpf, IIR_Lpf;		//Instances that will use 'IIR_object' structure

//--------------------------- INITIALIZING IIR FILTER Coefficients & Delay Blocks -----------------------------------//

static void InitIIR() {
	int  k;
	int frac_lpf = 13;		//Fractional part of LPF will have 13 places after decimal
	int frac_hpf = 12;		//Fractional part of LPF will have 12 places after decimal

	// Coefficient values of both these filters are obtained by designing the filter in MATLAB
	//------ LOW PASS FILTER Specifications (fc = 9Hz) ------//
	
	IIR_Lpf.n = 4;              //Order of Filter

	IIR_Lpf.num[0] =   double_to_fixed(0.003227021246802, frac_lpf);   //b0 coefficient
	IIR_Lpf.num[1] =   double_to_fixed(0.012908084987208, frac_lpf);   //b1 coefficient
	IIR_Lpf.num[2] =   double_to_fixed(0.019362127480811, frac_lpf);   //b2 coefficient
	IIR_Lpf.num[3] =   double_to_fixed(0.012908084987208, frac_lpf);   //b3 coefficient
	IIR_Lpf.num[4] =   double_to_fixed(0.003227021246802, frac_lpf);   //b4 coefficient

	IIR_Lpf.den[0] =   double_to_fixed(1.0,               frac_lpf);   //a0 coefficient
	IIR_Lpf.den[1] =  double_to_fixed(-2.548097716243580, frac_lpf);   //a1 coefficient
	IIR_Lpf.den[2] =   double_to_fixed(2.609926241233029, frac_lpf);   //a2 coefficient
	IIR_Lpf.den[3] =  double_to_fixed(-1.237146989281122, frac_lpf);   //a3 coefficient
	IIR_Lpf.den[4] =   double_to_fixed(0.226950804240504, frac_lpf);   //a4 coefficient

	//Delay Blocks added depending upon Coefficients standings
	for (k = 0; k < IIR_NMAX; k++) {  
		IIR_Lpf.x[k] = double_to_fixed(0.0, frac_lpf);
	}


	//------ HIGH PASS FILTER Specifications (fc = 0.5Hz) ------//

	IIR_Hpf.n = 4;              //Order of Filter

	IIR_Hpf.num[0] =  double_to_fixed(0.971224810242597,  frac_hpf);   //b0 coefficient
	IIR_Hpf.num[1] =  double_to_fixed(-3.884899240970387, frac_hpf);   //b1 coefficient
    IIR_Hpf.num[2] =  double_to_fixed(5.827348861455580,  frac_hpf);   //b2 coefficient
    IIR_Hpf.num[3] =  double_to_fixed(-3.884899240970387, frac_hpf);   //b3 coefficient
    IIR_Hpf.num[4] =  double_to_fixed(0.971224810242597,  frac_hpf);   //b4 coefficient

    IIR_Hpf.den[0] =  double_to_fixed(1.0,                 frac_hpf);   //a0 coefficient
    IIR_Hpf.den[1] =  double_to_fixed(-3.941607386855387,  frac_hpf);   //a1 coefficient
    IIR_Hpf.den[2] =  double_to_fixed(5.826520971017184,   frac_hpf);   //a2 coefficient
    IIR_Hpf.den[3] =  double_to_fixed(-3.828190973978209,  frac_hpf);   //a3 coefficient
    IIR_Hpf.den[4] =  double_to_fixed(0.943277632030769,   frac_hpf);   //a4 coefficient

	//Delay Blocks added depending upon Coefficients standings
	for (k = 0; k < IIR_NMAX; k++) {  
		IIR_Hpf.x[k] = double_to_fixed(0.0, frac_hpf);
	}

}
//------------------------------------------------------------------------------------------------------------------//



//------------------------------------------------- FILTER CALCULATIONS --------------------------------------------//

/* Which filter calculations are carried out in this function "IIRcalc" depends on
   what's passed on to the function in *filtobj (IIR_Hpf or IIR_Lpf) */

static signed int IIRcalc(IIR_object *filtobj, signed int fp_u, signed int val)  // "filtobj" is used as a pointer
{
	int k, n;
	signed long int y;		//Output variable of the filter
    signed int y_fixed;		//Fixed-point equivalent of output variable of the filter

	n = filtobj->n;
	y = (filtobj -> num[0] *fp_u + filtobj -> x[0]);    // "filtobj ->" is to dereference a pointer to a "structure"
	y = y << 3;
	y_fixed = y * pow(2, val);

	for (k=1; k<n; k++){
		// Delay "State" Equations
		filtobj -> x[k-1] = filtobj->num[k] *fp_u + filtobj->x[k] - filtobj->den[k] * y_fixed ;  
		}

	filtobj -> x[n-1] = filtobj->num[n] *fp_u  - filtobj->den[n]* y_fixed ; // Final Delay Equation Calculation
	return y_fixed;
}

//----------------------------------------------------------------------------------------------------------------------

int main (){
	FILE *fi, *fo;	//Pointers used to read raw samples from a file & write filtered output results to a file
	int  k;
	double input;  						//Input samples taken in 'double' format
	signed int fixed_input;				//Variable that stores converted form of input value
	signed int out_lp, out_hp;			//Output of LPF & HPF respectively
	double double_y_lp, double_y_hp;	//Used to check whether the Fixed-pt results obtained are of correct value

	//Reading Raw input samples from a text file
	printf("--- IIR Filter for ECG signals ---\n");
	if ((fi = fopen("Sample_2K_4K.txt", "r")) == NULL) {
		printf(" *** cannot open InputSamples.txt for reading.\n");
		exit(EXIT_FAILURE);
	}

	//Writing Filtered output data to a text file
	if ((fo = fopen("FixedPointTesting.txt", "w")) == NULL) {
		printf(" *** cannot open FilterOutput.txt for writing.\n");
		fclose(fi);
		exit(EXIT_FAILURE);
	}

    InitIIR();		//Initialization of LPF & HPF
	k = 0;
	printf("         k           input            out_lp               Float_lp               out_hp               Float_hp\n");
	printf("------------------------------------------------------\n");

	while (!feof(fi)) {		//Reading samples through input file until End of File
		if ((fscanf(fi, "%lf", &input)) == 1) {
			k++;

			//LOWPASS Filter Conversions
			fixed_input = double_to_fixed(input, 4);	   //Converting input sample from 'double' to 'fixed-pt' format
			out_lp = IIRcalc(&IIR_Lpf, fixed_input, -16);  //Coefficients Fractional Bits Value[13] + 3
			double_y_lp = fixed_to_double(out_lp, -4);     //For verifying whether the LPF output is correct 

			//HIGHPASS Filter Conversions
			out_hp = IIRcalc(&IIR_Hpf, out_lp, -15);	   //Coefficients Fractional Bits Value[12] + 3
            double_y_hp = fixed_to_double(out_hp, -4);     //For verifying whether the HPF output is correct

			printf("%10d %10ld %10ld = %.3lf  %10ld = %.3lf\n", k, fixed_input, out_lp, double_y_lp, out_hp, double_y_hp);
			fprintf(fo, "%.4lf\n", double_y_lp);		   //Writing Filtered output results to a text file
		}
	}
	printf("------------------------------------------------------\n");
	printf(" %d lines processed.\n", k);   //Number of samples processed 
	fclose(fo);							   //Close output file
	fclose(fi);							   //Close input file
	printf("That`s all.\n");
	return EXIT_SUCCESS;
}

/*for (k=0; k<=4; k++){
		printf("%d Num value is : %ld\n",k, IIR_Lpf.num[k]);
		printf("%d Den value is : %ld\n",k, IIR_Lpf.den[k]);
	}*/
