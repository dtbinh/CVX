
#include "math.h"
#include "mdefs.h"
#include "mex.h"
#include "string.h"

/* This code differentiates the state vector for a 2D passive bipedal 
 * walker. It accepts several different kinds of inputs, thanks to Matlab's
 * inane function handling methodology.
 */

void mexFunction( int nlhs, mxArray *plhs[], 
                    int nrhs, const mxArray *prhs[] ) {

/* ****************************** */
/* Declare pointers and variables */
/* ****************************** */
    const int dim = 4;
    int rows, cols;
    double K, M, Mp, g, L, C, slope; /* The constants ... */
    mxArray *K_ptr, *M_ptr, *Mp_ptr, *g_ptr, *L_ptr, *C_ptr, *slope_ptr; /* ... and their counterparts. */
    char flag;
    double q1, q2; /* Angles */
    double q1dot, q2dot, q1dotnew, q2dotnew; /* Angle velocities */
    double q1dotdot, q2dotdot; /* Angle accels */
    plhs[0] = mxCreateDoubleMatrix(4, 1, mxREAL);
    
/* ************ */
/* Parse inputs */
/* ************ */    
    /* Matlab arrays are indexed in a wonderfully undocumented fashion.
     * Imagine that you want to get element (2,3) of a 10-row matrix.
     * Then row = 2, col = 3 and the index you want is row+col*mxGetM(mtrx)
     * Or, 2 + 3*10 = 32. You access this element like so:
     *         mxGetPr(prhs[0])[32]
     * "Fantastic!" you say. Ah, but you forget: C indexing starts from 0.
     *         mxGetPr(prhs[0])[1+2*10]
     * is how you really do it. */    
    if (mxGetM(prhs[0]) == dim || mxGetN(prhs[0]) == dim) {
        q1 = mxGetPr(prhs[0])[0];
        q2 = mxGetPr(prhs[0])[1];
        q1dot = mxGetPr(prhs[0])[2];
        q2dot = mxGetPr(prhs[0])[3];
        /* mexPrintf("q1 is %g\n", q1);
        mexPrintf("q2 is %g\n", q2);
        mexPrintf("q1dot is %g\n", q1dot);
        mexPrintf("q2dot is %g\n", q2dot); */
        if (nrhs == 2) {
            if (mxIsChar(prhs[1])) {
                flag = *mxGetChars(prhs[1]);
            } else {
                mexErrMsgTxt("If the 1st input is the state vector, the 2nd must be the flag 's' or 'i'.");
            }
        } else if (nrhs == 1) {
            flag = 's';
        } else {
            mexErrMsgTxt("If the 1st input is the state vector, you can only have 2 inputs and the 2nd must be the flag 's' or 'i'.");
        }
    } else if (mxGetM(prhs[1]) == dim || mxGetN(prhs[1]) == dim) {
        q1 = mxGetPr(prhs[1])[0]; 
        q2 = mxGetPr(prhs[1])[1]; 
        q1dot = mxGetPr(prhs[1])[2]; 
        q2dot = mxGetPr(prhs[1])[3];
        if (nrhs == 2) {
            flag = 's';
        } else if (nrhs == 3) {
            if (mxIsChar(prhs[1])) {
                flag = *mxGetChars(prhs[1]);
            } else {
                mexErrMsgTxt("If the 1st input is time, from ode45, and the 2nd is the state vector, then the 3rd must be the flag 's' or 'i'.");
            }
        } else {
            mexErrMsgTxt("If the 2nd input is the state vector, you can only have 3 inputs and the 3rd must be the flag 's' or 'i'. The 1st can be anything.");
        }
    } else {
        mexErrMsgTxt("The input state vector must be either the 1st or the 2nd argument, only.");
    }
    
    /* Are the global variables defined appropriately? */ 
    K_ptr = mexGetVariable("global", "K");
    M_ptr = mexGetVariable("global", "M"); 
    Mp_ptr = mexGetVariable("global", "Mp"); 
    g_ptr = mexGetVariable("global", "g"); 
    L_ptr = mexGetVariable("global", "L"); 
    C_ptr = mexGetVariable("global", "C"); 
    slope_ptr = mexGetVariable("global", "slope");

    if (K_ptr == NULL || M_ptr == NULL || Mp_ptr == NULL || g_ptr == NULL || L_ptr == NULL || L_ptr == NULL || C_ptr == NULL) {
        mexErrMsgTxt("The global workspace must contain the variables M, Mp, g, L, C and slope.");
    } else {    
	K = mxGetScalar(K_ptr);
        M = mxGetScalar(M_ptr);
        Mp = mxGetScalar(Mp_ptr);
        g = mxGetScalar(g_ptr);
        L = mxGetScalar(L_ptr);
        C = mxGetScalar(C_ptr);
        slope = mxGetScalar(slope_ptr);
    }
    
    /* Make sure that we know what it is we're supposed to be doing. */
    if (!strcmp(&flag, "s") && !strcmp(&flag, "i")) {
        flag = 's';
        mexPrintf("\n Bad inputs! Calculating equations of state by default. \n");
    }
           
/* ******************* */
/* Solve for new state */
/* ******************* */
    if (flag == 'i') {
        
        /* mexPrintf("Computing state after impact."); */
/* ********************************************************************* */
/*      q1dotnew = (Mp*q1dot*Cos(q1 - q2))/(Mp + M*Power(Sin(q1 - q2),2));
        q2dotnew = (Mp*q1dot*Power(Cos(q1 - q2),2))/(Mp + M*Power(Sin(q1 - q2),2));*/
        q1dotnew = (2*Mp*q1dot*Power(Cos(q1 - q2),2))/(M + 2*Mp - M*Cos(2*(-q1 + q2)));
        q2dotnew = (2*Mp*q1dot*Cos(q1 - q2))/(M + 2*Mp - M*Cos(2*(-q1 + q2)));
/* ********************************************************************* */        
        q1dot = q1dotnew;
        q2dot = q2dotnew;
        mxGetPr(plhs[0])[0] = q2;
        mxGetPr(plhs[0])[1] = q1;
        mxGetPr(plhs[0])[2] = q1dot;
        mxGetPr(plhs[0])[3] = q2dot;
        
    } else if (flag == 's') {
        
        /* mexPrintf("Computing state derivatives."); */
/* ************************************************* */
        q1dotdot = (-(g*((M + 2*Mp)*Sin(q1) + M*Sin(q1 - 2*q2))) - 2*L*M*Power(q2dot,2)*Sin(q1 - q2) + L*M*Power(q1dot,2)*Sin(2*(q1 - q2)))/(L*(-M - 2*Mp + M*Cos(2*(q1 - q2))));
        q2dotdot = (((M + Mp)*(-(L*Power(q1dot,2)) + g*Cos(q1)) + L*M*Power(q2dot,2)*Cos(q1 - q2))*Sin(q1 - q2))/(L*(M + Mp - M*Power(Cos(q1 - q2),2)));

/*STANCE EQ:
q1dotdot = (-K*Sin(.9*(q1-q2)) -2*L*M*Power(q2dot,2)*Sin(q1 - q2) +L*M*Power(q1dot,2)*Sin(2*(q1 - q2)))/(L*(-M - 2*Mp + M*Cos(2*(q1 - q2)))); */
/*NONSTANCE EQ:
q2dotdot = (K*Sin(.9*(q1-q2)) +(L*M*Power(q2dot,2)*Cos(q1 - q2) + -(M + Mp)*(L*Power(q1dot,2)))*Sin(q1 - q2))/(L*(M + Mp - M*Power(Cos(q1 - q2),2))); */
/* ************************************************* */
        mxGetPr(plhs[0])[0] = q1dot;
        mxGetPr(plhs[0])[1] = q2dot;
        mxGetPr(plhs[0])[2] = q1dotdot;
        mxGetPr(plhs[0])[3] = q2dotdot;
    }
    /*
    mexPrintf("q1 is %g\n", q1);
    mexPrintf("q2 is %g\n", q2); 
    mexPrintf("q1dot is %g\n", q1dot); 
    mexPrintf("q2dot is %g\n", q2dot);
    mexPrintf("M is %g\n", M); 
    mexPrintf("Mp is %g\n", Mp); 
    mexPrintf("g is %g\n", g); 
    mexPrintf("L is %g\n", L);  
    mexPrintf("C is %g\n", C); 
    mexPrintf("slope is %g\n", slope);
    mexPrintf("flag is %c\n", flag);
    mexPrintf("q1dot is %f\n", q1dot);
    mexPrintf("q2dot is %f\n", q2dot);
    mexPrintf("q1dotdot is %f\n", q1dotdot);
    mexPrintf("q1dotdot is %f\n", q2dotdot);
    */
}
