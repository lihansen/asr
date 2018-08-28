#include <stdio.h>
#include <math.h>
#include <stdlib.h>
//#include<iostream>
//extern "C"{
//    static void init();
//    void get_flag(int *in1,int * in2,int *out1, int *out2,int *flag);
//}
void four1(double *data, int n, int isign)
{
    int nn, mmax, m, j, istep, i;
    double wtemp, wr, wpr, wpi, wi, theta, tempr, tempi, swap1, swap2;
    nn = n << 1;
    j = 1;
    for (i = 1; i < nn; i += 2)
    {
        if (j > i)
        {
            swap1 = data[j - 1];
            data[j - 1] = data[i - 1];
            data[i - 1] = swap1;
            swap2 = data[j];
            data[j] = data[i];
            data[i] = swap2;
        }
        m = n;
        while ((m >= 2) && (j > m))
        {
            j -= m;
            m >>= 1;
        }
        j += m;
    }
    mmax = 2;
    while (nn > mmax)
    {
        istep = mmax << 1;
        theta = isign * (6.28318530717959 / mmax);
        wtemp = sin(0.5 * theta);
        wpr = -2.0 * wtemp * wtemp;
        wpi = sin(theta);
        wr = 1.0;
        wi = 0.0;
        for (m = 1; m<mmax; m += 2)
        {
            for (i = m; i <= nn; i += istep)
            {
                j = i + mmax;
                tempr = wr * data[j - 1] - wi * data[j];
                tempi = wr * data[j] + wi * data[j - 1];
                data[j - 1] = data[i - 1] - tempr;
                data[j] = data[i] - tempi;
                data[i - 1] += tempr;
                data[i] += tempi;
            }
            wr = (wtemp = wr) * wpr - wi * wpi + wr;
            wi = wi * wpr + wtemp * wpi + wi;
        }
        mmax = istep;
    }
}
void realft(double data[],int n,int isign)
{
    int i, i1, i2, i3, i4;
    double c1 = 0.5, c2, h1r, h1i, h2r, h2i, wr, wi, wpr, wpi, wtemp;
    double theta = 3.141592653589793238 / (double)(n >> 1);
    if (isign == 1)
    {
        c2 = -0.5;
        four1(data, n / 2, 1);
    }
    else
    {
        c2 = 0.5;
        theta = -theta;
    }
    wtemp = sin(0.5 * theta);
    wpr = -2.0 * wtemp * wtemp;
    wpi = sin(theta);
    wr = 1.0 + wpr;
    wi = wpi;
    for (i = 1; i < (n >> 2); i++)
    {
        i2 = 1 + (i1 = i + i);
        i4 = 1 + (i3 = n - i1);
        h1r = c1 * (data[i1] + data[i3]);
        h1i = c1 * (data[i2] - data[i4]);
        h2r = -c2 * (data[i2] + data[i4]);
        h2i = c2 * (data[i1] - data[i3]);
        data[i1] = h1r + wr * h2r - wi * h2i;
        data[i2] = h1i + wr * h2i + wi * h2r;
        data[i3] = h1r - wr * h2r + wi * h2i;
        data[i4] = -h1i + wr * h2i + wi * h2r;
        wr = (wtemp = wr) * wpr - wi * wpi + wr;
        wi = wi * wpr + wtemp * wpi + wi;
    }
    if (isign == 1)
    {
        data[0] = (h1r = data[0]) + data[1];
        data[1] = h1r - data[1];
    }
    else
    {
        data[0] = c1 * ((h1r = data[0]) + data[1]);
        data[1] = c1 * (h1r - data[1]);
        four1(data, n / 2, -1);
    }
}
void sort(double *array, int len)
{
    double *pi, *pj;
    double temp = 0.0;
    for (pi = array; pi < array+len - 1; pi++)
    {
        for (pj = pi + 1; pj <array+ len; pj++)
        {
            if (*pj>*pi)
            {
                temp = *pi;
                *pi = *pj;
                *pj = temp;
            }
        }
    }
}
void PowerCount(int len, int inc, double data[], int n,double *f,int fh)
{
    int i=0,j,ii,i1,i2;
    double a[1024],*pa;
    for (pa = f; pa < f+fh; pa++)
    {
        *pa = 0.0;
        for (j = 0; j < len; j++)
        {
            a[j] = data[i*inc + j];
        }
        i++;
        realft(a,sizeof(a)/sizeof(a[0]), 1);
        for (ii =0; ii <193; ii++)
        {
            i1 = 2 * (ii + 63);
            i2 = 2 * (ii + 63) + 1;
            *pa = *pa+sqrt(pow(a[i1], 2)+ pow(a[i2], 2))/193.0;
        }
    }
}
 void ChannelCompare(short Buffer_ch1[],short Buffer_ch2[],double Power_ch1[],
                     double Power_ch2[], short data_in1[], short data_in2[],
                     int *Number_re, int channel_flag[],int *out_flag,double *Power_now,int *Number_flagch)
{
//    printf("{\n");
//    printf("short:");
//    for (int x =0;x<1024;x++){
//        printf(" %d ",data_in1[x]);
//    }
//    printf("\n");
    int i=0, ii=0, channel_flag1=0;
    double Power_temp1=0.0, Power_temp2=0.0, Diff_ch1, Diff_ch2, Ratio_ch,data_temp1[1024], data_temp2[1024], data_temp3[1024+512], data_temp4[1024 + 512];

    if (*Number_re < 48)
    {

        if (*Number_re < 16)
        {
            for (ii = 0;ii<1024;ii++)
            {
                Buffer_ch1[*Number_re * 1024 + ii] = data_in1[ii];
                Buffer_ch2[*Number_re * 1024 + ii] = data_in2[ii];
            }
            *Number_re = *Number_re + 1;
            *out_flag = 0;

            if (*Number_re == 1)
            {
                for (i = 0; i < 1024; i++)
                {
                    data_temp1[i] = (double)(Buffer_ch1[i] / (pow(2, 15) - 1));
                    data_temp2[i] = (double)(Buffer_ch2[i] / (pow(2, 15) - 1));
                }

                PowerCount(1024, 512, data_temp1, sizeof(data_temp1) / sizeof(data_temp1[0]), &Power_ch1[0],1);
                PowerCount(1024, 512, data_temp2, sizeof(data_temp2) / sizeof(data_temp2[0]), &Power_ch2[0],1);
            }
            else
            {
                for (i = 0; i < 1024+512; i++)
                {
                    data_temp3[i] = (double)(Buffer_ch1[(*Number_re-2)*1024+512+i] / (pow(2, 15) - 1));
                    data_temp4[i] = (double)(Buffer_ch2[(*Number_re - 2) * 1024 + 512 + i] / (pow(2, 15) - 1));
                }

                PowerCount(1024, 512, data_temp3, sizeof(data_temp3) / sizeof(data_temp3[0]), &Power_ch1[(*Number_re - 1) * 2-1],2);
                PowerCount(1024, 512, data_temp4, sizeof(data_temp4) / sizeof(data_temp4[0]), &Power_ch2[(*Number_re - 1) * 2-1],2);
            }

            return;
        }

        for (ii = 0; ii < 1024 * 15; ii++)
        {
            Buffer_ch1[ii] = Buffer_ch1[1024 + ii];
            Buffer_ch2[ii] = Buffer_ch2[1024 + ii];
        }
        for (ii = 0; ii < 1024; ii++)
        {
            Buffer_ch1[15 * 1024 + ii] = data_in1[ii];
            Buffer_ch2[15 * 1024 + ii] = data_in2[ii];
        }
        *Number_re = *Number_re + 1;
        *out_flag = 0;

        for (i = 0; i < 1024 + 512; i++)
        {
            data_temp3[i] = (double)(Buffer_ch1[14*1024+512+i] / (pow(2, 15) - 1));
            data_temp4[i] = (double)(Buffer_ch2[14 * 1024 + 512 + i] / (pow(2, 15) - 1));
        }

        PowerCount(1024, 512, data_temp3, sizeof(data_temp3) / sizeof(data_temp3[0]), &Power_ch1[(*Number_re - 1) * 2-1],2);
        PowerCount(1024, 512, data_temp4, sizeof(data_temp4) / sizeof(data_temp4[0]), &Power_ch2[(*Number_re - 1) * 2-1],2);
        return;
    }

    if (*Number_re >= 48)
    {
//        *Number_re = *Number_re + 1;

        if (*Power_now == 0.0)
        {
            sort(Power_ch1,95);
            Power_temp1 = Power_ch1[47];
            sort(Power_ch2,95);
            Power_temp2 = Power_ch2[47];
            if (Power_temp1>Power_temp2)
                *Power_now = Power_temp2;
            else
                *Power_now = Power_temp1;
        }

        for (ii = 0; ii < 1024 * 15; ii++)
        {
            Buffer_ch1[ii] = Buffer_ch1[1024 + ii];
            Buffer_ch2[ii] = Buffer_ch2[1024 + ii];
        }
        for (ii = 0; ii < 93; ii++)
        {
            Power_ch1[ii] = Power_ch1[ii+2];
            Power_ch2[ii] = Power_ch2[ii+2];
        }
        for (ii = 0; ii < 1024; ii++)
        {
            Buffer_ch1[15 * 1024 + ii] = data_in1[ii];
            Buffer_ch2[15 * 1024 + ii] = data_in2[ii];
        }

        for (i = 0; i < 1024 + 512; i++)
        {
            data_temp3[i] =(double)(Buffer_ch1[14*1024+512+i] / (pow(2, 15) - 1));
            data_temp4[i] =(double)(Buffer_ch2[14 * 1024 + 512 + i] / (pow(2, 15) - 1));
        }

        PowerCount(1024, 512, data_temp3, sizeof(data_temp3) / sizeof(data_temp3[0]), &Power_ch1[93],2);
        PowerCount(1024, 512, data_temp4, sizeof(data_temp4) / sizeof(data_temp4[0]), &Power_ch2[93],2);

        Diff_ch1 = Power_ch1[94] / *Power_now;
        Diff_ch2 = Power_ch2[94] / *Power_now;
        Ratio_ch = Power_ch1[94] / Power_ch2[94];
//        printf("diffch1-----------------------------------------%f\n",Diff_ch1);
//        printf("diffch2-----------------------------------------%f\n",Diff_ch2);

        channel_flag1 = 0;
        if (Diff_ch1 >= 1 && Diff_ch2 >= 1)
        {
            if  (Ratio_ch > 1.3)
                channel_flag1 = 1;
            else if  (Ratio_ch < 0.7692)
                channel_flag1 = 2;
            else if  ((Diff_ch1>1.8)&&(Diff_ch2>1.8)&&(1.3>Ratio_ch)&&(Ratio_ch>0.7692))
                channel_flag1 = 0; //3
            else
                channel_flag1 = 0;
        }
        else if ((Diff_ch1 >= 1) && (Diff_ch2 < 1))
        {
            if  (Diff_ch1 > 1.4 )
                channel_flag1 = 1;
            else
                channel_flag1 = 0;
        }
        else if ((Diff_ch1 < 1)&& (Diff_ch2 >= 1))
        {
            if  (Diff_ch2 > 1.4)
                channel_flag1 = 2;
            else
                channel_flag1 = 0;
        }
        else
            channel_flag1 = 0;
//        if(channel_flag1==3)
//            printf("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&%d",channel_flag1);
//        printf("}\n");
        *Number_flagch = *Number_flagch + 1;
        if (*Number_flagch <= 16)
        {
            channel_flag[*Number_flagch-1] = channel_flag1;
            *out_flag = 0;
        }
        else
        {
            if (*Number_flagch >10000){
                *Number_flagch = 16+(*Number_flagch -16)%6;
            }
            for (i = 0; i < 15; i++)
            {
                channel_flag[i] = channel_flag[1 + i];
            }
            channel_flag[15] = channel_flag1;

            *out_flag = 1;

//            for (i=0;i<16;i++){
//                printf("%d ",channel_flag[i]);
//            }
//            printf("\n");



        }
    }
}

 void DataOut(short Buffer_ch1[], const int len_bch1,
              short Buffer_ch2[],const int len_bch2, int channel_flag[],const int len_cf,
              int *dataout_flag, short *data_out1, short *data_out2,const int len_dout)
{
    int i, num0 = 0,num1 = 0,num2 = 0,num3 = 0;
//    printf("buffer:");
    for (i = 0; i < len_dout; i++)
    {
        data_out1[i] = Buffer_ch1[5 * 1024 + i];
        data_out2[i] = Buffer_ch2[5 * 1024 + i];
//        printf("%d",Buffer_ch1[5*1024 +i]);
    }
//    printf("\n");
//    short *pa,*pb;
//    for (pa = data_out1; i < len_dout; pa++)
//    {
//        *pa = Buffer_ch1[5 * 1024 + i];
//
//    }
//    for (pb = data_out2; i < len_dout; pb++)
//    {
//
//        *pb = Buffer_ch2[5 * 1024 + i];
//    }
    for (i = 0; i < len_cf; i++)
    {
        if (channel_flag[i] == 0)
            num0 = num0 + 1;
        else if (channel_flag[i] == 1)
            num1 = num1 + 1;
        else if (channel_flag[i] == 2)
            num2 = num2 + 1;
        else
            num3 = num3 + 1;
    }
    if (num3!= 0)
        *dataout_flag = 3;
    else
    {
        if (num2 != 0)
        {
            if (num1 != 0)
            {
                if ((num2 >= 2)&& (num1 >= 2))
                    *dataout_flag = 3;
                else if ((num2 >= 2)&& (num1 < 2))
                    *dataout_flag = 2;
                else if ((num2 < 2)&& (num1 >= 2))
                    *dataout_flag = 1;
                else
                    *dataout_flag = 0;
            }
            else
                *dataout_flag = 2;
        }
        else
        {
            if (num1!= 0 )
                *dataout_flag = 1;
            else
                *dataout_flag = 0;
        }
    }
}

