#include <stdio.h>
#include <stdlib.h>
#include "tree_pred.h"

int main(int argc,char** argv) {
	int n, i, j;
	int correct = 0;
	double *attr;
	int label, prediction,m;
	FILE* fp = fopen(argv[1],"r");
	fscanf(fp,"%d %d", &n,&m);
	attr = (double*) malloc(m*sizeof(double));
	
	for(i = 0; i < n; ++i) {
		fscanf(fp,"%d",&label);
		for(j=0;j<m;j++){
			fscanf(fp,"%lf",&attr[j]);
		}		
		prediction = tree_predict(attr);
		if(prediction == label) ++correct;
	}
	printf("Accuracy: %d/%d = %lf%%\n", correct, n, correct*100.0/n);
}
