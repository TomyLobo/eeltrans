#include "AVSTrans_cpp.h"
#include <iostream>

int main(int argc, char* argv[]) {
	/* 
	// test mkexecs
	StrVec bla(6);
	bla[0]="a";
	bla[1]="b";
	bla[2]="c";
	bla[3]="d";
	bla[4]="e";
	bla[5]="f";
	while (bla.size() > 1) {
		mkexecs(bla);
	}
	cout << bla[0];
	//*/
	
	/*
	// test CommandSplit
	StrVec bss;
	string bla="buh;bla;blub;";
	CommandSplit(bss, bla, false);
	cout << bla << "\n";
	for (int i = 0; i < bss.size(); ++i) cout << bss[i] << "\n";
	//*/
	
	/*
	// test SpecTrim
	string bla = " ";
	SpecTrim(bla);
	cout << "+" << bla << "+";
	//*/
	
	/*
	// test Split/Join
	StrVec bss;
	string bla="buh;bla;blub;argh";
	cout << bla << "\n\n";
	Split(bss, bla, ";", 2);
	for (int i = 0; i < bss.size(); ++i) cout << "+" << bss[i] << "+" << "\n";
	bla="";
	Join(bla,bss, ";");
	cout << "\n" << bla << "\n";
	//*/
	
	/*
	// test TransformCode/parsecommand
	cout << TransformCode("a;\r\nb;\r\nc;\r\nd;",0,mExec,true);
	//*/
	
	/*
	// test translate
	//cout << translate("loop(5,a=b);",mExec,true);
	cout << translate(
"//$DoAutoClipboard\r\n\
//$AVSTrans_Replacement=xp->megabuf(pn)\r\n\
//$AVSTrans_Replacement=yp->megabuf(pn+1)\r\n\
//$AVSTrans_Replacement=zp->megabuf(pn+2)\r\n\
//$AVSTrans_Replacement=xm->megabuf(pn+3)\r\n\
//$AVSTrans_Replacement=ym->megabuf(pn+4)\r\n\
//$AVSTrans_Replacement=zm->megabuf(pn+5)\r\n\
//$AVSTrans_Replacement=lifetime->megabuf(pn+6)\r\n\
//$AVSTrans_Replacement=ltm->megabuf(pn+7)\r\n\
//$AVSTrans_Replacement=pnm->8\r\n\
n=w*h*.01;\r\n\
pn=0;\r\n\
loop(n,\r\n\
  ym=.5*(rand(1000)/1000);\r\n\
  trm=rand(1000)*.002*$pi;\r\n\
  tdm=rand(1000)*.0001;\r\n\
  xm=cos(trm)*tdm;\r\n\
  zm=sin(trm)*tdm;\r\n\
//  xm=(rand(1000)*.002-1)*.1;\r\n\
//  zm=(rand(1000)*.002-1)*.1;\r\n\
  ix=1;\r\n\
  iz=1;\r\n\
  loop(10,\r\n\
    if(above(sqr(ix)+sqr(iz),.25),\r\n\
      ix=rand(1000)/1000-.5;\r\n\
      iz=rand(1000)/1000-.5;\r\n\
    ,\r\n\
      0\r\n\
    );\r\n\
  );\r\n\
  lifetime=rand(1000)/1000;\r\n\
  ltm=rand(1000)/2000+.5;\r\n\
\r\n\
  xp=ix;\r\n\
  yp=lifetime/ltm*ym;\r\n\
  zp=iz;\r\n\
\r\n\
  pn=pn+pnm;\r\n\
);"
	,mPlus,false);
	//*/
	
	/*
	// test CStyleReplace
	cout << translate(
"e=1+bla(3,4,5)+2;\r\n\
b;\r\n\
//$avstrans_replacement_cstyle=bla(a,b,c)->(a+b+c)\r\n\
c;//argl\r\n\
d;"
,mExec,true);
	//*/

	autoprefix="#define a(pn,x1,y1,z1,x2,y2,z2,x3,y3,z3) xp1(pn*pnm)=x1;yp1(pn*pnm)=y1;zp1(pn*pnm)=z1;xp2(pn*pnm)=x2;yp2(pn*pnm)=y2;zp2(pn*pnm)=z2;xp3(pn*pnm)=x3;yp3(pn*pnm)=y3;zp3(pn*pnm)=z3;\r\n\
#define xp1(pn) gmegabuf(pn)\r\n\
#define yp1(pn) gmegabuf(pn+1)\r\n\
#define zp1(pn) gmegabuf(pn+2)\r\n\
#define xp2(pn) gmegabuf(pn+3)\r\n\
#define yp2(pn) gmegabuf(pn+4)\r\n\
#define zp2(pn) gmegabuf(pn+5)\r\n\
#define xp3(pn) gmegabuf(pn+6)\r\n\
#define yp3(pn) gmegabuf(pn+7)\r\n\
#define zp3(pn) gmegabuf(pn+8)\r\n\
#define pnm 9\r\n\
#define rotate(vn,z,x,y) tmpx=x ## vn*c ## z-y ## vn*s ## z;tmpy=x ## vn*s ## z+y ## vn*c ## z;y ## vn=tmpy*c ## x-z ## vn*s ## x;tmpz=tmpy*s ## x+z ## vn*c ## x;x ## vn=tmpx*c ## y-tmpz*s ## y;z ## vn=tmpx*s ## y+tmpz*c ## y;\r\n\
#define transform(vn) tmpz=1/(z ## vn-oz);x ## vn=(x ## vn-ox)*tmpz*asp;y ## vn=(y ## vn-oy)*tmpz;\r\n\
#define vecprodx(ax,ay,az,bx,by,bz) ((ay)*(bz)-(az)*(by))\r\n\
#define vecprody(ax,ay,az,bx,by,bz) ((az)*(bx)-(ax)*(bz))\r\n\
#define vecprodz(ax,ay,az,bx,by,bz) ((ax)*(by)-(ay)*(bx))\r\n\
#define scalprod(ax,ay,az,bx,by,bz) ((ax)*(bx)+(ay)*(by)+(az)*(bz))\r\n\
#define normalise(x,y,z) r=invabs(x,y,z);x=x*r;y=y*r;z=z*r;\r\n\
#define invabs(x,y,z) invsqrt(sqr(x)+sqr(y)+sqr(z))\r\n\
#define min(a,b,c) min(a,min(b,c))\r\n\
#define max(a,b,c) max(a,min(b,c))\r\n\
#define debug(tri,val) if(equal(pn,tri*pnm),reg70=val,0);\r\n\
#define writeregs() reg00=ox;reg01=oy;reg02=oz;reg03=cx;reg04=cy;reg05=cz;reg06=sx;reg07=sy;reg08=sz;reg09=xfac;reg10=yfac;\r\n\
#define readregs() ox=reg00;oy=reg01;oz=reg02;cx=reg03;cy=reg04;cz=reg05;sx=reg06;sy=reg07;sz=reg08;xfac=reg09;yfac=reg10;";

	cout << translate("a=b=c=d;e=f;g=h;",mExec,false);

	//std::string s("123456"); std::cout << s.substr(3,2);
	string x;cin>>x;return 0;
}
