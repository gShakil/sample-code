#include <iostream>
#include <sstream>
#include <memory>
using namespace std;

class DoubleExpression;
class AdditionExpression;
class ExpressionVisitor {
public:
	virtual void visit(DoubleExpression* de) = 0;
	virtual void visit(AdditionExpression* ae) = 0;
};

class ExpressionPrinter :
	public ExpressionVisitor
{
public:
	ostringstream oss;
	string str() const { return oss.str(); }
	void visit(DoubleExpression* de) override
	{
		cout << "Double Expression visit .. " << endl;
	}
	void visit(AdditionExpression* ae) override
	{
		cout << "Additional Expression visit .. " << endl;
	}
};

class Expression {
	virtual void accept(ExpressionVisitor* visitor) = 0;
};

class DoubleExpression
	: public Expression
{
public:
	void accept(ExpressionVisitor* visitor) override {
		visitor->visit(this);
	}
};

class AdditionExpression
	: public Expression
{
public:
	void accept(ExpressionVisitor* visitor) override {
		visitor->visit(this);
	}
};



int main()
{
	ExpressionVisitor* pvisitor = new ExpressionPrinter;

	DoubleExpression de;
	de.accept(pvisitor);
	// Double Expression visit ..

	AdditionExpression ae;
	ae.accept(pvisitor);
	// Additional Expression visit ..
}
