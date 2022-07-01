#pragma once

struct Nothing {};

template<typename A>
class Maybe
{
private:

	union
	{
		A value;
		Nothing nothing;
	};

	bool is_just;
public:

	Maybe<A>() : nothing(Nothing()), is_just(false) {};

	Maybe<A>(A val) : value(val), is_just(true) {};

	~Maybe<A>() {};

	bool isJust()
	{
		return is_just;
	}

	bool isNothing()
	{
		return !is_just;
	}

	const A& fromJust()
	{
		return value;
	}

	const A& fromMaybe()
	{
		if (is_just)
		{
			return value;
		} else
		{
			return A();
		}
	}
};