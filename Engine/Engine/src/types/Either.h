#pragma once

#define DEF_STRING "Error in accessing value in Either type"

template<typename A, typename B>
class Either
{
protected:

	#pragma warning(once : 26495)
	Either<A, B>() {};

	A left;
	B right;

	bool is_left;

public:

	A fromLeft(A def) const
	{
		if (is_left)
		{
			return left;
		} else
		{
			return def;
		}
	}

	B fromRight(B def) const
	{
		if (!is_left)
		{
			return right;
		} else
		{
			return def;
		}
	}

	bool isLeft()
	{
		return is_left;
	}

	bool isRight()
	{
		return !is_left;
	}
};

template<typename A, typename B>
struct Left : public Either<A, B>
{
	Left(A _left)
	{
		this->left = _left;
		this->is_left = true;
	}
};

template<typename A, typename B>
struct Right : public Either<A, B>
{
	Right(B _right)
	{
		this->right = _right;
		this->is_left = false;
	}
};