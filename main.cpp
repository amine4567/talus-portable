#include <cstdlib>
#include <iostream>
#include <math.h>
#include <winbgim.h>
#include <stdio.h>
#include <graphics.h>

#define M_PI       3.14159265358979323846

#define c 7
#define gamma 16
#define phi 32

#define N 800 //resolution HZ
#define M 650 //resolution V

#define k 15 //1m dans la realite correspond à k px sur le dessin

using namespace std;

float rayon(float xo, float yo, float h)
{
    return sqrt(pow(xo-h*2.5,2)+pow(yo-h,2));
}

float calculFs(float xo, float yo, float h, float Fs, int n)
{
    float r,xp, yp, xm, *xtranche,b, *hm,*sina,*cosa,*tana, xmilieu;
    float yinf, ysup, Fs0, Tol=0.0001, m, S1, S2=0;
    int i;

    xtranche=(float*)malloc(sizeof(float)*(n+1));
    hm=(float*)malloc(sizeof(float)*n);
    sina=(float*)malloc(sizeof(float)*n);
    cosa=(float*)malloc(sizeof(float)*n);
    tana=(float*)malloc(sizeof(float)*n);

    yp=h;
    xp=yp*2.5;
    r=rayon(xo, yo, h);

    xm=xo-sqrt(pow(r,2)-pow(yo,2));

    b=(xp-xm)/n;

    xtranche[0]=xm;
    for(i=1; i<=n; i++)
    {
        xtranche[i]=xtranche[i-1]+b;
    }

    for(i=0; i<=n-1; i++)
    {
        xmilieu=(xtranche[i]+xtranche[i+1])/2;

        yinf=yo+sqrt(pow(r,2)-pow(xmilieu-xo,2));

        if(xmilieu<=0) ysup=0;
        else
            ysup=(yp/xp)*xmilieu;

        hm[i]=yinf-ysup;


        ysup=yo+sqrt(pow(r,2)-pow(xtranche[i]-xo,2));
        yinf=yo+sqrt(pow(r,2)-pow(xtranche[i+1]-xo,2));

        sina[i]=(yinf-ysup)/(sqrt(pow(b,2)+pow(yinf-ysup,2)));
        cosa[i]=b/(sqrt(pow(b,2)+pow(yinf-ysup,2)));
        tana[i]=sina[i]/cosa[i];

    }

    for(i=0; i<=n-1; i++)
    {
        S2+=hm[i]*sina[i];
    }

    do
    {
        Fs0=Fs;
        S1=0;
        for(i=0; i<=n-1; i++)
        {
            m=cosa[i]*(1+((tana[i]*tan(phi*M_PI/180))/Fs0));

            S1+=(c+(gamma-10)*hm[i]*tan(phi*M_PI/180))/m;
        }
        Fs=S1/(gamma*S2);
    }
    while(fabs(Fs-Fs0)>Tol);

    return Fs;
}

float zone_x(float h, float Fs, int n)
{
    float f1, f2,x=0.01;
    f2=calculFs(x,-0.01,h,Fs,n);

    do
    {
        x+=1;
        f1=f2;
        f2=calculFs(x,-0.01,h,Fs,n);
    }
    while(f2<f1);
    return x;
}

float zone_y(float h, float Fs, int n)
{
    float f1, f2, y=-0.01;
    f2=calculFs(0.01,y,h,Fs,n);
    do
    {
        y-=1;
        f1=f2;
        f2=calculFs(0.01,y,h,Fs,n);
    }
    while(f2<f1);
    return y;
}

void isocontour(float h, float Fs, int n)
{
    float xmin,ymin,xmax,ymax,x,y,ftemp;

    xmin=-5;
    ymin=zone_y(h,Fs,n);
    xmax=zone_x(h,Fs,n);
    ymax=-0.01;
    for(x=xmin; x<=h*2.5*0.96; x+=0.1)
    {
        for(y=x/2.5; y>=1.5*ymin; y-=0.1)
        {
            ftemp=calculFs(x,y,h,1,n);
            if(fabs(ftemp-Fs)<0.01)
            {
                putpixel(x*k+300,y*k+M-50-k*h,COLOR((int)(ftemp*1000)%255,255-(int)(ftemp*1000)%255,0));
            }
        }
    }
}

