#ifndef Subdomain_h
#define Subdomain_h
#include <string>
#include <Variable.h>
using std::string;

namespace moving_boundary
{
enum SubdomainType
{
	subdomain_volume=0,
	subdomain_membrane,
	subdomain_point,
};

struct Subdomain {

	Subdomain(const string & name);
	virtual ~Subdomain();

	const string & name( ) const {
		return name_;
	}
	virtual SubdomainType getType()=0;
	void addVariable(Variable* v)
	{
		_variables.push_back(v);
	}

private:
	string name_;
	vector<Variable*> _variables;
};

}

#endif