const int len_bch1 = 1024 * 16, len_bch2 = 1024 * 16, len_cf = 16, len_dout = 1024* 6;
short Buffer_ch1[1024 * 16], Buffer_ch2[1024 * 16];//, data_out1[1024 * 6], data_out2[1024 * 6];
int Number_re = 0, out_flag = 0, Number_flagch = 0,dataout_flag=0, channel_flag[16]= { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
double Power_ch1[95], Power_ch2[95], Power_now = 0.0;
short data_in1[1024], data_in2[1024];

 void init(){
    int i = 0;

    for (i = 0; i < 1024 * 16; i++)
    {
        Buffer_ch1[i] = 0;
        Buffer_ch2[i] = 0;
    }
    for (i = 0; i < 95; i++)
    {
        Power_ch1[i] = 0.0;
        Power_ch2[i] = 0.0;
    }
//    for (i = 0; i < 1024 * 6; i++)
//    {
//        data_out1[i] = 10;
//        data_out2[i] = 10;
//    }
}

void get_channel1(char *stream,short *left,short *right) {

    short *p1, *p2, *p3;
    p1 = (short *) stream;
    p2 = (short *) left;
    p3 = (short *) right;
    for (int p = 0; p < 2048; p += 2) {
        *p2 = *p1;
        p2++;
        *p3 = *(p1 + 1);
        p3++;
    }
}

void get_channel2(char *stream,char *left,char *right) {

    int i=0,p=0;
    while(p<1024*4){
        left[i] = stream[p++];
        left[i+1] = stream[p++];

        right[i] = stream[p++];
        right[i+1] = stream[p++];
        i+=2;
    }

}


void get_flag(char *stream,char *out1, char *out2,int *flag,int *_out_flag,int *_number_flagch){

//    short in1 [1024],in2 [1024];
//    get_channel1(stream,in1,in2);
;
    get_channel2(stream,out1,out2);

    ChannelCompare(Buffer_ch1, Buffer_ch2, Power_ch1, Power_ch2,(short *)out1,(short *)out2,
                   &Number_re, channel_flag, &out_flag, &Power_now, &Number_flagch);
    *_number_flagch = Number_flagch;
    *_out_flag = out_flag;
    if ((out_flag == 1) && (Number_flagch >= 16) && ((Number_flagch - 16) % 6) == 0) {
        DataOut(Buffer_ch1, len_bch1, Buffer_ch2, len_bch2, channel_flag, len_cf,
                &dataout_flag, (short*)out1,(short *)out2, len_dout);
    }

    *flag = dataout_flag;
//    printf("out1:");
//    for (int i = 0; i < 2048*6; ++i) {
//        printf(" %d ", out1[i]);
//    }
//    printf("\n");

}

void get_flag2(char *in1,char * in2,char *out1, char *out2,int *flag){

    ChannelCompare(Buffer_ch1, Buffer_ch2, Power_ch1, Power_ch2, (short*)in1,(short *)in2,
                   &Number_re, channel_flag, &out_flag, &Power_now, &Number_flagch);
    if ((out_flag == 1) && (Number_flagch >= 16) && ((Number_flagch - 16) % 6) == 0) {
        DataOut(Buffer_ch1, len_bch1, Buffer_ch2, len_bch2, channel_flag, len_cf,
                &dataout_flag, (short*)out1,(short *)out2, len_dout);
    }

    *flag = dataout_flag;
}

void test3(char *in1,char *out1){
    for (int i=0;i<1024;i++){
        out1[i] = in1[i];
    }
}
