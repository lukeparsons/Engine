#pragma once

template<typename A>
class Maybe
{
private:
	union
	{
		A value;
		std::nullopt_t nothing;
	};

	bool is_just;
public:

	Maybe<A>() : nothing(std::nullopt), is_just(false) {};

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

	A fromJust()
	{
		return value;
	}

	A fromMaybe()
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