/************************************************************************
Program to condense network structures by combining short segments.
Reads Network.dat and writes NetworkCondensed.dat.
Only nodes with two segments attached can be removed.
These parameters must be varied within the program (beginning of main.cpp). 
ltarget: combine segments until this target is reached.
lmin: combine with another segment if a segment is shorter than this.
TWS, April 2019
***********************************************************************/
#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "nrutil.h"


int nseg,nnod,nnodbc;
int *nodtyp,*nodname,*segname,*segtyp,*ista,*iend,**segnodname,**nodnod,**nodseg;
float *diam,*q,*hd,*nodvar,*segvar,**cnode,*lseg;

int main(int argc, char *argv[])
{
	int iseg,inod,i,k,inodbc,inod1,inod2,flag,nodsegm,iseg1,iseg2,nsegact,nnodact;
	int max=100;
	float ltarget = 50.;	//combine segments until this target is reached
	float lmin = 10.;	//combine segments if a segment is shorter than this
	float lseg1, lseg2;
	char bb[100],s1[50], *pos;
	FILE *ifp,*ofp;

	ifp = fopen("Network.dat", "r");
	ofp = fopen("NetworkCondensed.dat", "w");
	sprintf(s1, "target length = %g, min length = %g", ltarget, lmin);
	for(i=1; i<=5; i++){
		fgets(bb,max,ifp);
		if (i == 1) {	//remove newline character from string
			if ((pos = strchr(bb, '\n')) != NULL) *pos = '\0';
			fprintf(ofp, "%s %s\n", bb, s1);	//add information about ltarget and lmin
		}
		else fprintf(ofp, "%s", bb);
	}
	fscanf(ifp,"%i%*[^\n]", &nodsegm);
	fscanf(ifp, "%i%*[^\n]", &nseg);
	fgets(bb,max,ifp);
	fgets(bb,max,ifp);
	segname = ivector(1,nseg); 
	segtyp = ivector(1,nseg); 
	ista = ivector(1,nseg); 
	iend = ivector(1,nseg); 
	segnodname = imatrix(1,2,1,nseg); 
	diam = vector(1,nseg); 
	q = vector(1,nseg); 
	hd = vector(1,nseg); 
	lseg = vector(1,nseg); 
	segvar = vector(1,nseg);
	for(iseg=1; iseg<=nseg; iseg++)	fscanf(ifp, "%i %i %i %i %f %f %f%*[^\n]",
		&segname[iseg],&segtyp[iseg],&segnodname[1][iseg],&segnodname[2][iseg],&diam[iseg],&q[iseg],&hd[iseg]);
	fgets(bb,max,ifp);
	fscanf(ifp,"%i%*[^\n]", &nnod);
	fgets(bb,max,ifp);
	fgets(bb,max,ifp);
	nodname = ivector(1,nnod); 
	nodtyp = ivector(1,nnod); 
	nodvar = vector(1,nnod); 
	cnode = matrix(1,3,1,nnod); 
	nodnod = imatrix(1,nodsegm,1,nnod);
	nodseg = imatrix(1,nodsegm,1,nnod);
	for(i=1; i<=nnod; i++)
		fscanf(ifp, "%i %f %f %f%*[^\n]", &nodname[i],&cnode[1][i],&cnode[2][i],&cnode[3][i]);
	for (iseg = 1; iseg <= nseg; iseg++) if (segtyp[iseg] == 4 || segtyp[iseg] == 5) {
		//Search for node name corresponding to start node name of this segment
		inod = segnodname[1][iseg];	//if node names are sequential, then no search is needed
		if (nodname[inod] == inod) {
			ista[iseg] = inod;
			goto foundit1;
		}
		for (inod = 1; inod <= nnod; inod++) if (nodname[inod] == segnodname[1][iseg]) {	//otherwise search
			ista[iseg] = inod;
			goto foundit1;
		}
		printf("*** Error: No matching node found for nodname %i\n", segnodname[1][iseg]);
	foundit1:;
		//Search for node corresponding to end node of this segment
		inod = segnodname[2][iseg];	//if node names are sequential, then no search is needed
		if (nodname[inod] == inod) {
			iend[iseg] = inod;
			goto foundit2;
		}
		for (inod = 1; inod <= nnod; inod++) if (nodname[inod] == segnodname[2][iseg]) {	//otherwise search
			iend[iseg] = inod;
			goto foundit2;
		}
		printf("*** Error: No matching node found for nodname %i\n", segnodname[2][iseg]);
	foundit2:;
	}
//Setup nodtyp, nodseg and nodnod
	nsegact = 0;
	for(inod=1; inod<=nnod; inod++) nodtyp[inod] = 0;
	for(iseg=1; iseg<=nseg; iseg++)	if(segtyp[iseg] == 4 || segtyp[iseg] == 5){
		nsegact++;
		inod1 = ista[iseg];
		inod2 = iend[iseg];
		nodtyp[inod1] = nodtyp[inod1] + 1;
		nodtyp[inod2] = nodtyp[inod2] + 1;
		if(nodtyp[inod1] > nodsegm) printf("*** Error: Too many segments connected to node %i, increase nodsegm in Network.dat\n", inod1);
		if(nodtyp[inod2] > nodsegm) printf("*** Error: Too many segments connected to node %i, increase nodsegm in Network.dat\n", inod2);
		nodseg[nodtyp[inod1]][inod1] = iseg;
		nodseg[nodtyp[inod2]][inod2] = iseg;
		nodnod[nodtyp[inod1]][inod1] = inod2;
		nodnod[nodtyp[inod2]][inod2] = inod1;
	}
//compute segment lengths
	for(iseg=1; iseg<=nseg; iseg++) if(segtyp[iseg] == 4 || segtyp[iseg] == 5){
		lseg[iseg] = 0.;
		for(k=1; k<=3; k++)	lseg[iseg] += SQR(cnode[k][ista[iseg]] - cnode[k][iend[iseg]]);
		lseg[iseg] = sqrt(lseg[iseg]);
	}

	printf("Initial number of segments = %i\n",nsegact);
	printf("Initial number of nodes = %i\n", nnod);
	
	do{		//keep removing nodes until targets are satisfied
		flag = 0;
		for(inod=1; inod<=nnod; inod++) if(nodtyp[inod] == 2){
			inod1 = nodnod[1][inod];
			inod2 = nodnod[2][inod];
			if (inod1 != inod2) {
				iseg1 = nodseg[1][inod];
				iseg2 = nodseg[2][inod];
				// lengths of the two segments
				lseg1 = 0.;
				lseg2 = 0;
				for (k = 1; k <= 3; k++) {
					lseg1 += SQR(cnode[k][inod1] - cnode[k][inod]);
					lseg2 += SQR(cnode[k][inod2] - cnode[k][inod]);
				}
				lseg1 = sqrt(lseg1);
				lseg2 = sqrt(lseg2);
				if ((lseg1 < ltarget && lseg2 < ltarget) || lseg1 < lmin || lseg2 < lmin) {	//eliminate inod, iseg2
					flag = 1;
					nodtyp[inod] = 0;	//notyp 0 signals that node is no longer included
					segtyp[iseg2] = 10;	//segtyp 10 signals that segment is no longer included
					if (ista[iseg1] == inod) ista[iseg1] = inod2;
					else if (iend[iseg1] == inod) iend[iseg1] = inod2;
					if (nodnod[1][inod1] == inod) nodnod[1][inod1] = inod2;
					else if (nodnod[2][inod1] == inod) nodnod[2][inod1] = inod2;
					if (nodnod[1][inod2] == inod) nodnod[1][inod2] = inod1;
					else if (nodnod[2][inod2] == inod) nodnod[2][inod2] = inod1;
					if (nodseg[1][inod2] == iseg2) nodseg[1][inod2] = iseg1;
					else if (nodseg[2][inod2] == iseg2) nodseg[2][inod2] = iseg1;
					if (segnodname[1][iseg1] == segnodname[1][iseg2]) segnodname[1][iseg1] = segnodname[2][iseg2];
					else if (segnodname[1][iseg1] == segnodname[2][iseg2]) segnodname[1][iseg1] = segnodname[1][iseg2];
					else if (segnodname[2][iseg1] == segnodname[1][iseg2]) segnodname[2][iseg1] = segnodname[2][iseg2];
					else if (segnodname[2][iseg1] == segnodname[2][iseg2]) segnodname[2][iseg1] = segnodname[1][iseg2];
				}
			}
		}
	}
	while(flag == 1);

	nsegact = 0;
	for (iseg = 1; iseg <= nseg; iseg++)	if (segtyp[iseg] == 4 || segtyp[iseg] == 5) nsegact++;
	printf("Final number of segments = %i\n", nsegact);
	nnodact = 0;
	for (i = 1; i <= nnod; i++) if (nodtyp[i] > 0) nnodact++;
	printf("Final number of nodes = %i\n", nnodact);

	fprintf(ofp,"%i  nodsegm, max. number of segments per node\n", nodsegm);
	fprintf(ofp,"%i  total number of segments \n", nsegact);	//reduced number of segments
	fprintf(ofp,"name type from  to  diam flow HD\n");
	for (iseg = 1; iseg <= nseg; iseg++) if (segtyp[iseg] == 4 || segtyp[iseg] == 5)	//only print included segments
		fprintf(ofp, "%i %i %i %i %8.3f %8.3f %8.4f\n",
			segname[iseg], segtyp[iseg], segnodname[1][iseg], segnodname[2][iseg], diam[iseg], q[iseg], hd[iseg]);
	fprintf(ofp," %i       total number of nodes\n", nnodact);
	fprintf(ofp,"name    x       y       z\n");
	for (i = 1; i <= nnod; i++) if(nodtyp[i] > 0)
		fprintf(ofp, "%i %7.0f %7.0f %7.0f\n", nodname[i], cnode[1][i], cnode[2][i], cnode[3][i]);

//boundary nodes - just copy from old file to new one
	fscanf(ifp,"%i%*[^\n]", &nnodbc);
	fgets(bb,max,ifp);
	fgets(bb,max,ifp);
	fprintf(ofp,"   %i  total number of boundary nodes\n", nnodbc);
	fprintf(ofp, "node bctyp press/flow   HD    PO2\n");
	for(inodbc=1; inodbc<=nnodbc; inodbc++){
		fgets(bb,max,ifp);
		fprintf(ofp,"%s",bb);
	}
	fclose(ifp);
	fclose(ofp);

	return 0;
}