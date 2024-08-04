import com.wxfx.smart.Circle;

class ImportCircle
{
	public static void main(String args[])
	{
		Circle c1 = new Circle(3.5);
		System.out.println("radius 3.5 area: " + c1.getArea());
		Circle c2 = new Circle(5.5);
		System.out.println("radius 5.5 area: " + c2.getArea());
	}
}