class CircleUsing
{
	public static void main(String args[])
	{
		com.wxfx.smart.Circle c1 = new com.wxfx.smart.Circle(3.5);
		System.out.println("radius 3.5, area: " + c1.getArea());
		
		com.fxmx.simple.Circle c2 = new com.fxmx.simple.Circle(3.5);
		System.out.println("radius 3.5, Perimeter: " + c2.getPerimeter());
	}
}