void dessiner_cercle(float h,float xcr, float ycr, float r)
{
    float Xm,Ym,Xp,Yp,Xcr,Ycr,Xs,Ys,stangle,endangle;

    Xs=300;
    Ys=M-50-k*h;

    Xm=Xs+k*(xcr-sqrt(pow(r,2)-pow(ycr,2)));
    Ym=Ys;

    Xp=Xs+2.5*k*h;
    Yp=M-50;

    Xcr=Xs+xcr*k;
    Ycr=Ys+ycr*k;

    setcolor(BLUE);
    line(Xcr,Ycr,Xm,Ym);//droite entre centre cercle et M
    line(Xcr,Ycr,Xp,Yp);//droite entre centre cercle et P

    stangle=180+atan((Ys-Ycr)/(Xcr-Xm))*180/M_PI;
    if(Xcr<Xp)
        endangle=270+atan(fabs(Xcr-Xp)/(Yp-Ycr))*180/M_PI;
    else
        endangle=270-atan(fabs(Xcr-Xp)/(Yp-Ycr))*180/M_PI;

    arc(Xcr,Ycr,stangle,endangle,r*15);
}

float balayage(float h, float Fs, int n, float *xf, float *yf)
{
    float Fsmin,f2,xmin,ymin,xmax,ymax,temp,x,y,pas=5;
    int crit=0;

    Fsmin=calculFs(0.01,0.01,h,Fs,n);
    xmax=zone_x(h,Fs,n);
    xmin=0.01;
    ymin=zone_y(h,Fs,n);
    ymax=-0.01;

    do
    {
        f2=Fsmin;
        for(x=xmin; x<=xmax; x+=pas)
        {

            for(y=ymax; y>=ymin; y-=pas)
            {
                temp=calculFs(x,y,h,Fs,n);
                setcolor(BLACK);
                if(temp<Fsmin)
                {
                    Fsmin=temp;
                    *xf=x;
                    *yf=y;
                    crit++;
                    if(crit%2!=0 || crit<8)
                    {
                        isocontour(h,Fsmin,n);
                    }
                }
            }
        }
        xmin=*xf-pas;
        ymin=*yf-pas;
        xmax=*xf+pas;
        ymax=*yf+pas;
        pas=pas/3;
    }
    while(fabs(f2-Fsmin)>0.001);
    return Fsmin;
}

void dessiner(float h)
{
    float Xp,Yp,Xs,Ys;

    Xs=300;
    Ys=M-50-k*h;

    Xp=Xs+2.5*k*h;
    Yp=M-50;

    initwindow(N,M);
    setbkcolor(WHITE);
    cleardevice();

    setcolor(BLACK);
    line(0,Yp,N,Yp);//droite hz inf
    line(0,Ys,Xs,Ys);//droite hz sup
    line(Xs,Ys,Xp,Yp);//pente talus
}


int main(int argc, char *argv[])
{
    float  h, Fs,*xf=NULL,*yf=NULL, r;
    int n;

    xf=(float*)malloc(sizeof(float));
    yf=(float*)malloc(sizeof(float));

    printf("La hauteur du deblai: h=");
    scanf("%f",&h);
    printf("Facteur de securite pour initier: Fs=");
    scanf("%f",&Fs);
    printf("Nombre de tranches: n=");
    scanf("%d",&n);

    dessiner(h);

    Fs=balayage(h,Fs,n,xf,yf);
    r=rayon(*xf,*yf, h);
    printf("\nFs minimal= %.3f\nXcr= %.3f m\nYcr= %.3f m\nRayon= %.3f m",Fs,*xf,*yf, r);

    dessiner_cercle(h,*xf,*yf,r);

    printf("\n\n");
    system("PAUSE");
    return EXIT_SUCCESS;
}
