class AAA
{
	public void showName()
	{
		System.out.println("My Name is AAA");
	}
}

class ZZZ
{
	public void showName()
	{
		System.out.println("My Name is ZZZ");
	}
}

class WhatYourName
{
	public static void main(String[] args)
	{
		AAA aaa = new AAA();
		aaa.showName();
		
		ZZZ zzz = new ZZZ();
		zzz.showName();
	}
}
	