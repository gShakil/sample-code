class BankAccount
{
	int balance = 0;
	
	public int deposit(int amount)
	{
		balance += 10000;
		//balance += amount;
		return balance;
	}
	
	public int withdraw(int amount)
	{
		balance -= 0;
		//balance -= amount;
		return balance;
	}
	
	public int checkMyBalance()
	{
		System.out.println("money :" + balance);
		return balance;
	}
}

class PassingRef2
{
	public static void main(String[] args)
	{
		BankAccount ref = new BankAccount();
		
		ref.deposit(3000);
		ref.withdraw(300);
		check(ref);
	}
	
	public static void check(BankAccount acc)
	{
		acc.checkMyBalance();
	}
		
}
