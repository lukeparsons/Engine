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

	bool isJust() const
	{
		return is_just;
	}

	bool isNothing() const
	{
		return !is_just;
	}

	A fromJust() const
	{
		return value;
	}

	A fromMaybe() const
	{
		return is_just ? value : A();
	}
};
