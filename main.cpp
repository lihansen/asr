#include <stdio.h>
#include <math.h>
#include<iostream>
#include <stdlib.h>
/*  ����1����������Ҷ�任  */
static void four1(double *data, int n, int isign)
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
/*  ����2��ʵ������Ҷ�任   */
static void realft(double data[],int n,int isign)
{
    int i, i1, i2, i3, i4;
    double c1 = 0.5, c2, h1r, h1i, h2r, h2i, wr, wi, wpr, wpi, wtemp;
    double theta = 3.141592653589793238 / (double(n >> 1));
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
/*   ����3����������  */
static void sort(double *array, int len)
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
/*  ����4������Ƶ������ֵ  */
static void PowerCount(int len, int inc, double data[], int n,double *f,int fh)
{
    int i=0,j,ii,i1,i2;
    double a[1024],*pa;
    for (pa = f; pa < f+fh; pa++)
    {
        *pa = 0.0;
        for (j = 0; j < len; j++)
        {
            a[j] = data[i*inc + j];            //�����ÿ�������
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
/*  ����5������channel_flag��ֵ   */
static void ChannelCompare(int Buffer_ch1[],int Buffer_ch2[],double Power_ch1[],double Power_ch2[],int data_in1[], int data_in2[], int *Number_re, int channel_flag[],int *out_flag,double *Power_now,int *Number_flagch)
{
    int i=0, ii=0, channel_flag1=0;
    double Power_temp1=0.0, Power_temp2=0.0, Diff_ch1, Diff_ch2, Ratio_ch,data_temp1[1024], data_temp2[1024], data_temp3[1024+512], data_temp4[1024 + 512];

    if (*Number_re < 48) // ����֡������48֡�����������ź�������û�������������out_flag = 0
    {
        //����buffer
        if (*Number_re < 16) // ֡��С��16��������������buffer��
        {
            for (ii = 0;ii<1024;ii++)
            {
                Buffer_ch1[*Number_re * 1024 + ii] = data_in1[ii];    //���������忽��
                Buffer_ch2[*Number_re * 1024 + ii] = data_in2[ii];
            }
            *Number_re = *Number_re + 1;
            *out_flag = 0;
            // 1k~4kHzƽ��ֵ�����������Power����
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
            //�������ݼ��㻷�����������������
            return;
        }
        // ��������buffer
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
        // 1k~4kHzƽ��ֵ�����������Power���飬�����ۻ�
        for (i = 0; i < 1024 + 512; i++)
        {
            data_temp3[i] = (double)(Buffer_ch1[14*1024+512+i] / (pow(2, 15) - 1));
            data_temp4[i] = (double)(Buffer_ch2[14 * 1024 + 512 + i] / (pow(2, 15) - 1));
        }

        PowerCount(1024, 512, data_temp3, sizeof(data_temp3) / sizeof(data_temp3[0]), &Power_ch1[(*Number_re - 1) * 2-1],2);
        PowerCount(1024, 512, data_temp4, sizeof(data_temp4) / sizeof(data_temp4[0]), &Power_ch2[(*Number_re - 1) * 2-1],2);
        return;
    }
    // ��������֡������48����ʼ���㻷������������buffer��ʼ��λ
    if (*Number_re >= 48)
    {
        *Number_re = *Number_re + 1;        //Ҫ������ֵ�������
        //���㲻�������ֵ
        if (*Power_now == 0.0)
        {
            sort(Power_ch1,95);   //sort��������ݰ��մ�С��������
            Power_temp1 = Power_ch1[47];
            sort(Power_ch2,95);
            Power_temp2 = Power_ch2[47];
            if (Power_temp1>Power_temp2)
                *Power_now = Power_temp2;     //�õ���ǰ����������ֵ
            else
                *Power_now = Power_temp1;
        }
        //��������buffer
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
        //����Power����
        for (i = 0; i < 1024 + 512; i++)
        {
            data_temp3[i] =(double)(Buffer_ch1[14*1024+512+i] / (pow(2, 15) - 1));
            data_temp4[i] =(double)(Buffer_ch2[14 * 1024 + 512 + i] / (pow(2, 15) - 1));
        }

        PowerCount(1024, 512, data_temp3, sizeof(data_temp3) / sizeof(data_temp3[0]), &Power_ch1[93],2);
        PowerCount(1024, 512, data_temp4, sizeof(data_temp4) / sizeof(data_temp4[0]), &Power_ch2[93],2);
        //����ͨ�������Ա��ж�
        Diff_ch1 = Power_ch1[94] / *Power_now;               //��ʵ�������
        Diff_ch2 = Power_ch2[94] / *Power_now;               //��ʵ�������
        Ratio_ch = Power_ch1[94] / Power_ch2[94];
        channel_flag1 = 0;                                              //ÿ�δ������ǰͨ����ʶ����
        if (Diff_ch1 >= 1 && Diff_ch2 >= 1) // ������ֵ[1]
        {
            if  (Ratio_ch > 1.3)               //������ֵ[2]
                channel_flag1 = 1;
            else if  (Ratio_ch < 0.7692)   //������ֵ1 / 1.3
                channel_flag1 = 2;
            else if  ((Diff_ch1>1.4)&&(Diff_ch2>1.4)&&(1.3>Ratio_ch)&&(Ratio_ch>0.7692))   //������ֵ[3]1.4��������ֵ1.3��1 / 1.3
                channel_flag1 = 3;
            else
                channel_flag1 = 0;
        }
        else if ((Diff_ch1 >= 1) && (Diff_ch2 < 1))//������ֵ
        {
            if  (Diff_ch1 > 1.4 )                           //������ֵ
                channel_flag1 = 1;
            else
                channel_flag1 = 0;
        }
        else if ((Diff_ch1 < 1)&& (Diff_ch2 >= 1)) //������ֵ
        {
            if  (Diff_ch2 > 1.4)                          //������ֵ
                channel_flag1 = 2;
            else
                channel_flag1 = 0;
        }
        else
            channel_flag1 = 0;

        *Number_flagch = *Number_flagch + 1;           //������ֵ�������
        if (*Number_flagch <= 16)
        {
            channel_flag[*Number_flagch-1] = channel_flag1;
            *out_flag = 0;
        }
        else
        {
            for (i = 0; i < 15; i++)
            {
                channel_flag[i] = channel_flag[1 + i];
            }
            channel_flag[15] = channel_flag1;
            *out_flag = 1;
        }
    }
}
/*    ����6���ж�ͨ���������    */
static void DataOut(int Buffer_ch1[], const int len_bch1, int Buffer_ch2[],const int len_bch2, int channel_flag[],const int len_cf, int *dataout_flag, int data_out1[], int data_out2[],const int len_dout)
{
    int i, num0 = 0,num1 = 0,num2 = 0,num3 = 0;
    for (i = 0; i < len_dout; i++)
    {
        data_out1[i] = Buffer_ch1[5 * 1024 + i];
        data_out2[i] = Buffer_ch2[5 * 1024 + i];
    }
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
    if (num3!= 0) // ��3
        *dataout_flag = 3;
    else        //û��3
    {
        if (num2 != 0) // ��2
        {
            if (num1 != 0) // ��1
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
            else                //û��1
                *dataout_flag = 2;
        }
        else                           //û��2
        {
            if (num1!= 0 )// ��1
                *dataout_flag = 1;
            else               //û��1
                *dataout_flag = 0;
        }
    }
}

const int len_bch1 = 1024 * 16, len_bch2 = 1024 * 16, len_cf = 16, len_dout = 1024* 6;
int Buffer_ch1[1024 * 16], Buffer_ch2[1024 * 16], data_out1[1024 * 6], data_out2[1024 * 6];
int Number_re = 0, out_flag = 0, Number_flagch = 0,dataout_flag=0, channel_flag[16]= { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
double Power_ch1[95], Power_ch2[95], Power_now = 0.0;
int data_in1[1024], data_in2[1024];

static void init(){
    int i = 0;
    /*1����ʼ������*/
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
    for (i = 0; i < 1024 * 6; i++)
    {
        data_out1[i] = 0;
        data_out2[i] = 0;
    }
}

static void get_flag(int *in1,int * in2,int *out1, int *out2,int *flag){
    ChannelCompare(Buffer_ch1, Buffer_ch2, Power_ch1, Power_ch2, in1, in2,
                   &Number_re, channel_flag, &out_flag, &Power_now, &Number_flagch);
    if ((out_flag == 1) && (Number_flagch >= 16) && ((Number_flagch - 16) % 6) == 0) {
        DataOut(Buffer_ch1, len_bch1, Buffer_ch2, len_bch2, channel_flag, len_cf,
                &dataout_flag, out1, out2, len_dout);
    }
    *flag = dataout_flag;
}

//
//static void my_cc(int *in1,int *in2){
//    ChannelCompare(Buffer_ch1, Buffer_ch2, Power_ch1, Power_ch2, in1, in2,
//                   &Number_re, channel_flag, &out_flag, &Power_now, &Number_flagch);
//}
//
//
//static void test(){
//
//    init();
//    int i = 0,j=0;
//    /* 2��ѭ�����ú���  */
//    for (i = 0; i < 1000; i++)
//    {    /* ��������������� */
//        for (j = 0; j < 1024; j++)//�ֿ� 1024 ��֡
//        {
//            data_in1[j] = (rand() % 10);    //���������1024�����������
//            data_in2[j] = (rand() % 10);
//        }
//        /*  �������ò���  */
//        //�������뵽������
////        ChannelCompare(Buffer_ch1, Buffer_ch2, Power_ch1, Power_ch2, data_in1, data_in2,
////                       &Number_re, channel_flag, &out_flag, &Power_now, &Number_flagch);
//        my_cc(data_in1,data_in2);
//        if ((out_flag == 1) && (Number_flagch >= 16) && ((Number_flagch - 16) % 6) == 0){
//            DataOut(Buffer_ch1, len_bch1, Buffer_ch2, len_bch2, channel_flag, len_cf,
//                    &dataout_flag, data_out1, data_out2, len_dout);
//        }
//
//        printf("%d\n",i);
//    }
//}

static void my_test(){
    init();
    int my_flag = 0;
    int i = 0,j=0;
    /* 2��ѭ�����ú���  */
    for (i = 0; i < 1000; i++)
    {    /* ��������������� */
        for (j = 0; j < 1024; j++)//�ֿ� 1024 ��֡
        {
            data_in1[j] = (rand() % 10);    //���������1024�����������
            data_in2[j] = (rand() % 10);
        }
        /*  �������ò���  */
        //�������뵽������
        get_flag(data_in1,data_in2,data_out1,data_out2,&my_flag);

        printf("%d\n",i);
    }
}

int main()
{
    my_test();
}